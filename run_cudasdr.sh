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

# Display platform:
# Native Wayland + NVIDIA often busy-spins (libEGL dri2 failures → no real vsync) and
# can push CPU past 200%. Prefer X11/xcb (XWayland on a Wayland session) unless the
# user explicitly forces Wayland: QT_QPA_PLATFORM=wayland ./run_cudasdr.sh
if [[ -z "${QT_QPA_PLATFORM:-}" ]]; then
    if [[ -n "${WAYLAND_DISPLAY:-}" ]]; then
        echo "==> Wayland session detected; defaulting QT_QPA_PLATFORM=xcb (XWayland)"
        echo "    Set QT_QPA_PLATFORM=wayland to force native Wayland (higher CPU risk on NVIDIA)."
    fi
    export QT_QPA_PLATFORM=xcb
fi
# xcb_glx: best 3D panadapter (desktop GL). xcb_egl: legacy EGL path.
# 2D pan/waterfall use direct GL and work on either after NoPartialUpdate + pan GL fallback.
export QT_XCB_GL_INTEGRATION="${QT_XCB_GL_INTEGRATION:-xcb_glx}"

# NVIDIA: steer GL/EGL onto the proprietary vendor path. Without this, Mesa may probe the
# NVIDIA PCI id, log dri2 failures, and leave a high-CPU fallback path.
if [[ -e /proc/driver/nvidia/version ]] || lsmod 2>/dev/null | grep -q '^nvidia\b'; then
    export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
    export GBM_BACKEND="${GBM_BACKEND:-nvidia-drm}"
    echo "==> NVIDIA driver detected; using __GLX_VENDOR_LIBRARY_NAME=${__GLX_VENDOR_LIBRARY_NAME} GBM_BACKEND=${GBM_BACKEND}"
fi

# --- Qt 6.11+ Detection Logic ---
REQUIRED_QT_VERSION="6.11.0"
QT_SEARCH_PATHS=(
    "$HOME/Qt/${REQUIRED_QT_VERSION}/gcc_64"
    "$HOME/Qt/${REQUIRED_QT_VERSION}/linux_gcc_64"
    "/opt/Qt/${REQUIRED_QT_VERSION}/gcc_64"
    "/usr/local/Qt-${REQUIRED_QT_VERSION}"
    "/usr/lib/qt6"
)

version_satisfies_requirement() {
    [[ -n "$1" ]] || return 1
    [[ "$(printf '%s\n%s\n' "$1" "${REQUIRED_QT_VERSION}" | sort -V | head -n 1)" == "${REQUIRED_QT_VERSION}" ]]
}

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
            if version_satisfies_requirement "${version}"; then
                echo "${candidate}"
                return 0
            fi
        done
    fi

    for prefix in "${QT_SEARCH_PATHS[@]}"; do
        if [[ -f "${prefix}/lib/cmake/Qt6/Qt6Config.cmake" ]]; then
            version=$(_qt_version_from_prefix "${prefix}")
            if version_satisfies_requirement "${version}"; then
                echo "${prefix}"
                return 0
            fi
        fi
    done
    return 1
}

if QT_PREFIX=$(find_qt); then
    echo "==> Using Qt ${REQUIRED_QT_VERSION}+ at: ${QT_PREFIX}"
    export QT_QPA_PLATFORM_PLUGIN_PATH="${QT_PREFIX}/plugins"
    export LD_LIBRARY_PATH="${QT_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
else
    echo "WARNING: Qt ${REQUIRED_QT_VERSION}+ not found. Launch might fail due to library conflicts."
fi

echo "Starting cudasdr..."
echo "  binary: ${BINARY_PATH}"
echo "  QT_QPA_PLATFORM=${QT_QPA_PLATFORM}"
echo "  QT_XCB_GL_INTEGRATION=${QT_XCB_GL_INTEGRATION}"
echo "  __GLX_VENDOR_LIBRARY_NAME=${__GLX_VENDOR_LIBRARY_NAME:-<unset>}"
echo "  GBM_BACKEND=${GBM_BACKEND:-<unset>}"
echo "  QT_QPA_PLATFORM_PLUGIN_PATH=${QT_QPA_PLATFORM_PLUGIN_PATH:-<default>}"

exec "${BINARY_PATH}" "$@"
