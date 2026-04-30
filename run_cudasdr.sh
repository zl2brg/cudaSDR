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

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
export QT_XCB_GL_INTEGRATION="${QT_XCB_GL_INTEGRATION:-xcb_egl}"
export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-}"

# If Qt6_DIR is set (as suggested by build.sh), derive runtime plugin/lib paths.
if [[ -n "${Qt6_DIR:-}" ]]; then
    QT_PREFIX="$(realpath -m "${Qt6_DIR}/../../.." 2>/dev/null || true)"
    if [[ -d "${QT_PREFIX}/plugins/platforms" ]]; then
        export QT_QPA_PLATFORM_PLUGIN_PATH="${QT_PREFIX}/plugins/platforms"
    fi
    if [[ -d "${QT_PREFIX}/lib" ]]; then
        export LD_LIBRARY_PATH="${QT_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    fi
fi

echo "Starting cudasdr..."
echo "  binary: ${BINARY_PATH}"
echo "  QT_QPA_PLATFORM=${QT_QPA_PLATFORM}"
echo "  QT_QPA_PLATFORM_PLUGIN_PATH=${QT_QPA_PLATFORM_PLUGIN_PATH:-<default>}"

exec "${BINARY_PATH}" "$@"
