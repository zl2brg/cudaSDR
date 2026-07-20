#!/usr/bin/env bash
set -euo pipefail

# Runtime launcher aligned with Qt6/CMake build outputs.

BUILD_DIR="${BUILD_DIR:-build}"
BINARY_CANDIDATES=(
    "${BUILD_DIR}/cudasdr"
    "${BUILD_DIR}/Debug/cudasdr"
    "${BUILD_DIR}/Release/cudasdr"
)

find_binary() {
    local candidate
    for candidate in "${BINARY_CANDIDATES[@]}"; do
        if [[ -x "${candidate}" ]]; then
            echo "${candidate}"
            return 0
        fi
    done
    return 1
}

if ! BINARY_PATH="$(find_binary)"; then
    echo "ERROR: cudasdr binary not found."
    echo "Searched:"
    printf "  - %s\n" "${BINARY_CANDIDATES[@]}"
    echo "Build first with: ./build.sh"
    exit 1
fi

# Detect display server and dynamically choose best platform with fallbacks
if [[ -n "${WAYLAND_DISPLAY:-}" ]]; then
    export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-wayland;xcb}"
else
    export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
fi
# xcb_glx: best 3D panadapter (desktop GL). xcb_egl: legacy EGL path.
# 2D pan/waterfall use direct GL and work on either after NoPartialUpdate + pan GL fallback.
export QT_XCB_GL_INTEGRATION="${QT_XCB_GL_INTEGRATION:-xcb_glx}"

# --- Qt 6.11.0 Detection Logic ---
REQUIRED_QT_VERSION="6.11.0"
QT_SEARCH_PATHS=(
    "$HOME/Qt/${REQUIRED_QT_VERSION}/gcc_64"
    "$HOME/Qt/${REQUIRED_QT_VERSION}/linux_gcc_64"
    "/opt/Qt/${REQUIRED_QT_VERSION}/gcc_64"
    "/usr/local/Qt-${REQUIRED_QT_VERSION}"
    "/usr/lib/qt6"
)

find_qt() {
    local prefix version
    _qt_version_from_prefix() {
        local p="$1"
        grep -oP '_qt_package_version "\K[^"]+' \
            "${p}/lib/cmake/Qt6/Qt6Targets.cmake" 2>/dev/null | head -1
    }

    if [[ -n "${Qt6_DIR:-}" ]]; then
        for candidate in "${Qt6_DIR}" "${Qt6_DIR}/../../.."; do
            candidate=$(realpath -m "${candidate}" 2>/dev/null || echo "${candidate}")
            version=$(_qt_version_from_prefix "${candidate}")
            if [[ "${version}" == "${REQUIRED_QT_VERSION}" ]]; then
                echo "${candidate}"
                return 0
            fi
        done
    fi

    for prefix in "${QT_SEARCH_PATHS[@]}"; do
        if [[ -f "${prefix}/lib/cmake/Qt6/Qt6Config.cmake" ]]; then
            version=$(_qt_version_from_prefix "${prefix}")
            if [[ "${version}" == "${REQUIRED_QT_VERSION}" ]]; then
                echo "${prefix}"
                return 0
            fi
        fi
    done
    return 1
}

if QT_PREFIX=$(find_qt); then
    echo "==> Using Qt ${REQUIRED_QT_VERSION} at: ${QT_PREFIX}"
    export QT_QPA_PLATFORM_PLUGIN_PATH="${QT_PREFIX}/plugins"
    export LD_LIBRARY_PATH="${QT_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
else
    echo "WARNING: Qt ${REQUIRED_QT_VERSION} not found. Launch might fail due to library conflicts."
fi

echo "Starting cudasdr..."
echo "  binary: ${BINARY_PATH}"
echo "  QT_QPA_PLATFORM=${QT_QPA_PLATFORM}"
echo "  QT_QPA_PLATFORM_PLUGIN_PATH=${QT_QPA_PLATFORM_PLUGIN_PATH:-<default>}"

exec "${BINARY_PATH}" "$@"
