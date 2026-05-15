# cudaSDR Project Review Findings (2026-05-01)

## Scope

Repository-level technical review covering build health, maintainability, warning debt, documentation, and stabilization opportunities.

## Initial assessment

- Build/configure was functional with CMake and Qt6 in this environment.
- Project status matched "active migration / work in progress" rather than fully hardened production state.
- Main risks identified:
  - Runtime/script mismatch (Qt5 hardcoding vs Qt6 build target).
  - High compile-warning volume masking real issues.
  - Thin first-party documentation and quickstart guidance.
  - Limited app-level automated tests compared with vendored library tests.

## Findings and actions taken

### 1) Runtime and environment consistency

- Updated `run_cudasdr.sh` to align with Qt6/CMake outputs.
- Removed hardcoded user/Qt5 paths.
- Added binary auto-discovery and clear missing-binary error.
- Added optional `Qt6_DIR`-based plugin/lib path derivation.

### 2) Documentation quality

- Replaced minimal `README.md` text with a practical quickstart:
  - Current status
  - Prerequisites
  - Build and run commands
  - Useful runtime overrides
  - Known gaps/TODO

### 3) Compile errors and correctness

- Fixed message handler signature mismatch in `src/main.cpp`:
  - `QLoggingCategory` argument -> `QMessageLogContext` for `qInstallMessageHandler`.
- Fixed suspicious boolean logic in `src/AudioEngine/cusdr_iambic.cpp`:
  - `!radioState != MOX` -> `radioState != MOX`.
- Fixed enum-in-boolean-context issue in `src/cusdr_transmitTabWidget.cpp`.

### 4) Warning debt reduction

- Reduced significant warning noise across multiple files by:
  - removing unused variables/parameters,
  - adding `Q_UNUSED(...)` where parameters are intentionally unused,
  - correcting several constructor init-order issues,
  - adding missing enum handling for `QSDR::SoapySDR` in switch statements.
- Addressed deprecated usage warnings:
  - Replaced `QScopedPointer::take()` layout ownership patterns.
  - Added explicit default copy-assignment for `DoubleColor` in `cusdr_colorTriangle.cpp`.

## Current status after cleanup

- Full clean rebuild succeeds:
  - `cmake --build build-eval --target clean`
  - `cmake --build build-eval --parallel 4`
- Major warning classes from the reviewed paths were reduced/eliminated.
- Remaining warnings, if any, are expected to be legacy and lower-priority compared with originally identified issues.

## Suggested next steps (future)

1. Add CI for configure + full rebuild in Debug/Release.
2. Introduce app-level tests around `DataEngine`/protocol boundaries.
3. Gradually tighten warning policy (`-Werror` in selected modules first).
4. Document feature support matrix (P1/P2, Soapy status, CW status).

## Related notes

- [Protocol 1 / USB doc comparison and Alex filtering](protocol_and_alex_notes.md) — Metis vs USB PDF scope, padding table, and Alex `alexConfig` / `alexStates` encoding caveats.

