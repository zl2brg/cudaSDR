# MVC slice binding

`SliceModel` is the runtime source of truth per receiver. `RadioController` and direct slice connections wire DSP/hardware; `Settings` persists to INI via `m_receiverDataList`.

## Data flow

```mermaid
flowchart LR
  UI[Views: main / AGC / popup / pan]
  SM[SliceModel]
  RC[RadioController]
  DE[DataEngine]
  RX[SliceProcessor]
  WDSP[QWDSPEngine]

  UI -->|setVolume setAgcMode setFilter*| SM
  SM --> RC
  RC --> DE
  SM --> WDSP
  RX --> WDSP
```

## Bound through `RadioController` (protocol / TX only)

Per slice (`id` == receiver index), after `DataEngine::initReceivers()`:

| Signal | Target |
|--------|--------|
| `centerFrequencyChanged` | `io.rx_freq_change` (Protocol 1/2 center-freq CC) |
| `dspModeChanged` | `DataEngine::applySliceDspMode` (TX CC bytes) |

VFO (`frequencyChanged`), filters, volume, and WDSP mode are **not** routed through `SliceProcessor`; they go `SliceModel` → `QWDSPEngine` at RX init.

Legacy center-frequency updates from `Settings::setCtrFrequency` still emit `ctrFrequencyChanged` → `DataEngine::setFrequency` via `connectDSPSlots()`.

## Bound directly on `SliceModel` (at RX init)

| Signal | Target |
|--------|--------|
| `volumeChanged` / `muteChanged` | `QWDSPEngine::setVolume` |
| `agcModeChanged` | `QWDSPEngine::setAGCMode` |
| `agcGainChanged` | `QWDSPEngine::setAGCThreshold` |
| `agcMaxGainChanged` | `QWDSPEngine::setAGCMaximumGain` |
| `agcFixedGainChanged` | `SetRXAAGCFixed` |
| `agcHangThresholdChanged` / `agcSlopeChanged` | WDSP AGC params |
| `dspModeChanged` / `filterChanged` | WDSP mode & filters |
| S-meter | `SliceProcessor` → `RadioTelemetry::setSMeterValue` → `SliceModel` → `OGLDisplayPanel` |
| Spectrum / link status | `SliceProcessor` / protocols → `RadioTelemetry` → GL panels |
| NB/NR/ANF/SNB, FFT, pan/wf modes, avg, grid | `SliceModel` → WDSP / GL (no Settings relay) |
| Protocol stream sync | `DataIO` sequence check → `RadioTelemetry::setProtocolSync` |

## Settings entry points

| User action | Updates |
|-------------|---------|
| Volume slider | `SliceModel::setVolume` (or `Settings::setMainVolume` → slice) |
| Mute | `SliceModel::setMute` |
| AGC mode / gain / hang | `Settings::setAGC*` → `SliceModel` when `RadioModel` present |

Legacy `Settings::*Changed` signals for volume/AGC/mode/filter are **not** relayed from `syncSlicesWithSettings`; views listen to `SliceModel` instead.

## Persistence

- **Load:** `Settings::syncSlicesWithSettings()` — INI → `SliceModel`
- **Save:** `Settings::syncSettingsWithSlices()` — `SliceModel` → `m_receiverDataList` → INI

## Phase 3: SliceProcessor without `TReceiver` mirror

`SliceProcessor` (formerly `Receiver`) is a DSP-thread worker only (IQ queue, spectrum, audio). It no longer holds a copy of `TReceiver m_receiverData` or mirrors Settings signals into local state.

| Concern | Source |
|---------|--------|
| Frequency, mode, filters, volume, AGC | `SliceModel` (runtime) |
| Ham band, attenuators, DSP core, INI fields | `Settings` getters / persistence |
| Protocol center-freq CC | `RadioController` → `DataEngine::setFrequency` → `io.rx_freq_change` only |
| WDSP init & live updates | `QWDSPEngine` reads `SliceModel` first (`centerFrequencyHz`, `currentDspMode`) |

Removed from `SliceProcessor`: mirror slots (`setHamBand`, `setDspMode`, `setCtrFrequency`, …), redundant getters, and `DataEngine::setFrequency` no longer calls `RX[rx]->setCtrFrequency`.

## Phase 4: UI reads slice + Settings getters

Views no longer copy `TReceiver` lists at init. Runtime fields come from `SliceModel` (when the widget is slice-scoped) or `Settings::sliceModel(rx)` / slice-aware getters (`getFilterLo`, `getDSPMode`, `getPanadapterMode`, …). Persistence-only per-band data (`dspModeList`, last-frequency lists, dBm scale per band) uses new `Settings` accessors.

Removed `QList<TReceiver> m_rxDataList` / `m_receiverDataList` from GL panels, radio/popup widgets, AGC/display/color options, and filter/radio UI helpers. `CProtocol2` still reads `getReceiverDataList()` for encoding (protocol layer, not UI).

## Phase 5: `Receiver` → `SliceProcessor`

The DSP worker class and files were renamed to reflect MVC roles: `SliceModel` owns GUI-thread state; `SliceProcessor` runs IQ → WDSP → spectrum/audio on the DSP thread. `DataEngine::RX` is now `QList<SliceProcessor*>`.

Unchanged (different concepts): `ReceiverWidget`, `ReceiverConfig`, `ReceiverAudioOutput`, `AudioReceiver`, HPSDR “receiver” count/index terminology.

## Files

- `src/Controllers/RadioController.{h,cpp}`
- `src/DataEngine/cusdr_sliceProcessor.{h,cpp}` — per-slice DSP worker (IQ queue, spectrum)
- `src/QtWDSP/qtwdsp_dspEngine.cpp` — slice-first WDSP connects
- `src/Models/RadioTelemetry.{h,cpp}` — live spectrum, meters, sync/PA telemetry
- `src/cusdr_settings.cpp` — persistence / config only (no telemetry relays)
- `src/cusdr_mainWidget.cpp`, `src/cusdr_agcWidget.cpp` — slice UI feedback
