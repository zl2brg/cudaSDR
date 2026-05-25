# SoapySDR sample rate strategy

Recommendations for handling sample-rate mismatches between SoapySDR hardware and cudaSDR’s WDSP pipeline. Captured 2026-05-25 after RTL-SDR / `fexchange0` pacing work.

## Current architecture

`SoapySDRDataSource` already uses a **two-rate** model:

| Layer | Rate | Role |
|--------|------|------|
| **DSP / WDSP** | `Settings::getSampleRate()` (48k, 96k, …) | `OpenChannel`, `fexchange0`, 1024-sample blocks |
| **RF / Soapy** | `m_rfSampleRate` | `setSampleRate` on the device |
| **Bridge** | `m_decimRatio` | Boxcar average in `runStream()` → 1024 complex samples per enqueue |

Negotiation at open (`SoapySDRDataSource.cpp`):

1. Check discrete `listSampleRates` for exact DSP rate match.
2. Else query `getSampleRateRange` and set `rf = ceil(minRf / dsp) * dsp`, `decim = rf / dsp`.
3. Hardware-key overrides (e.g. LimeSDR-Mini floor).
4. `setSampleRate`, read back actual rate, adjust `m_decimRatio` with `round(actual / dsp)`.

Works when hardware honors an **integer multiple** (e.g. RTL 240 kHz → 48 kHz ×5). Weak when:

- Driver **rounds** RF so `actual / dsp` is not an integer.
- **Best** native rate is not `ceil(min/dsp)*dsp` (PLL, stability, driver quirks).
- User changes DSP rate but **Receiver pump interval**, **WDSP**, and **Soapy RF** drift out of sync.

WDSP (`QWDSPEngine::setSampleRate`) only accepts discrete IQ rates: 48k, 96k, 192k, 384k, 768k, 1536k. Soapy paths should **feed WDSP at the chosen DSP rate**, not arbitrary native USB rates.

Related code:

- `src/DataEngine/SoapySDRDataSource.{h,cpp}` — RF negotiation, decimation, stream
- `src/DataEngine/cusdr_receiver.cpp` — Soapy queue, pump timer (~`1024 / dspRate` ms), one block per tick
- `src/QtWDSP/qtwdsp_dspEngine.cpp` — `OpenChannel` / `setSampleRate`
- `src/DataEngine/fractresampler.{h,cpp}` — fractional resampler (used elsewhere; not yet on Soapy IQ path)

## Recommended architecture: `SoapyRatePlan`

Centralize negotiation in one struct, computed at **open** and on **`sampleRateChanged`**:

```cpp
struct SoapyRatePlan {
    int dspRateHz;        // Settings / WDSP IQ rate
    int rfRateHz;         // value passed to Soapy (after readback: actualRfHz)
    int decimRatio;       // integer only; 0 means "use resampler"
    double resampleRatio; // rfActual / dspRate if non-integer
    int dspBlockSamples;  // BUFFER_SIZE (1024)
    int pumpIntervalMs;   // 1000 * dspBlockSamples / dspRateHz
};
```

Single function: `computeRatePlan(device, dspRateHz, hwKey)`.

On apply:

1. Soapy: `setSampleRate`, restart stream if needed.
2. Emit `soapyRatePlanChanged(plan)` → `Receiver::updateSoapyPumpInterval()`, `qtwdsp->setSampleRate(dspRateHz)`.
3. Audio sink stays at 48 kHz (WDSP audio output rate unchanged).

UI example: *“RTL-SDR: 240 kS/s → 48 kS/s (×5)”* — not implying the radio runs at 48 kS/s on the wire.

## RF rate selection (improve on `ceil(min/dsp)*dsp`)

1. **Exact match** — device supports `dspRateHz` → `rf = dsp`, `decim = 1`.
2. **Integer decimation** — score candidates `rf` in `[min, max]` (discrete list + snapped values: min, 2×min, 1M, 2M, …):
   - `decim = rf / dsp` must be integer (±1 Hz after readback).
   - Prefer lowest RF that meets minimum (less USB bandwidth), unless `hwKey` table overrides (Lime ≥1.3 MSPS, etc.).
3. **Read back** `getSampleRate()` — if `round(actual/dsp)` is not integer, **do not** use boxcar decim; use resampler (below).
4. **Publish** `effectiveDsp = actualRf / decim` — warn if ≠ `dspRateHz`.

## Non-integer ratio: resampler vs boxcar

Boxcar decimation assumes **exact** `rf = decim × dsp`.

| Case | Bridge |
|------|--------|
| Integer ratio (RTL, many devices) | Lightweight boxcar in `runStream` (current) |
| Rounded / non-integer ratio | `CFractResampler` RF → `dspRateHz`, fixed 1024-sample output blocks |
| Very high RF (Lime 2+ MSPS) | Prefer native RF with clean integer decim to DSP; avoid resampling 2 MSPS → 48 kHz in software |

Alternative (heavier CPU): open WDSP at **native RF** and use channel internal resampling to 48 kHz audio — simplifies Soapy source, costly at high rates.

## Timing contract (WDSP backpressure)

Invariant:

> **Exactly one WDSP block (1024 samples at `dspRateHz`) per pump period.**

- Pump interval: `1000 * BUFFER_SIZE / dspRateHz` ms (`Receiver::updateSoapyPumpInterval`).
- Do not burst multiple `fexchange0` calls per USB read or timer tick (causes `fexchange0` error -2).
- IQ enqueue decoupled from DSP; timer owns real-time pacing.
- Prime WDSP with a few silent blocks after Soapy stream start / rate change.

`readStream` at RF may be 1024 samples per call with accumulation, or `1024 * decim` per read — either way, **emit rate** to the DSP queue must match `dspRateHz`.

## Settings / UI policy

- **HPSDR**: user picks protocol-native IQ rates (unchanged).
- **Soapy**: `Settings::sampleRate` = **“DSP IQ rate”** (48/96/192k ladder), not USB chip rate.
- Optional advanced: RF mode Auto / Native / Minimum bandwidth; default **Auto** via `computeRatePlan`.
- Filter UI rate choices to WDSP `validRates`.

## Runtime sample-rate changes

On `sampleRateChanged`:

1. Recompute `SoapyRatePlan` (stored `m_minSampleRate` + hw overrides).
2. Queue stream restart (existing atomics on `SoapySDRDataSource`).
3. Reset decim state; prime WDSP; restart pump timer.
4. Call `qtwdsp->setSampleRate` with the same `dspRateHz` as the plan (Soapy path must not only retune RF).

## Implementation priority (incremental)

1. **Validate** integer ratio after `getSampleRate()`; log and switch path if invalid.
2. **`SoapyRatePlan` + signal** — sync source, receiver pump, WDSP, telemetry/UI.
3. **Scoring** for RF candidate selection (discrete list + hw table).
4. **`FractResampler`** on Soapy thread for non-integer fallback.
5. UI readout of RF → DSP (decim or resample).

## Soapy pacing fixes (already applied)

Reference for related receiver/source work:

- `pumpSoapyIfReady`: one block per timer tick (not 6).
- `scheduleSoapyDsp` / `enqueueSoapyData`: start timer only, no extra pumps.
- Prime blocks + delayed pump start after `DataEngineUp`.
- Rate-limit `fexchange0` error -2 logging in `QWDSPEngine::processDSP`.
