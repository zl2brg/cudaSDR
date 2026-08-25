# Protocol 1, USB documentation, and Alex filtering notes

Saved from technical discussion (2026-05-01); updated 2026-08-25 for Protocol 1
Alex encoding helpers and regression tests. Source: `USB_protocol_V1.59.pdf`
(text via `pdftotext`), `src/DataEngine/CProtocol1.cpp`,
`src/DataEngine/protocol_boundary_utils.h`, related UI/settings.

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
Bitfield for **filter paths** (canonical layout in Settings load/save and Protocol 2):

| Bit | Meaning |
|-----|---------|
| `0x0001` | Manual HPF/LPF select |
| `0x0002` | Bypass all HPFs |
| `0x0004` | 6 m LNA |
| `0x0008`…`0x0080` | HPF 1.5 / 6.5 / 9.5 / 13 / 20 MHz |
| `0x0100`…`0x4000` | LPF 160 … 6 m |

**`alexStates` (per-band `int`, `Settings::getAlexStates()`)**  
Per Ham band: **RX antenna** `[1:0]`, **RX aux** `[4:2]`, **TX antenna** `[6:5]`, **Alex attenuator** `[8:7]`.

DataEngine copies both into TX params for encode.

### Protocol 1: where bits are sent

Pure packing helpers live in `protocol_boundary_utils.h` and are covered by
`tests/alex_encoding_tests.cpp`.

**`encodeCCBytes` case 0** (general `C0` frame):  
- **C3 RX antenna field**: `protocol1AlexRxAntennaBits(state)` = `state & 0x03`.
- **C4 antenna relay**: `protocol1AlexAntennaRelayBits(state, transmitting)` — same policy as Protocol 2: **RX Ant** `[1:0]` while receiving, **TX Ant** `[6:5]` while MOX/PTT; mapped to wire `0=Tx1,1=Tx2,2=Tx3`. Attenuator bits must not leak into the relay field.

**`encodeCCBytes` case 3** (`C0 = 0x12`):  
- **C2 bit 6**: set when `alexConfig & 0x01` (manual). Hardware ignores C3/C4 filter bits unless this is set.
- **C3**: `protocol1AlexManualHpfByte(alexConfig)` + VNA in bit 7.
- **C4**: `protocol1AlexC4LpfByte(...)` — manual LPF from `alexConfig` bits `0x100`–`0x4000`; if manual but no LPF bit is set while transmitting, fall back to frequency auto-select; auto mode uses TX frequency only while MOX/PTT.

**Case 4** (`C0 = 0x1C`) is ADC assignment / Mercury step attenuator, **not** Alex LPF (comments above that case describe historical LPF layout).

**Protocol 2** maps the same `alexConfig` HPF/LPF layout into the Alex0 word (`CProtocol2.cpp`).

### UI

`AlexFilterWidget` HPF buttons must write the **Settings-canonical** bits above (not a C3-wire mirror). `setAlexConfig` syncs local LED bools from that bitfield.

---

## 3. Related source files (quick index)

| Area | Files |
|------|--------|
| P1 encode/decode | `src/DataEngine/CProtocol1.cpp`, `CProtocol1.h`, `protocol_boundary_utils.h` |
| Alex packing tests | `tests/alex_encoding_tests.cpp` |
| RX split 512+512 | `src/DataEngine/cusdr_dataEngine.cpp` (`processReadData`) |
| Alex UI / config | `src/cusdr_alexFilterWidget.cpp`, `cusdr_alexAntennaWidget.cpp`, `cusdr_mainWidget.cpp` |
| Settings persistence | `src/cusdr_settings.cpp`, `cusdr_settings.h` |
| P2 Alex bitfield | `src/DataEngine/CProtocol2.cpp` |
