# Protocol 1, USB documentation, and Alex filtering notes

Saved from technical discussion (2026-05-01). Source: `USB_protocol_V1.59.pdf` (text via `pdftotext`), `src/DataEngine/CProtocol1.cpp`, related UI/settings.

---

## 1. USB protocol PDF vs `CProtocol1`

### What the PDF describes

`USB_protocol_V1.59.pdf` is the **Ozy USB** view: **512-byte** frames on EP2/EP6, sync **`0x7F 0x7F 0x7F`**, then **C0–C4**, then IQ/mic (or PC→radio L/R + TX IQ). It does **not** define the **Metis/Ethernet** wrapper (`0xEF 0xFE …`, 1032-byte UDP payloads, start/stop `0x04`).

### What “Protocol 1” is in cudaSDR

The **inner 512-byte HPSDR frame** matches the USB document’s semantics. It is carried inside **Metis UDP**: 8-byte header + **two** 512-byte frames (1024 bytes payload). `DataProcessor::processReadData()` splits the payload and calls `processInputBuffer` twice per packet (`cusdr_dataEngine.cpp`).

### Alignment checks

| Topic | PDF | Code |
|--------|-----|------|
| Sync | `0x7F` × 3 | `SYNC` in `cusdr_settings.h`; start of each 512-byte slice |
| Padding (NOTE 1) | Receivers 1–8 | `CProtocol1::processInputBuffer` `maxSamples` table matches |
| C&C MOX/speed/NCO tables | PC↔radio | `encodeCCBytes` / `decodeCCBytes` broadly consistent |
| RX C0 address | EP6: bits [7:3] | `decodeCCBytes`: `roundRobin = buffer.at(0) >> 3` |

### Gaps / caveats

- **Transport**: Metis header, ports, 1032-byte init — verify against **Metis / openHPSDR network** docs, not USB V1.59 alone.
- **Receivers 9–20**: PDF NOTE 1 only lists 1–8; extended `maxSamples` values are **not** validated by this PDF.
- **C4 receiver count**: PDF diagram shows 3 bits for 1–8 RX; code can use more bits for higher RX counts (possible Hermes extension).
- **PDF inconsistency**: EP6 says C0 address in bits **[7:3]**; later PC→radio section says **[7:1]** — decode uses EP6-style `>> 3`; encode uses full-byte addresses with MOX in bit 0.
- **`formatOutputPacket`**: `qFromBigEndian(sequence)` + append yields **big-endian** sequence on the wire consistent with `protocol1Sequence()`; naming is misleading vs `qToBigEndian` but behavior on LE is OK.
- **TX I/Q swap** (PDF note): historical swap on **PC→HPSDR** path; verify TX assembly elsewhere if spectrum sign matters.

---

## 2. Alex filtering and related `CProtocol1` behavior

### Two settings blobs

**`alexConfig` (`quint16`, `Settings::getAlexConfig()`)**  
Bitfield for **filter paths**: `0x01` manual HPF/LPF; `0x02` bypass all HPFs; `0x04` 6 m LNA; `0x08`…`0x80` HPF lines; `0x100`…`0x4000` LPF lines. Documented in `cusdr_settings.cpp`, `cusdr_alexFilterWidget.cpp`, `cusdr_mainWidget.cpp`.

**`alexStates` (per-band `int`, `Settings::getAlexStates()`)**  
Per Ham band: **RX antenna** `[1:0]`, **RX aux** `[4:2]`, **TX antenna** `[6:5]`, **Alex attenuator** `[8:7]` (see `cusdr_mainWidget.cpp` `m_alexAttnState = 0x03 & (m_alexStates[...] >> 7)` and `cusdr_alexAntennaWidget.cpp` decode).

DataEngine copies both into `io.ccTx` (`cusdr_dataEngine.cpp` ~1353–1355).

### Protocol 1: where bits are sent

**`encodeCCBytes` case 0** (general `C0` frame, `CProtocol1.cpp`):  
Builds **C3** including Alex attenuator from `alexStates >> 7`, then dither/random/preamp, then **Alex RX antenna / RX out** from a variable `rxAnt` derived as:

```cpp
rxAnt = 0x07 & (io->ccTx.alexStates.at(io->ccTx.currentBand) >> 2);
```

The UI defines **RX antenna as `state & 0x03`**, but this uses **`(state >> 2) & 0x07`**, i.e. **RX aux**, not RX antenna. Likely **bug** if Alex RX path on air does not match the antenna widget.

**`encodeCCBytes` case 3** (`C0 = 0x12`):  
Maps **`alexConfig` bits `0x02`–`0x80`** into **C3** (HPF / bypass / 6 m LNA) when manual control applies. For **C4**: if MOX/PTT, sets **TX LPF from TX frequency only** (thresholds in `CProtocol1.h`); if not TX, **`control_out[4] = 0`**. The **`alexConfig` manual LPF bits (`0x100`–`0x4000`) are not merged into C4** in this path. Comments above **case 4** still describe Alex LPF, but **case 4** currently sends **ADC assignment / step attenuator** (`0x1C`), not manual Alex LPF — comments may be stale. **Risk**: manual LPF from the Alex filter panel may not reach hardware on **Protocol 1** as expected.

**Protocol 2** maps `alexConfig` HPF/LPF into a wide `alex0` word (`CProtocol2.cpp` ~418–436).

### UI vs manual mode

Main-window Alex button toggles **`alexConfig` bit `0x01`** (`alexBtnClickedEvent` → `setAlexToManual`). Filter auto/manual logic in `AlexFilterWidget` depends on that bit. Protocol text: HPF/LPF bits are only meaningful when **manual** is enabled.

---

## 3. Related source files (quick index)

| Area | Files |
|------|--------|
| P1 encode/decode | `src/DataEngine/CProtocol1.cpp`, `CProtocol1.h`, `protocol_boundary_utils.h` |
| RX split 512+512 | `src/DataEngine/cusdr_dataEngine.cpp` (`processReadData`) |
| Alex UI / config | `src/cusdr_alexFilterWidget.cpp`, `cusdr_alexAntennaWidget.cpp`, `cusdr_mainWidget.cpp` |
| Settings persistence | `src/cusdr_settings.cpp`, `cusdr_settings.h` |
| P2 Alex bitfield | `src/DataEngine/CProtocol2.cpp` |
