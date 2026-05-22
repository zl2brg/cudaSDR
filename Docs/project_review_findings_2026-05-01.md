# cudaSDR Project Review Findings

## Scope

Repository-level technical review covering build health, maintainability, warning debt, documentation, protocol correctness, and stabilization opportunities.

---

## Review update (2026-05-18)

### Summary

The project has moved forward since the initial May review: **Qt6 / modern OpenGL** is the stated baseline, **legacy CUDA/OpenCL paths were removed**, and **networking + protocol layers were refactored**. Documentation and targeted unit tests were added. The codebase is still **active WIP** (README), with a few **integration gaps** and **known Protocol 1 Alex-encoding risks** called out in dedicated notes.

### What improved (since initial review)

| Area | Notes |
|------|--------|
| **Architecture** | Modular `DataEngine`, `IHPSDRProtocol` (P1/P2), `QUdpSocket`-based I/O; band/mode/filter switching refactor (recent commits). |
| **Rendering** | Shader/VBO waterfall and spectrum work (`236285f`); inactive panadapter polish. |
| **Audio** | Qt6-oriented audio engine; legacy ALSA dependency reduced (`67f6878`). |
| **Build** | Single CMake path; CUDA/OpenCL stripped from main target (`655e6e5`). |
| **Docs** | `README.md` quickstart; `Docs/protocol_and_alex_notes.md`; `Docs/codebase_architecture.md`. |
| **Tests** | `protocol_boundary_tests`, `protocol2_format_init_tests` when `BUILD_TESTING=ON` (see `CMakeLists.txt`). |
| **Correctness fixes** | TX spectrum / analyzer timing / RX frequency isolation (`728cdc8`). |

### Current risks / open items

1. **Protocol 1 — Alex filtering (high impact if using Alex on Metis/Hermes)**  
   Documented in [protocol_and_alex_notes.md](protocol_and_alex_notes.md):
   - C3 “RX antenna” may be driven from **RX aux** bits (`>> 2`) instead of `& 0x03`.
   - Manual **LPF** bits in `alexConfig` may not be encoded on P1 TX/RX paths; case 4 comments vs ADC/attenuator code mismatch.

2. **Unintegrated work in tree**  
   - Untracked `src/GL/WidebandRenderer.{h,cpp}` — not referenced in `CMakeLists.txt` (wideband still via `cusdr_oglWidebandPanel.cpp`).

3. **CI / reproducible build**  
   - No in-repo CI workflow observed; fresh configure requires **Qt 6.11** on `CMAKE_PREFIX_PATH` / `Qt6_DIR`.
   - Local `build/` contains binaries and test targets; running tests may need correct `LD_LIBRARY_PATH` (Qt 6.11 vs older `libQt6Core` on `PATH`).

4. **Feature matrix still informal**  
   README lists SoapySDR and CW validation as gaps; P1 vs P2 vs hardware (Metis/Hermes) support not tabulated in one place.

5. **Runtime artifacts**  
   - `build/cudaSDR.log`, `build/settings.ini` under build dir — avoid committing; consider `.gitignore` if not already.

### Suggested priorities (next)

1. Fix or verify **P1 Alex** encoding against `USB_protocol_V1.59.pdf` / hardware (see protocol notes).
2. Wire or drop **WidebandRenderer** (integrate in CMake + panel, or remove WIP files).
3. Add **GitHub Actions** (or similar): configure Release, build, run `ctest` with Qt 6.11.
4. Expand tests: P1 padding table per receiver count; Alex bit packing round-trip.
5. Publish a short **feature matrix** in `README` or `Docs/`.

### Related documentation

- [Protocol 1 / USB / Alex notes](protocol_and_alex_notes.md)
- [Codebase architecture diagrams](codebase_architecture.md)

---

## Initial review (2026-05-01)

### Initial assessment

- Build/configure was functional with CMake and Qt6 in this environment.
- Project status matched "active migration / work in progress" rather than fully hardened production state.
- Main risks identified:
  - Runtime/script mismatch (Qt5 hardcoding vs Qt6 build target).
  - High compile-warning volume masking real issues.
  - Thin first-party documentation and quickstart guidance.
  - Limited app-level automated tests compared with vendored library tests.

### Findings and actions taken (May 2026 stabilization pass)

#### 1) Runtime and environment consistency

- Updated `run_cudasdr.sh` to align with Qt6/CMake outputs.
- Removed hardcoded user/Qt5 paths.
- Added binary auto-discovery and clear missing-binary error.
- Added optional `Qt6_DIR`-based plugin/lib path derivation.

#### 2) Documentation quality

- Replaced minimal `README.md` text with a practical quickstart:
  - Current status
  - Prerequisites
  - Build and run commands
  - Useful runtime overrides
  - Known gaps/TODO

#### 3) Compile errors and correctness

- Fixed message handler signature mismatch in `src/main.cpp`:
  - `QLoggingCategory` argument -> `QMessageLogContext` for `qInstallMessageHandler`.
- Fixed suspicious boolean logic in `src/AudioEngine/cusdr_iambic.cpp`:
  - `!radioState != MOX` -> `radioState != MOX`.
- Fixed enum-in-boolean-context issue in `src/cusdr_transmitTabWidget.cpp`.

#### 4) Warning debt reduction

- Reduced significant warning noise across multiple files by:
  - removing unused variables/parameters,
  - adding `Q_UNUSED(...)` where parameters are intentionally unused,
  - correcting several constructor init-order issues,
  - adding missing enum handling for `QSDR::SoapySDR` in switch statements.
- Addressed deprecated usage warnings:
  - Replaced `QScopedPointer::take()` layout ownership patterns.
  - Added explicit default copy-assignment for `DoubleColor` in `cusdr_colorTriangle.cpp`.

### Status after May cleanup (historical)

- Full clean rebuild succeeded in the review environment (`build-eval`).
- Major warning classes from the reviewed paths were reduced/eliminated.
- Remaining warnings were expected to be legacy and lower-priority.

### Original suggested next steps

1. Add CI for configure + full rebuild in Debug/Release. — **partially addressed** by local tests; **CI still open**
2. Introduce app-level tests around `DataEngine`/protocol boundaries. — **started** (`protocol_*_tests`)
3. Gradually tighten warning policy (`-Werror` in selected modules first). — **open**
4. Document feature support matrix (P1/P2, Soapy status, CW status). — **open**
