# cudaSDR Codebase Evaluation and Improvement Plan

**Date**: September 2026  
**Status**: Active Architecture & Roadmap Document  

---

## 1. Executive Summary

**cudaSDR** has evolved from the original cuSDR / PowerSDR legacy architecture (C++, Qt4/Qt5, OpenCL, and raw CUDA acceleration) toward a modern SDR transceiver application built with **Qt 6**, modern **OpenGL 3.3+ Core profile** (shaders, VBOs/VAOs, and PBO waterfall texture streaming), and bundled **WDSP 2.0**.

Key system capabilities:
- Multi-receiver scaling (up to 8 slices).
- Dual-protocol OpenHPSDR hardware support (Protocol 1 and Protocol 2).
- Generic SDR support via SoapySDR (LimeSDR, PlutoSDR, RTL-SDR, etc.).
- FreeDV / Codec2 digital voice integration.
- Hardware and software CW keying engine (iambic, straight key, sidetone).
- External ecosystem integrations: **TCI Server** and **Hamlib rigctl Server**.

While modern and responsive, the codebase still carries architectural debt from legacy cuSDR:
- **Dual Source of Truth**: State split between the legacy `Settings` singleton (5,900+ lines) and the reactive MVC models (`RadioModel`, `SliceModel`, `TransmitModel`).
- **Monolithic UI Panels**: Massive GL panel classes (>2,500 lines) coupling rendering, geometry math, and mouse hit-testing.
- **Direct WDSP C-API Calls**: Scattered C calls without RAII lifetime guarantees.
- **Heterogeneous Threading**: Mixed mutex types and un-atomic shared variables accessed across real-time DSP threads.

---

## 2. Core Architectural Evaluation

### 2.1 State Management: Settings vs. MVC Models
- **Current State**: `Settings` acts simultaneously as a persistent configuration store, runtime property cache, and signal dispatcher. Parallel to this, reactive MVC models (`RadioModel`, `SliceModel`, `TransmitModel`) hold slice and transmit parameters.
- **Risk**: Bi-directional sync methods (`syncSlicesWithSettings()`, `syncSettingsWithSlices()`, `syncTransmitWithSettings()`, `syncSettingsWithTransmit()`) create race conditions, signal cascades, and initialization ordering bugs.
- **Target**: `RadioModel` and its sub-models become the sole runtime source of truth. `Settings` is demoted strictly to a disk serialization/deserialization utility.

### 2.2 DSP Architecture & WDSP Coupling
- **Current State**: Direct WDSP calls (`OpenChannel`, `SetChannelState`, `SetTXAMode`, `SetTXABandpassFreqs`, `fexchange0`, `Spectrum0`, `GetPixels`) are called directly throughout `Transmitter`, `SliceProcessor`, `DataProcessor`, and `QWDSPEngine`.
- **Risk**: Channel IDs (`TX_ID = 10`, RX `0..7`) are hardcoded, and parameter setters can crash if called before channel initialization.
- **Target**: Wrap WDSP channels in RAII C++ classes (`WdspRxChannel`, `WdspTxChannel`) that manage channel lifecycle, Wisdom, and parameter application safely.

### 2.3 Concurrency & Real-Time Threading
- **Current State**: Threads for Network I/O (`DataIO`), Stream Processing (`DataProcessor`), Slice DSP (`SliceProcessor`), Spectrum Decimation (`SpectrumBinWorker`), Audio I/O, and GUI communicate via both queued signals and raw member mutation.
- **Target**: Ensure all shared state variables checked on audio/DSP loops (e.g. `m_state`, `mox`, `transmitting`) are `std::atomic<T>` and lock guards are strictly RAII.

### 2.4 Hardware Abstraction Layer (HAL)
- **Current State**: Protocol 1 and Protocol 2 implement `IHPSDRProtocol`, but SoapySDR handling relies on `#ifdef HAVE_SOAPYSDR` preprocessor blocks inside slice processing and data engine classes.
- **Target**: Create a unified `ISdrDevice` abstraction interface encapsulating packet formatting, duplexing modes, and sample rate conversion.

### 2.5 OpenGL Rendering & UI Decomposition
- **Current State**: Rendering was modernized with shaders and VBOs (`PanadapterRenderer`, `WaterfallRenderer`, `OverlayRenderer`, `TraceRenderer`), but enclosing panels (`cusdr_oglReceiverPanel.cpp`, `cusdr_oglDisplayPanel.cpp`) still exceed 2,500 lines.
- **Target**: Extract input controllers (mouse drag, tuning, zoom, filter drag) from OpenGL display panels so panels are pure views.

---

## 3. Prioritized Improvement Roadmap

### Phase 1: High-Impact Stability & Code Hygiene (Completed)
1. **Clean Source Tree**:
   - Deleted leftover backup files (`src/QtWDSP/qtwdsp_dspEngine.cpp.save`, `src/rxpanel.bak`).
2. **Standardize Shared State Access**:
   - Converted critical cross-thread flags (`SliceProcessor::m_state`, etc.) to `std::atomic<RadioState>` to eliminate data races.
3. **Decouple Ham Database Tables**:
   - Moved ~1,400 lines of static arrays and lookup tables from `src/cusdr_hamDatabase.h` into a compiled `src/cusdr_hamDatabase.cpp` unit to reduce header bloat and compile times.
4. **Verify Build & Tests**:
   - Clean compilation and 100% pass rate across all 17 unit test suites.

### Phase 2: State Model Consolidation (Completed)
1. Established `RadioModel`, `SliceModel`, and `TransmitModel` as the single runtime authority.
2. Eliminated double-setting anti-patterns in `RadioPopupController` (mutates `SliceModel` directly; no duplicate `Settings` calls).
3. Consolidated `Settings` getters (`getFilterLo`, `getFilterHi`, `getAGCSlope`, `getAGCHangThreshold`, `getAGCHangLeveldB`, `getAGCMaximumGain_dB`, `getAGCFixedGain_dB`, `getMainVolume`, `getCwDecode`, etc.) to defer to active `SliceModel`.
4. Connected `SliceModel::filterChanged` and `SliceModel::dspModeChanged` to forward through `Settings` signals (`filterFrequenciesChanged`, `dspModeChanged`) so legacy decoupled listeners (such as `Transmitter`) receive immediate updates without circular loops.
5. Added automated unit tests in `tests/models_tests.cpp` validating live model getter delegation and signal forwarding.
6. Demoted `Settings` to pure configuration serialization and persistence (load on startup, save on shutdown).

### Phase 3: DSP & Device Layer Encapsulation
1. Implement `WdspRxChannel` and `WdspTxChannel` RAII classes.
2. Consolidate hardware sources behind an abstract `ISdrDevice` interface, removing `#ifdef HAVE_SOAPYSDR` branches from core DSP paths.

### Phase 4: UI Modernization & Modularization
1. Separate user interaction from OpenGL rendering by creating `PanadapterInputController`.
2. Consolidate duplicate dialog/tab settings and ensure full High-DPI support across renderers.

### Phase 5: Advanced SDR Capabilities
1. Integrate WDSP PureSignal (Adaptive Digital Predistortion) for transmitter linearization.
2. Implement automated diversity gain/phase processing for dual-ADC receivers.
