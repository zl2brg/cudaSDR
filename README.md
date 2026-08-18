## cudaSDR

`cudaSDR` was originally written by Hermann von Hasseln (DL1HVH/DL3HVH). Modernised and maintained by Simon Eatough (ZL2BRG/G4GVQ). It is a Qt-based SDR with ongoing modernization and protocol work.

### Current status

- **Modernized to Qt 6.11+**: Fully ported to modern Qt6 signal/slot syntax and Core Profile OpenGL.
- **Architectural Modularization**: Major components like `MainWindow` and `ReceiverPanel` have been decomposed into specialized modules for better maintainability.
- **Modern OpenGL Pipeline**: High-performance rendering using Shaders and VBOs, featuring optimized waterfall scrolling and transparent overlays.
- **Standardized Foundation**: Unidied networking via `QUdpSocket` and cross-platform threading/monitoring.
- **High-DPI Support**: Native scaling for 4K and Retina displays.
- **Advanced DSP**: Integrated **WDSP 2.0** (NURBS EQ/CFC, NR1–NR4, EMNR post2), Protocol 2 support, and FreeDV/Codec2 capabilities - no rade yet.
- **Experimental Soapy Integration  **
- ** TCI server **
- AI Development assist from 2025 on


This project is still work in progress and very much alpha.

### Credits

- Hermann von Hasseln (DL1HVH/DL3HVH)
- Moe Wheatley (AE4JY)
- John Melton (G0ORH/N6YT)
- N1GP cudaSDR upstream contributors

### Prerequisites

- Linux
- CMake 3.16+
- C++17 compiler (GCC/Clang)
- Qt 6.11.x (`Widgets`, `Core`, `Gui`, `Multimedia`, `Network`, `OpenGL`, `OpenGLWidgets`, `WebSockets`, `Test`)
- FFTW3 (`fftw3`, `fftw3f`)
- Liquid DSP
- OpenGL
- Bundled `wdsp-2.00/` (WDSP 2.0 by NR0V; built via CMake — do not use the removed `wdsp-1.29` tree)
- Optional: Codec2 (enables FreeDV integration)

### Build

Use the provided build helper:

```bash
./build.sh
```

Or run CMake directly:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

On Ubuntu/Debian, install Qt dev packages including WebSockets:

```bash
sudo apt install qt6-websockets-dev
```

### Coverage

Requires `lcov` and `genhtml` in addition to the normal build dependencies:

```bash
sudo apt install lcov
./scripts/run_coverage.sh
```

Optional: reuse an existing build directory:

```bash
./scripts/run_coverage.sh build
```

### Run

After building:

```bash
./run_cudasdr.sh
```

Useful runtime overrides:

- `BUILD_DIR=build-qtfix-test ./run_cudasdr.sh`
- `Qt6_DIR=$HOME/Qt/6.11.0/gcc_64/lib/cmake/Qt6 ./run_cudasdr.sh`

### AppImage

Portable Linux builds use **Ubuntu 24.04 + Qt 6.11 inside Docker** — the same path
locally and on GitHub Actions. Qt is downloaded into the image with `aqtinstall`
(no host Qt mount).

#### Local (matches GHA)

```bash
# First run builds the image (Qt download, cached thereafter) then packages.
./scripts/docker_appimage.sh

# Reuse an existing image
./scripts/docker_appimage.sh --no-build-image
```

AppImages land in `./out/`.

#### Host packaging (optional)

If you already have a Release build and Qt 6.11 on the host:

```bash
./scripts/make_appimage.sh
./scripts/make_appimage.sh --build
```

#### GitHub Actions

Workflow: `.github/workflows/appimage.yml`

- Triggers: `workflow_dispatch`, tags `v*`
- Builds `packaging/Dockerfile`, runs `scripts/ci_appimage.sh`, uploads the AppImage
- On `v*` tags, attaches the AppImage to the GitHub Release

If `deps/freedv-backend` is private, add a repo secret `GH_PAT` with `repo` scope
so the HTTPS submodule clone works.

### Docs

Protocol and WDSP references are in `Docs/`.

### Known gaps / TODO

- CW polish and validation






