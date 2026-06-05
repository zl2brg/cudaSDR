## cudaSDR

`cudaSDR` is a Qt-based SDR client forked from N1GP's cudaSDR work, with ongoing modernization and protocol work.

### Current status

- **Modernized to Qt 6.11+**: Fully ported to modern Qt6 signal/slot syntax and Core Profile OpenGL.
- **Architectural Modularization**: Major components like `MainWindow` and `ReceiverPanel` have been decomposed into specialized modules for better maintainability.
- **Modern OpenGL Pipeline**: High-performance rendering using Shaders and VBOs, featuring optimized waterfall scrolling and transparent overlays.
- **Standardized Foundation**: Unidied networking via `QUdpSocket` and cross-platform threading/monitoring.
- **High-DPI Support**: Native scaling for 4K and Retina displays.
- **Advanced DSP**: Integrated WDSP, Protocol 2 support, and FreeDV/Codec2 capabilities - no rade yet.
- **Soapy Integration underway **
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
- Qt 6.11.x (`Widgets`, `Core`, `Gui`, `Multimedia`, `Network`, `OpenGL`, `OpenGLWidgets`)
- FFTW3 (`fftw3`, `fftw3f`)
- Liquid DSP
- OpenGL
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

### Run

After building:

```bash
./run_cudasdr.sh
```

Useful runtime overrides:

- `BUILD_DIR=build-qtfix-test ./run_cudasdr.sh`
- `Qt6_DIR=$HOME/Qt/6.11.0/gcc_64/lib/cmake/Qt6 ./run_cudasdr.sh`

### Docs

Protocol and WDSP references are in `Docs/`.

### Known gaps / TODO

- CW polish and validation






