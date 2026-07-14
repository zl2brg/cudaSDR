#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build-coverage}"

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "$1 is required but was not found in PATH" >&2
        exit 1
    fi
}

require_qt_websockets_dev() {
    local candidates=()
    if [[ -n "${Qt6_DIR:-}" ]]; then
        candidates+=("$(dirname "${Qt6_DIR}")/../Qt6WebSockets/Qt6WebSocketsConfig.cmake")
    fi
    candidates+=(
        "/usr/lib/x86_64-linux-gnu/cmake/Qt6WebSockets/Qt6WebSocketsConfig.cmake"
        "/usr/lib/cmake/Qt6WebSockets/Qt6WebSocketsConfig.cmake"
    )

    for candidate in "${candidates[@]}"; do
        if [[ -f "${candidate}" ]]; then
            return 0
        fi
    done

    cat >&2 <<'EOF'
Qt6 WebSockets development files were not found.

The coverage script configures a full cudaSDR + test build, which requires Qt6WebSockets.

Install the dev package (system Qt on Ubuntu/Debian):
  sudo apt install qt6-websockets-dev

Or, if you use the Qt online installer, add the WebSockets module and set Qt6_DIR:
  export Qt6_DIR=$HOME/Qt/6.11.0/gcc_64/lib/cmake/Qt6
EOF
    exit 1
}

require_cmd cmake
require_cmd lcov
require_cmd genhtml
require_qt_websockets_dev

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DCUDASDR_ENABLE_COVERAGE=ON \
    ${Qt6_DIR:+-DQt6_DIR="${Qt6_DIR}"}

cmake --build "${BUILD_DIR}" -j"$(nproc)"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

lcov --directory "${BUILD_DIR}" --capture --output-file "${BUILD_DIR}/coverage.info"
lcov --remove "${BUILD_DIR}/coverage.info" '/usr/*' '*/tests/*' --output-file "${BUILD_DIR}/coverage.info"
genhtml "${BUILD_DIR}/coverage.info" --output-directory "${BUILD_DIR}/coverage-html"

echo "Coverage report: ${BUILD_DIR}/coverage-html/index.html"
