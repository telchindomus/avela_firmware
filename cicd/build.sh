#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# build.sh — the Avela firmware images from origin, one set per official skin
#
# Usage:
#   cicd/build.sh                    build from BUILD_BRANCH (cicd/.env, default main)
#   cicd/build.sh --tag v1.0.0       build from a tag
#   cicd/build.sh --version 1.0.0    build from tag v1.0.0
#
# Always from a fresh shallow clone in $CLONES_DIR/build-<time>, never the working copy, with sdkconfig.defaults and
# the skin chosen on top of it; the clone is removed afterwards.
# Needs an activated ESP-IDF environment. Output: $ARTIFACTS_DIR/<version>/<skin>/ with
#   avela_firmware.bin            the app: network update (setup page, avela-tuner update) or USB at 0x10000
#   ota_data_initial.bin          otadata, written with the app over USB (keeps the settings)
#   bootloader/, partition_table/ for a first install by parts
#   flash_args                    the offsets and flash settings (esptool.py write_flash @flash_args, from that folder)
#   avela_firmware_full.bin       the merged image at 0x0, for a first install (erases the settings)
# and checksums.sha256 and version.txt (made here, never committed) in <version>/, and a `latest` symlink.
# ---------------------------------------------------------------------------
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
die() { echo "ERROR: $*" >&2; exit 1; }

[[ -f "$REPO_ROOT/cicd/.env" ]] && { set -a; source "$REPO_ROOT/cicd/.env"; set +a; }
: "${BUILD_DIR:=$REPO_ROOT/cicd/build}"
: "${ARTIFACTS_DIR:=$BUILD_DIR/artifacts}"
: "${CLONES_DIR:=$BUILD_DIR/clones}"
: "${REPO_URL:=$(git -C "$REPO_ROOT" remote get-url origin 2> /dev/null || true)}"
: "${BUILD_BRANCH:=main}"
[[ -n "$REPO_URL" ]] || die "No REPO_URL (cicd/.env) and no origin remote"
command -v idf.py > /dev/null || die "No idf.py: activate the ESP-IDF environment first."

REF="$BUILD_BRANCH"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --tag) [[ $# -ge 2 ]] || die "--tag requires a value"; REF="$2"; shift 2 ;;
        --version) [[ $# -ge 2 ]] || die "--version requires a value"; REF="v$2"; shift 2 ;;
        *) die "Unknown argument: $1. Usage: $0 [--tag vX.Y.Z | --version X.Y.Z]" ;;
    esac
done

mkdir -p "$CLONES_DIR"
CLONE_DIR="$CLONES_DIR/build-$(date +%Y%m%d-%H%M%S)"
OUT_DIR=""
BUILD_OK=false
trap 'rm -rf "$CLONE_DIR"; $BUILD_OK || [[ -z "$OUT_DIR" ]] || rm -rf "$OUT_DIR"' EXIT
echo "Cloning $REPO_URL ($REF) → $CLONE_DIR"
git clone --branch "$REF" --depth 1 --quiet "$REPO_URL" "$CLONE_DIR"

VERSION=$(sed -n 's/^set(PROJECT_VER "\(.*\)")$/\1/p' "$CLONE_DIR/CMakeLists.txt")
[[ -n "$VERSION" ]] || die "Could not read PROJECT_VER from CMakeLists.txt"
OUT_DIR="$ARTIFACTS_DIR/$VERSION"
if [[ -d "$OUT_DIR" ]]; then
    if [[ -t 0 ]]; then
        read -r -p "Build $VERSION already exists at $OUT_DIR. Delete and rebuild? [y/N] " confirm
        [[ "$confirm" =~ ^[Yy]$ ]] || { OUT_DIR=""; die "Aborted."; }
    fi
    rm -rf "$OUT_DIR"
fi
mkdir -p "$OUT_DIR"

cd "$CLONE_DIR"
for kconfig in components/avela_skin_*/Kconfig; do
    skin="$(basename "$(dirname "$kconfig")")"
    # the skin's entry in the Skin choice: its first `config` line
    symbol=$(awk '/^ *config AVELA_SKIN_/ { print $2; exit }' "$kconfig")
    [[ -n "$symbol" ]] || die "$kconfig: no Skin choice entry"
    build="build_$skin"
    mkdir -p "$build"
    printf 'CONFIG_%s=y\n' "$symbol" > "$build/sdkconfig.skin"
    idf.py -B "$build" -D SDKCONFIG="$build/sdkconfig" \
        -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;$build/sdkconfig.skin" build
    (cd "$build" && esptool.py --chip esp32 merge_bin -o avela_firmware_full.bin @flash_args)

    mkdir -p "$OUT_DIR/$skin"
    # flash_args and the files it names, at their paths, so `esptool.py write_flash @flash_args` works from there
    mapfile -t images < <(tail -n +2 "$build/flash_args" | cut -d' ' -f2)
    (cd "$build" && cp --parents flash_args avela_firmware_full.bin "${images[@]}" "$OUT_DIR/$skin/")
done

(cd "$OUT_DIR" && find . -type f ! -name checksums.sha256 -print0 | sort -z | xargs -0 sha256sum > checksums.sha256)
printf '%s\n' "$VERSION" > "$OUT_DIR/version.txt"
ln -sfn "$VERSION" "$ARTIFACTS_DIR/latest"
BUILD_OK=true
echo "Built $VERSION → $OUT_DIR"
(cd "$OUT_DIR" && find . -type f | sort)
