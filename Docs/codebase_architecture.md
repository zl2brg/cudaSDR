# cudaSDR codebase architecture

High-level views of the repository layout and runtime structure. Diagrams use [Mermaid](https://mermaid.js.org/) (render in GitHub, VS Code, or Cursor preview).

---

## 1. Repository layout (physical)

```mermaid
flowchart TB
  subgraph root["cudaSDR /"]
    CM[CMakeLists.txt]
    README[README.md]
    RUN[run_cudasdr.sh]
    DOC[Docs/]
  end

  subgraph app["Application source"]
    SRC[src/]
  end

  subgraph dsp["Bundled DSP"]
    WDSP[wdsp-1.29/]
    WLIB[wdsp-libs/]
  end

  subgraph other["Tools & sim"]
    TESTS[tests/]
    HPSIM[hpsdrsim/]
    RES[res/]
  end

  CM --> SRC
  SRC --> WDSP
  SRC --> WLIB
  CM --> TESTS
```

---

## 2. `src/` module map (logical)

```mermaid
flowchart LR
  subgraph UI["UI shell & widgets"]
    MW[MainWindow]
    MWUI[MainWindowUI — menu, toolbar, actions]
    RW[cusdr_radioWidget + tabs]
    UIpkg[UI/ — setup, filters, tx dialog, …]
    MW --> MWUI
    MW --> RW
    RW --> UIpkg
  end

  subgraph CFG["Configuration"]
    SET[Settings singleton]
    HDB[cusdr_hamDatabase]
    MW --> SET
    SET --> HDB
  end

  subgraph DE["DataEngine"]
    DEng[DataEngine]
    DIO[DataIO — UDP, discovery]
    DProc[DataProcessor]
    REC[Receiver × N]
    TX[Transmitter]
    P1[CProtocol1]
    P2[CProtocol2]
    WB[WidebandProcessor]
    DEng --> DIO
    DEng --> DProc
    DProc --> REC
    DEng --> TX
    DIO --> P1
    DIO --> P2
    DEng --> WB
  end

  subgraph DSPB["QtWDSP bridge"]
    QWDSP[qtwdsp_dspEngine]
    REC --> QWDSP
  end

  subgraph AE["AudioEngine"]
    AIN[audio input / iambic]
    AOUT[audiooutputmanager]
    CODEC[codec2 / freedv optional]
  end

  subgraph GL["GL — spectrum & panadapter"]
    RP[oglReceiverPanel]
    WR[WaterfallRenderer]
    PR[PanadapterRenderer]
    OR[OverlayRenderer]
    RP --> WR
    RP --> PR
    RP --> OR
    OGL[wideband, 3D, …]
  end

  subgraph UT["Util"]
    RIG[rigctl server]
    CPU[CPUMonitor]
    SPL[splash, timers, …]
  end

  SET <--> DEng
  MW <--> DEng
  REC --> RP
  DEng --> AE
  MW --> AE
  QWDSP --> WDSP[(wdsp C lib)]
```

---

## 3. IQ / control data path (simplified)

```mermaid
sequenceDiagram
  participant Net as Network Metis/Hermes
  participant DIO as DataIO
  participant Q as iq_queue
  participant DP as DataProcessor
  participant P as CProtocol1/2
  participant RX as Receiver
  participant WDSP as qtwdsp / wdsp
  participant GL as OpenGL panels

  Net->>DIO: UDP datagrams
  DIO->>Q: enqueue payload
  DP->>Q: dequeue
  DP->>P: processInputBuffer
  P->>RX: raw IQ / state
  RX->>WDSP: DSP chain
  WDSP-->>RX: audio / FFT buffers
  RX->>GL: spectrum / waterfall
```

---

## 4. Key directories (quick reference)

| Path | Role |
|------|------|
| `src/main.cpp` | `QApplication`, logging, `MainWindow` |
| `src/cusdr_settings.*` | Global settings, persistence, signals |
| `src/Settings/` | Modularized configuration (Network, Hardware, etc.) |
| `src/DataEngine/` | Radio I/O, protocols, receivers, TX |
| `src/QtWDSP/` | C++ bridge to WDSP demod/modem chain |
| `src/AudioEngine/` | Sound devices, CW keyer, FreeDV/Codec2 |
| `src/GL/` | OpenGL UI with specialized sub-renderers |
| `src/UI/MainWindow/` | MainWindow UI decomposition (`MainWindowUI`) |
| `src/UI/` | Smaller dialogs and embedded widgets |
| `src/Util/` | Rig control, splash, CPU monitor, timers |
| `wdsp-1.29/` | Upstream-style WDSP sources |
| `wdsp-libs/` | Prebuilt / vendored libs (e.g. rnnoise, specbleach) |
| `tests/` | CMake `BUILD_TESTING` unit tests |
| `hpsdrsim/` | Simulator companion project |
| `Docs/` | Project notes, protocol write-ups |

---

*Generated for navigation and onboarding; refine subgraphs as subsystems move or split.*
