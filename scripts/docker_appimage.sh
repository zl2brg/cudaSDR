#!/usr/bin/env bash
# Build the CI Docker image and run the same AppImage pipeline GitHub Actions uses.
# No host Qt mount — Qt 6.11 is installed inside the image via aqtinstall.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

IMAGE="${IMAGE:-cudasdr-ci:24.04}"
OUT_DIR="${OUT_DIR:-${ROOT}/out}"
DOCKERFILE="${DOCKERFILE:-${ROOT}/packaging/Dockerfile}"
JOBS="${JOBS:-$(nproc)}"
BUILD_IMAGE=1
EXTRA_DOCKER_ARGS=()

# Prefer docker, fall back to podman.
if command -v docker >/dev/null 2>&1; then
    DOCKER=(docker)
elif command -v podman >/dev/null 2>&1; then
    DOCKER=(podman)
else
    echo "ERROR: need docker or podman on PATH" >&2
    exit 1
fi

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --no-build-image   Reuse an existing ${IMAGE} (skip image build)
  --jobs N           Parallel compile jobs inside the container (default: nproc)
  -h, --help         Show this help

Environment:
  IMAGE       Image tag (default: cudasdr-ci:24.04)
  OUT_DIR     Host directory for AppImage output (default: ./out)
  DOCKERFILE  Dockerfile path (default: packaging/Dockerfile)
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-build-image) BUILD_IMAGE=0; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

if [[ "${BUILD_IMAGE}" -eq 1 ]]; then
    echo "==> Building image ${IMAGE} with ${DOCKER[*]} (Qt 6.11 download on first build)..."
    "${DOCKER[@]}" build -f "${DOCKERFILE}" -t "${IMAGE}" "${ROOT}/packaging"
fi

mkdir -p "${OUT_DIR}"

# Pass through a GitHub token if present so private submodule HTTPS clones work.
if [[ -n "${GH_TOKEN:-${GITHUB_TOKEN:-}}" ]]; then
    EXTRA_DOCKER_ARGS+=(-e "GH_TOKEN=${GH_TOKEN:-${GITHUB_TOKEN}}")
    EXTRA_DOCKER_ARGS+=(-e "GITHUB_TOKEN=${GH_TOKEN:-${GITHUB_TOKEN}}")
fi

echo "==> Running CI AppImage build in ${IMAGE}..."
"${DOCKER[@]}" run --rm \
    -e "JOBS=${JOBS}" \
    -e "BUILD_DIR=build-ci" \
    -e "OUTPUT_DIR=/out" \
    -e "APPIMAGE_EXTRACT_AND_RUN=1" \
    "${EXTRA_DOCKER_ARGS[@]}" \
    -v "${ROOT}:/src:rw" \
    -v "${OUT_DIR}:/out:rw" \
    -w /src \
    "${IMAGE}" \
    ./scripts/ci_appimage.sh

echo
echo "==> Done. Artifacts in ${OUT_DIR}:"
ls -lh "${OUT_DIR}"/*.AppImage 2>/dev/null || ls -lh "${OUT_DIR}"
