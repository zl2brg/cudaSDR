#!/usr/bin/env bash
# Build a cudaSDR .deb (Ubuntu 24.04 / KDE neon).
#
# Qt 6.12 is not packaged by the distro, so Qt and any other library that no
# installed package provides (Qt's ICU/FFmpeg, liquid, rade) are bundled under
# /opt/cudasdr. Everything the distro ships is removed from the bundle and
# becomes a Depends: entry via dpkg-shlibdeps.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

BUILD_DIR="${BUILD_DIR:-build}"
OUTPUT_DIR="${OUTPUT_DIR:-${ROOT}/out}"
APPDIR="${DEB_APPDIR:-${ROOT}/AppDir-deb}"
PKGROOT="${PKGROOT:-${ROOT}/debroot}"
PKG_NAME="cudasdr"
INSTALL_PREFIX="/opt/cudasdr"
MAINTAINER="${DEB_MAINTAINER:-Simon Eatough <simon.eatough@gmail.com>}"
HOMEPAGE="https://github.com/zl2brg/cudaSDR"

APPIMAGE_ARGS=()

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --build           Run ./build.sh before packaging
  --skip-download   Do not download linuxdeploy tools (must already exist)
  -h, --help        Show this help

Environment:
  BUILD_DIR         CMake build directory (default: build)
  OUTPUT_DIR        Where the .deb is written (default: ./out)
  DEB_APPDIR        Scratch AppDir (default: ./AppDir-deb)
  PKGROOT           Scratch package root (default: ./debroot)
  DEB_VERSION       Override the package version (default: from git describe)
  DEB_MAINTAINER    Override the Maintainer field
  TOOLS_DIR, QMAKE, Qt6_DIR, LINUXDEPLOY are passed to make_appimage.sh
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build|--skip-download) APPIMAGE_ARGS+=("$1"); shift ;;
        -h|--help) usage; exit 0 ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

log() { printf '==> %s\n' "$*"; }
die() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

for tool in dpkg dpkg-deb dpkg-shlibdeps file; do
    command -v "${tool}" >/dev/null 2>&1 || die "${tool} not found (install dpkg-dev and file)"
done

# v6.4.1-3-gabc1234-dirty -> 6.4.1+git3.gabc1234.dirty
deb_version() {
    local described
    described="$(git -C "${ROOT}" describe --tags --always --dirty 2>/dev/null || true)"
    [[ -n "${described}" ]] || { date +0.0+git%Y%m%d; return; }
    described="$(sed -E \
        -e 's/^v//' \
        -e 's/-([0-9]+)-g([0-9a-f]+)/+git\1.g\2/' \
        -e 's/-dirty$/.dirty/' \
        -e 's/-/./g' <<<"${described}")"
    [[ "${described}" =~ ^[0-9] ]] || described="0.0+git.${described}"
    echo "${described}"
}

VERSION="${DEB_VERSION:-$(deb_version)}"
ARCH="$(dpkg --print-architecture)"
DEB_FILE="${OUTPUT_DIR}/${PKG_NAME}_${VERSION}_${ARCH}.deb"
OPT="${PKGROOT}${INSTALL_PREFIX}"

log "Packaging ${PKG_NAME} ${VERSION} (${ARCH})"

mkdir -p "${OUTPUT_DIR}"
[[ -w "${OUTPUT_DIR}" ]] || die "${OUTPUT_DIR} is not writable (set OUTPUT_DIR)"

log "Collecting binary, Qt and libraries with linuxdeploy..."
APPDIR="${APPDIR}" BUILD_DIR="${BUILD_DIR}" OUTPUT_DIR="${OUTPUT_DIR}" \
    "${ROOT}/scripts/make_appimage.sh" --appdir-only "${APPIMAGE_ARGS[@]}"

[[ -x "${APPDIR}/usr/bin/cudasdr" ]] || die "AppDir has no usr/bin/cudasdr"

rm -rf "${PKGROOT}"
mkdir -p "${OPT}" \
         "${PKGROOT}/DEBIAN" \
         "${PKGROOT}/usr/bin" \
         "${PKGROOT}/usr/share/applications" \
         "${PKGROOT}/usr/share/icons/hicolor/256x256/apps"

cp -a "${APPDIR}/usr/." "${OPT}/"
rm -rf "${OPT}/share/applications" "${OPT}/share/icons"

log "Removing bundled libraries that the distro already provides..."
pruned=0
shopt -s nullglob
for lib in "${OPT}/lib"/*.so*; do
    name="$(basename "${lib}")"
    # Distro Qt (e.g. KDE neon's) must never stand in for the bundled 6.12.
    [[ "${name}" == libQt6* ]] && continue
    if dpkg -S "*/${name}" >/dev/null 2>&1; then
        rm -f "${lib}"
        pruned=$((pruned + 1))
    fi
done
shopt -u nullglob
log "  removed ${pruned}; keeping:"
for lib in "${OPT}/lib"/*.so*; do
    printf '    %s\n' "$(basename "${lib}")"
done

ln -sfn "${INSTALL_PREFIX}/bin/cudasdr" "${PKGROOT}/usr/bin/cudasdr"
install -m 0644 "${ROOT}/packaging/cudasdr.desktop" "${PKGROOT}/usr/share/applications/cudasdr.desktop"
install -m 0644 "${APPDIR}/cudasdr.png" "${PKGROOT}/usr/share/icons/hicolor/256x256/apps/cudasdr.png"

DOC_DIR="${PKGROOT}/usr/share/doc/${PKG_NAME}"
mkdir -p "${DOC_DIR}"
install -m 0644 "${ROOT}/packaging/debian-copyright" "${DOC_DIR}/copyright"
{
    printf '%s (%s) unstable; urgency=medium\n\n' "${PKG_NAME}" "${VERSION}"
    printf '  * Build of %s.\n\n' "$(git -C "${ROOT}" describe --tags --always --dirty 2>/dev/null || echo unknown)"
    printf ' -- %s  %s\n' "${MAINTAINER}" "$(date -R)"
} | gzip -9n > "${DOC_DIR}/changelog.gz"
chmod 0644 "${DOC_DIR}/changelog.gz"

log "Computing Depends with dpkg-shlibdeps..."
mapfile -d '' ELF_FILES < <(
    find "${OPT}" -type f -print0 | while IFS= read -r -d '' f; do
        if file -b "${f}" | grep -q '^ELF'; then
            printf '%s\0' "${f}"
        fi
    done
)
[[ ${#ELF_FILES[@]} -gt 0 ]] || die "No ELF files found under ${OPT}"

log "Stripping ${#ELF_FILES[@]} ELF files..."
for f in "${ELF_FILES[@]}"; do
    strip --strip-unneeded "${f}" 2>/dev/null || true
    if [[ "${f}" == *.so* ]]; then
        chmod 0644 "${f}"
    fi
done

SHLIBS_WORK="$(mktemp -d)"
trap 'rm -rf "${SHLIBS_WORK}"' EXIT
mkdir -p "${SHLIBS_WORK}/debian"
printf 'Source: %s\n\nPackage: %s\nArchitecture: any\n' "${PKG_NAME}" "${PKG_NAME}" \
    > "${SHLIBS_WORK}/debian/control"

SHLIBS_OUT="$(cd "${SHLIBS_WORK}" && dpkg-shlibdeps -O --ignore-missing-info \
    -l"${OPT}/lib" "${ELF_FILES[@]}" 2> >(grep -vE 'warning: |diversion by' >&2))"
DEPENDS="$(sed -n 's/^shlibs:Depends=//p' <<<"${SHLIBS_OUT}")"
[[ -n "${DEPENDS}" ]] || die "dpkg-shlibdeps produced no dependencies"

INSTALLED_SIZE="$(du -sk --exclude=DEBIAN "${PKGROOT}" | cut -f1)"

cat > "${PKGROOT}/DEBIAN/control" <<EOF
Package: ${PKG_NAME}
Version: ${VERSION}
Section: hamradio
Priority: optional
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Installed-Size: ${INSTALLED_SIZE}
Depends: ${DEPENDS}
Suggests: soapysdr0.8-module-all
Homepage: ${HOMEPAGE}
Description: HPSDR / SoapySDR software defined radio client
 cudaSDR is a Qt-based receiver and transceiver application for OpenHPSDR
 (Protocol 1 and 2) radios and SoapySDR devices, with OpenGL panadapter and
 waterfall displays, WDSP signal processing, FreeDV and a TCI server.
 .
 Qt ${REQUIRED_QT_VERSION:-6.12} is bundled privately under ${INSTALL_PREFIX}.
EOF

chmod -R u+rwX,go+rX,go-w "${PKGROOT}"

rm -f "${DEB_FILE}"
log "Building ${DEB_FILE}..."
dpkg-deb --root-owner-group -Zxz --build "${PKGROOT}" "${DEB_FILE}"

if command -v lintian >/dev/null 2>&1; then
    log "lintian (informational):"
    # dir-or-file-in-opt and embedded-library (Qt's FFmpeg) are expected.
    lintian --tag-display-limit 0 "${DEB_FILE}" || true
fi

log ".deb ready:"
ls -lh "${DEB_FILE}"
cat <<EOF

Install with:
  sudo apt install ${DEB_FILE}
EOF
