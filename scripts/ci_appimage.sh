#!/usr/bin/env bash
# Build cudaSDR (Release) and package an AppImage.
# Intended to run inside packaging/Dockerfile (local docker or GitHub Actions).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

BUILD_DIR="${BUILD_DIR:-build-ci}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
OUTPUT_DIR="${OUTPUT_DIR:-${ROOT}/out}"
QT_VERSION="${QT_VERSION:-6.11.0}"
QT_PREFIX="${QT_PREFIX:-/opt/Qt/${QT_VERSION}/gcc_64}"
JOBS="${JOBS:-$(nproc)}"

log() { printf '==> %s\n' "$*"; }
die() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

export APPIMAGE_EXTRACT_AND_RUN="${APPIMAGE_EXTRACT_AND_RUN:-1}"

if [[ ! -f "${QT_PREFIX}/lib/cmake/Qt6/Qt6Config.cmake" ]]; then
    die "Qt ${QT_VERSION} not found at ${QT_PREFIX}. Build/run the packaging/Dockerfile image."
fi

export PATH="${QT_PREFIX}/bin:${PATH}"
export LD_LIBRARY_PATH="${QT_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export Qt6_DIR="${QT_PREFIX}/lib/cmake/Qt6"
export CMAKE_PREFIX_PATH="${QT_PREFIX}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export QMAKE="${QT_PREFIX}/bin/qmake"

log "Qt: ${QT_PREFIX} (qmake=$("${QMAKE}" -query QT_VERSION))"

# Bind-mounted worktrees are often owned by the host user while the container
# runs as root — mark the tree safe so submodule/update commands work.
git config --global --add safe.directory "${ROOT}"
git config --global --add safe.directory '*'

TOKEN="${GH_TOKEN:-${GITHUB_TOKEN:-}}"

# Never use `git submodule update` here: .gitmodules / local gitlink history may
# still point at ssh:// and the container has no ssh client. Clone via HTTPS.
ensure_freedv_backend() {
	local path="deps/freedv-backend"
	local https_url="https://github.com/zl2brg/freedv-backend.git"
	local clone_url="${https_url}"
	local commit=""

	if [[ -n "${TOKEN}" ]]; then
		clone_url="https://x-access-token:${TOKEN}@github.com/zl2brg/freedv-backend.git"
	fi

	if [[ -f "${path}/CMakeLists.txt" ]]; then
		log "Using existing ${path}"
		return 0
	fi

	[[ -e .git ]] || die "${path} missing and /src is not a git checkout"

	commit="$(git ls-tree HEAD "${path}" | awk '{print $3}')"
	[[ -n "${commit}" ]] || die "No gitlink commit found for ${path}"

	log "Cloning ${path} via HTTPS @ ${commit}..."
	rm -rf "${path}"
	mkdir -p "$(dirname "${path}")"
	git clone --no-checkout "${clone_url}" "${path}"
	git -C "${path}" checkout --force "${commit}"
}

ensure_freedv_backend

log "Configuring (${BUILD_TYPE}) in ${BUILD_DIR}..."
cmake \
    -S "${ROOT}" \
    -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -DCMAKE_FIND_ROOT_PATH="${QT_PREFIX}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

log "Building (jobs=${JOBS})..."
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

mkdir -p "${OUTPUT_DIR}"
export BUILD_DIR
export OUTPUT_DIR
export TOOLS_DIR="${TOOLS_DIR:-${ROOT}/.tools-ci}"
export APPDIR="${APPDIR:-${ROOT}/AppDir-ci}"
export LDA_OUTPUT_VERSION="${LDA_OUTPUT_VERSION:-$(git describe --tags --always --dirty 2>/dev/null || date +%Y%m%d)}"

log "Packaging AppImage into ${OUTPUT_DIR}..."
"${ROOT}/scripts/make_appimage.sh"

# Prefer artifacts under OUTPUT_DIR; make_appimage may also leave copies in ROOT.
shopt -s nullglob
artifacts=("${OUTPUT_DIR}"/*.AppImage)
if [[ ${#artifacts[@]} -eq 0 ]]; then
    artifacts=("${ROOT}"/*cudaSDR*.AppImage "${ROOT}"/*cudasdr*.AppImage)
fi
shopt -u nullglob

[[ ${#artifacts[@]} -gt 0 ]] || die "No AppImage produced"

log "AppImage artifact(s):"
for f in "${artifacts[@]}"; do
    # Ensure final copies live under OUTPUT_DIR for the docker mount / GHA upload.
    if [[ "$(dirname "${f}")" != "${OUTPUT_DIR}" ]]; then
        cp -f "${f}" "${OUTPUT_DIR}/"
        f="${OUTPUT_DIR}/$(basename "${f}")"
    fi
    ls -lh "${f}"
done
