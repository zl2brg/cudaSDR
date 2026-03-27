#!/usr/bin/env bash
set -euo pipefail

# ---------------------------------------------------------------------------
# Build script for cudaSDR
# Detects Qt 6.9.3 and runs CMake
# ---------------------------------------------------------------------------

REQUIRED_QT_VERSION="6.9.3"
BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc)}"

# Common Qt installation prefixes to search
QT_SEARCH_PATHS=(
    "$HOME/Qt/${REQUIRED_QT_VERSION}/gcc_64"
    "$HOME/Qt/${REQUIRED_QT_VERSION}/linux_gcc_64"
    "/opt/Qt/${REQUIRED_QT_VERSION}/gcc_64"
    "/usr/local/Qt-${REQUIRED_QT_VERSION}"
    "/usr/lib/qt6"
)

find_qt() {
    local prefix version cmake_dir qmake_bin

    # Helper: extract version from Qt6Targets.cmake (where Qt actually stores it)
    _qt_version_from_prefix() {
        local p="$1"
        grep -oP '_qt_package_version "\K[^"]+' \
            "${p}/lib/cmake/Qt6/Qt6Targets.cmake" 2>/dev/null | head -1
    }

    # 1. Honour Qt6_DIR env var
    if [[ -n "${Qt6_DIR:-}" ]]; then
        # Qt6_DIR may point to lib/cmake/Qt6 or the prefix itself
        for candidate in "${Qt6_DIR}" "${Qt6_DIR}/../../.."; do
            candidate=$(realpath -m "${candidate}" 2>/dev/null || echo "${candidate}")
            version=$(_qt_version_from_prefix "${candidate}")
            if [[ "${version}" == "${REQUIRED_QT_VERSION}" ]]; then
                echo "${candidate}"
                return 0
            fi
        done
    fi

    # 2. Search known installation paths
    for prefix in "${QT_SEARCH_PATHS[@]}"; do
        cmake_dir="${prefix}/lib/cmake/Qt6"
        if [[ -f "${cmake_dir}/Qt6Config.cmake" ]]; then
            version=$(_qt_version_from_prefix "${prefix}")
            if [[ "${version}" == "${REQUIRED_QT_VERSION}" ]]; then
                echo "${prefix}"
                return 0
            fi
        fi
    done

    # 3. Try qmake6 / qmake on PATH
    for qmake_bin in qmake6 qmake; do
        if command -v "${qmake_bin}" &>/dev/null; then
            version=$("${qmake_bin}" -query QT_VERSION 2>/dev/null || true)
            if [[ "${version}" == "${REQUIRED_QT_VERSION}" ]]; then
                echo "$("${qmake_bin}" -query QT_INSTALL_PREFIX)"
                return 0
            fi
        fi
    done

    return 1
}

echo "==> Searching for Qt ${REQUIRED_QT_VERSION}..."

QT_PREFIX=""
if QT_PREFIX=$(find_qt); then
    echo "==> Found Qt ${REQUIRED_QT_VERSION} at: ${QT_PREFIX}"
else
    echo "ERROR: Qt ${REQUIRED_QT_VERSION} not found."
    echo
    echo "Please install Qt ${REQUIRED_QT_VERSION} and either:"
    echo "  - Set Qt6_DIR to the Qt6 cmake directory, e.g.:"
    echo "      export Qt6_DIR=\$HOME/Qt/${REQUIRED_QT_VERSION}/gcc_64/lib/cmake/Qt6"
    echo "  - Or add the Qt bin directory to PATH"
    exit 1
fi

CMAKE_PREFIX="${QT_PREFIX}/lib/cmake"

echo "==> Configuring (BUILD_TYPE=${BUILD_TYPE})..."
cmake \
    -S "$(dirname "$0")" \
    -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -DCMAKE_FIND_ROOT_PATH="${QT_PREFIX}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "==> Building (jobs=${JOBS})..."
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

echo
echo "==> Build complete. Binary in: ${BUILD_DIR}/"
