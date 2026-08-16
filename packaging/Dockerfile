# cudaSDR CI / AppImage build environment
# Same image for local docker runs and GitHub Actions.
# Qt 6.11.0 is installed inside the image via aqtinstall (no host Qt mount).
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive \
    QT_VERSION=6.11.0 \
    QT_PATH=/opt/Qt \
    APPIMAGE_EXTRACT_AND_RUN=1

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        file \
        git \
        libcodec2-dev \
        libfftw3-dev \
        libgl1-mesa-dev \
        libglib2.0-0 \
        libgstreamer-plugins-base1.0-0 \
        libgstreamer1.0-0 \
        libopengl-dev \
        libpulse0 \
        libsoapysdr-dev \
        libxcb-cursor0 \
        libxcb-icccm4 \
        libxcb-image0 \
        libxcb-keysyms1 \
        libxcb-render-util0 \
        libxcb-shape0 \
        libxcb-xinerama0 \
        libxkbcommon-dev \
        libxkbcommon-x11-0 \
        ninja-build \
        pkg-config \
        python3 \
        python3-pip \
        python3-pil \
        soapysdr-tools \
        wget \
        xz-utils \
    && rm -rf /var/lib/apt/lists/*

# Official Qt desktop tree (gcc_64). Keep aqt recent for 6.11 mirror layout.
RUN pip3 install --break-system-packages --no-cache-dir 'aqtinstall>=3.3.0' \
    && ARCH="$(aqt list-qt linux desktop --arch ${QT_VERSION} | tr ' ' '\n' | grep -E '^(linux_gcc_64|gcc_64)$' | head -1)" \
    && test -n "$ARCH" \
    && echo "aqt arch=$ARCH" \
    && aqt install-qt linux desktop ${QT_VERSION} "$ARCH" \
        -O ${QT_PATH} \
        -m qtmultimedia qtwebsockets qtshadertools qtimageformats \
    && rm -rf /root/.cache

# Autotools for freedv-backend ExternalProjects (opus / rnnoise autogen.sh).
# Fontconfig/freetype for Qt host tools (qsb, uic, …).
# Kept as a separate layer so Qt install stays cacheable when this list grows.
RUN apt-get update && apt-get install -y --no-install-recommends \
        autoconf \
        automake \
        fontconfig \
        libfontconfig1 \
        libfreetype6 \
        libportaudio2 \
        libportaudiocpp0 \
        libtool \
        m4 \
        portaudio19-dev \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

# Official Qt desktop tree (gcc_64). Keep aqt recent for 6.11 mirror layout.
RUN pip3 install --break-system-packages --no-cache-dir 'aqtinstall>=3.3.0' \
    && ARCH="$(aqt list-qt linux desktop --arch ${QT_VERSION} | tr ' ' '\n' | grep -E '^(linux_gcc_64|gcc_64)$' | head -1)" \
    && test -n "$ARCH" \
    && echo "aqt arch=$ARCH" \
    && aqt install-qt linux desktop ${QT_VERSION} "$ARCH" \
        -O ${QT_PATH} \
        -m qtmultimedia qtwebsockets qtshadertools qtimageformats \
    && rm -rf /root/.cache

ENV Qt6_DIR=${QT_PATH}/${QT_VERSION}/gcc_64/lib/cmake/Qt6 \
    CMAKE_PREFIX_PATH=${QT_PATH}/${QT_VERSION}/gcc_64 \
    PATH=${QT_PATH}/${QT_VERSION}/gcc_64/bin:${PATH} \
    LD_LIBRARY_PATH=${QT_PATH}/${QT_VERSION}/gcc_64/lib \
    QMAKE=${QT_PATH}/${QT_VERSION}/gcc_64/bin/qmake

WORKDIR /src

# Default entry: build Release + AppImage into /out
CMD ["./scripts/ci_appimage.sh"]
