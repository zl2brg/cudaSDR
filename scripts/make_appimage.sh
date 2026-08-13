#!/usr/bin/env bash
# Build a portable cudaSDR AppImage with linuxdeploy + the Qt plugin.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
APPDIR="${APPDIR:-${ROOT}/AppDir}"
TOOLS_DIR="${TOOLS_DIR:-${ROOT}/.tools}"
OUTPUT_DIR="${OUTPUT_DIR:-${ROOT}}"
ARCH="$(uname -m)"
REQUIRED_QT_VERSION="${REQUIRED_QT_VERSION:-6.11.0}"

DO_BUILD=0
SKIP_DOWNLOAD=0
CLEAN_APPDIR=1

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --build           Run ./build.sh before packaging
  --no-clean        Keep an existing AppDir (default: wipe and recreate)
  --skip-download   Do not download linuxdeploy tools (must already exist)
  -h, --help        Show this help

Environment:
  BUILD_DIR         CMake build directory (default: build)
  BUILD_TYPE        Passed to build.sh when --build is used (default: Release)
  APPDIR            AppDir path (default: ./AppDir)
  TOOLS_DIR         Where linuxdeploy AppImages are cached (default: ./.tools)
  OUTPUT_DIR        Where the .AppImage is written (default: project root)
  QMAKE             Override qmake path for the Qt plugin
  LINUXDEPLOY       Override path to linuxdeploy-*.AppImage
  Qt6_DIR           Used by ./build.sh / Qt discovery when --build is set
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build) DO_BUILD=1; shift ;;
        --no-clean) CLEAN_APPDIR=0; shift ;;
        --skip-download) SKIP_DOWNLOAD=1; shift ;;
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

version_satisfies_requirement() {
    [[ -n "$1" ]] || return 1
    [[ "$(printf '%s\n%s\n' "$1" "${REQUIRED_QT_VERSION}" | sort -V | head -n 1)" == "${REQUIRED_QT_VERSION}" ]]
}

find_qt_prefix() {
    local prefix version candidate
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

    for prefix in \
        "${HOME}/Qt/${REQUIRED_QT_VERSION}/gcc_64" \
        "${HOME}/Qt/${REQUIRED_QT_VERSION}/linux_gcc_64" \
        "/opt/Qt/${REQUIRED_QT_VERSION}/gcc_64" \
        "/usr/local/Qt-${REQUIRED_QT_VERSION}" \
        "/usr/lib/qt6"
    do
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

find_binary() {
    local candidate
    for candidate in \
        "${BUILD_DIR}/cudasdr" \
        "${BUILD_DIR}/Release/cudasdr" \
        "${BUILD_DIR}/Debug/cudasdr" \
        "${ROOT}/qtbuild/cudasdr"
    do
        if [[ -x "${candidate}" ]]; then
            echo "${candidate}"
            return 0
        fi
    done
    return 1
}

download_tool() {
    local url="$1"
    local dest="$2"
    if [[ -x "${dest}" ]]; then
        return 0
    fi
    if [[ "${SKIP_DOWNLOAD}" -eq 1 ]]; then
        die "Missing tool ${dest} and --skip-download was set"
    fi
    mkdir -p "$(dirname "${dest}")"
    log "Downloading $(basename "${dest}")..."
    if command -v wget >/dev/null 2>&1; then
        wget -q --show-progress -O "${dest}" "${url}"
    elif command -v curl >/dev/null 2>&1; then
        curl -fL --progress-bar -o "${dest}" "${url}"
    else
        die "Need wget or curl to download ${url}"
    fi
    chmod +x "${dest}"
}

if [[ "${DO_BUILD}" -eq 1 ]]; then
    log "Building cudaSDR (${BUILD_TYPE})..."
    BUILD_DIR="${BUILD_DIR}" BUILD_TYPE="${BUILD_TYPE}" "${ROOT}/build.sh"
fi

BINARY="$(find_binary)" || die "cudasdr binary not found under ${BUILD_DIR}/. Build first or pass --build."
log "Using binary: ${BINARY}"

QT_PREFIX=""
if QT_PREFIX=$(find_qt_prefix); then
    log "Using Qt ${REQUIRED_QT_VERSION}+ at: ${QT_PREFIX}"
    export PATH="${QT_PREFIX}/bin:${PATH}"
    export LD_LIBRARY_PATH="${QT_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    if [[ -z "${QMAKE:-}" ]]; then
        if [[ -x "${QT_PREFIX}/bin/qmake" ]]; then
            export QMAKE="${QT_PREFIX}/bin/qmake"
        elif [[ -x "${QT_PREFIX}/bin/qmake6" ]]; then
            export QMAKE="${QT_PREFIX}/bin/qmake6"
        fi
    fi
else
    log "WARNING: Qt ${REQUIRED_QT_VERSION}+ not found via usual paths; relying on QMAKE/PATH"
fi

if [[ -z "${QMAKE:-}" ]]; then
    if command -v qmake >/dev/null 2>&1; then
        export QMAKE="$(command -v qmake)"
    elif command -v qmake6 >/dev/null 2>&1; then
        export QMAKE="$(command -v qmake6)"
    else
        die "qmake not found. Set QMAKE to your Qt ${REQUIRED_QT_VERSION}+ qmake."
    fi
fi
log "QMAKE=${QMAKE}"

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage"
QT_PLUGIN_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${ARCH}.AppImage"
LINUXDEPLOY_DEST="${TOOLS_DIR}/linuxdeploy-${ARCH}.AppImage"
QT_PLUGIN_DEST="${TOOLS_DIR}/linuxdeploy-plugin-qt-${ARCH}.AppImage"

# Seed .tools from a user-provided binary or common download locations.
if [[ -n "${LINUXDEPLOY:-}" && -x "${LINUXDEPLOY}" ]]; then
    mkdir -p "${TOOLS_DIR}"
    cp -f "${LINUXDEPLOY}" "${LINUXDEPLOY_DEST}"
    chmod +x "${LINUXDEPLOY_DEST}"
elif [[ ! -x "${LINUXDEPLOY_DEST}" ]]; then
    for candidate in \
        "${HOME}/Downloads/linuxdeploy-${ARCH}.AppImage" \
        "${HOME}/bin/linuxdeploy-${ARCH}.AppImage"
    do
        if [[ -x "${candidate}" ]]; then
            mkdir -p "${TOOLS_DIR}"
            cp -f "${candidate}" "${LINUXDEPLOY_DEST}"
            chmod +x "${LINUXDEPLOY_DEST}"
            break
        fi
    done
fi

download_tool "${LINUXDEPLOY_URL}" "${LINUXDEPLOY_DEST}"
download_tool "${QT_PLUGIN_URL}" "${QT_PLUGIN_DEST}"
LINUXDEPLOY_BIN="${LINUXDEPLOY_DEST}"

# linuxdeploy discovers the Qt plugin beside itself or on PATH
export PATH="${TOOLS_DIR}:${PATH}"

# Prefer a square icon for AppImage / desktop themes; fall back to logos.
ICON_SRC=""
for candidate in \
    "${ROOT}/res/img/hpsdr4.png" \
    "${ROOT}/res/img/cudaSDRLogo.png" \
    "${ROOT}/res/img/cusdrLogo.png"
do
    if [[ -f "${candidate}" ]]; then
        ICON_SRC="${candidate}"
        break
    fi
done
[[ -n "${ICON_SRC}" ]] || die "No icon found under res/img/"

# linuxdeploy only accepts specific square sizes (not e.g. 400x400).
prepare_appimage_icon() {
    local src="$1"
    local dest="$2"
    local size="${3:-256}"
    log "Preparing ${size}x${size} icon from ${src}"
    python3 - "$src" "$dest" "$size" <<'PY'
import sys
from PIL import Image

src, dest, size_s = sys.argv[1], sys.argv[2], sys.argv[3]
size = int(size_s)
im = Image.open(src).convert("RGBA")
w, h = im.size
side = max(w, h)
canvas = Image.new("RGBA", (side, side), (0, 0, 0, 0))
canvas.paste(im, ((side - w) // 2, (side - h) // 2), im)
canvas = canvas.resize((size, size), Image.Resampling.LANCZOS)
canvas.save(dest, format="PNG")
print(dest)
PY
}

ICON_PREP="${TOOLS_DIR}/cudasdr-icon-${ICON_SIZE:-256}.png"
mkdir -p "${TOOLS_DIR}"
prepare_appimage_icon "${ICON_SRC}" "${ICON_PREP}" "${ICON_SIZE:-256}"
ICON_SRC="${ICON_PREP}"

DESKTOP_SRC="${ROOT}/packaging/cudasdr.desktop"
[[ -f "${DESKTOP_SRC}" ]] || die "Missing ${DESKTOP_SRC}"

# Qt online-installer imageformats/libqtiff.so links against libtiff.so.5, which
# many current distros no longer ship (only libtiff.so.6). linuxdeploy-plugin-qt
# fails hard on that. Stage a plugin tree without broken optional plugins and
# make qmake -query QT_INSTALL_PLUGINS point at it.
stage_qt_plugins() {
    local qt_plugins_src="$1"
    local stage_root="${TOOLS_DIR}/qt-plugins-stage"
    local stage_plugins="${stage_root}/plugins"
    local plugin removed=0

    [[ -d "${qt_plugins_src}" ]] || die "Qt plugins directory not found: ${qt_plugins_src}"

    log "Staging Qt plugins (drop optional plugins with missing deps)..."
    rm -rf "${stage_root}"
    mkdir -p "${stage_plugins}"
    cp -a "${qt_plugins_src}/." "${stage_plugins}/"

    # Always drop TIFF — not needed by cudaSDR and commonly broken vs host libtiff.
    rm -f "${stage_plugins}/imageformats/libqtiff.so" \
          "${stage_plugins}/imageformats/libqtiff.so.debug" \
          "${stage_plugins}/imageformats/qtiff.debug"

    # Drop any other plugin whose immediate shared deps are unresolved on this host.
    while IFS= read -r -d '' plugin; do
        if ldd "${plugin}" 2>/dev/null | grep -q 'not found'; then
            log "  omitting $(basename "$(dirname "${plugin}")")/$(basename "${plugin}") (unresolved deps)"
            rm -f "${plugin}"
            removed=$((removed + 1))
        fi
    done < <(find "${stage_plugins}" -type f -name '*.so' -print0)

    log "Staged plugins at ${stage_plugins} (removed ${removed} broken plugin(s))"

    # linuxdeploy-plugin-qt runs `qmake -query` (full dump) and parses KEY:value
    # lines. It also resolves $QMAKE via which(), so expose a basename on PATH.
    local wrapper="${TOOLS_DIR}/qmake-appimage"
    cat > "${wrapper}" <<EOF
#!/usr/bin/env bash
# Rewrite QT_INSTALL_PLUGINS so linuxdeploy-plugin-qt uses the staged tree.
set -euo pipefail
REAL_QMAKE=$(printf '%q' "${QMAKE}")
STAGE_PLUGINS=$(printf '%q' "${stage_plugins}")

rewrite_query() {
    local line key
    while IFS= read -r line || [[ -n "\${line}" ]]; do
        key="\${line%%:*}"
        if [[ "\${key}" == "QT_INSTALL_PLUGINS" ]]; then
            echo "QT_INSTALL_PLUGINS:\${STAGE_PLUGINS}"
        else
            echo "\${line}"
        fi
    done
}

if [[ "\${1:-}" == "-query" ]]; then
    if [[ "\${#}" -eq 1 ]]; then
        "\${REAL_QMAKE}" -query | rewrite_query
        exit 0
    fi
    if [[ "\${2:-}" == "QT_INSTALL_PLUGINS" ]]; then
        echo "\${STAGE_PLUGINS}"
        exit 0
    fi
fi
exec "\${REAL_QMAKE}" "\$@"
EOF
    chmod +x "${wrapper}"

    # Prefer basename + PATH: plugin does which($QMAKE).
    ln -sfn "${wrapper}" "${TOOLS_DIR}/qmake"
    ln -sfn "${wrapper}" "${TOOLS_DIR}/qmake6"
    export PATH="${TOOLS_DIR}:${PATH}"
    export QMAKE="qmake-appimage"
    log "QMAKE wrapper: ${wrapper} (QMAKE=${QMAKE})"
    log "Staged QT_INSTALL_PLUGINS=$(${wrapper} -query QT_INSTALL_PLUGINS)"
}

QT_PLUGINS_SRC=""
if [[ -n "${QT_PREFIX}" && -d "${QT_PREFIX}/plugins" ]]; then
    QT_PLUGINS_SRC="${QT_PREFIX}/plugins"
else
    QT_PLUGINS_SRC="$("${QMAKE}" -query QT_INSTALL_PLUGINS 2>/dev/null || true)"
fi
[[ -n "${QT_PLUGINS_SRC}" && -d "${QT_PLUGINS_SRC}" ]] || die "Could not locate Qt plugins to stage"
stage_qt_plugins "${QT_PLUGINS_SRC}"

if [[ "${CLEAN_APPDIR}" -eq 1 ]]; then
    log "Preparing AppDir at ${APPDIR}"
    rm -rf "${APPDIR}"
fi
mkdir -p "${APPDIR}/usr/bin" "${APPDIR}/usr/share/applications" "${APPDIR}/usr/share/icons/hicolor/256x256/apps"

cp -f "${BINARY}" "${APPDIR}/usr/bin/cudasdr"
chmod +x "${APPDIR}/usr/bin/cudasdr"
cp -f "${DESKTOP_SRC}" "${APPDIR}/usr/share/applications/cudasdr.desktop"
cp -f "${DESKTOP_SRC}" "${APPDIR}/cudasdr.desktop"
cp -f "${ICON_SRC}" "${APPDIR}/cudasdr.png"
cp -f "${ICON_SRC}" "${APPDIR}/usr/share/icons/hicolor/256x256/apps/cudasdr.png"

# Flag missing shared libs early (still package; linuxdeploy may pull most in).
if command -v ldd >/dev/null 2>&1; then
    missing="$(ldd "${APPDIR}/usr/bin/cudasdr" | awk '/not found/ {print $1}' || true)"
    if [[ -n "${missing}" ]]; then
        log "WARNING: binary reports missing libraries before bundling:"
        printf '    %s\n' ${missing}
    fi
fi

mkdir -p "${OUTPUT_DIR}"
export LDA_OUTPUT_VERSION="${LDA_OUTPUT_VERSION:-$(git -C "${ROOT}" describe --tags --always --dirty 2>/dev/null || date +%Y%m%d)}"
export LINUXDEPLOY_OUTPUT_VERSION="${LINUXDEPLOY_OUTPUT_VERSION:-${LDA_OUTPUT_VERSION}}"

log "Running linuxdeploy (this can take a while)..."
(
    cd "${OUTPUT_DIR}"
    # APPIMAGE_EXTRACT_AND_RUN helps in sandboxed / FUSE-less environments
    export APPIMAGE_EXTRACT_AND_RUN="${APPIMAGE_EXTRACT_AND_RUN:-1}"
    export QMAKE
    "${LINUXDEPLOY_BIN}" \
        --appdir "${APPDIR}" \
        --executable "${APPDIR}/usr/bin/cudasdr" \
        --desktop-file "${APPDIR}/cudasdr.desktop" \
        --icon-file "${APPDIR}/cudasdr.png" \
        --plugin qt \
        --output appimage
)

shopt -s nullglob
artifacts=("${OUTPUT_DIR}"/*cudaSDR*.AppImage "${OUTPUT_DIR}"/*cudasdr*.AppImage)
if [[ ${#artifacts[@]} -eq 0 ]]; then
    # linuxdeploy names from desktop Name= field
    artifacts=("${OUTPUT_DIR}"/*.AppImage)
fi
shopt -u nullglob

if [[ ${#artifacts[@]} -eq 0 ]]; then
    die "linuxdeploy finished but no .AppImage was found in ${OUTPUT_DIR}"
fi

log "AppImage ready:"
for f in "${artifacts[@]}"; do
    ls -lh "${f}"
done

cat <<EOF

Run with:
  chmod +x ${artifacts[-1]}
  ${artifacts[-1]}

Tip: select the intended HPSDR device in Network settings before Start when multiple devices are present.
EOF
