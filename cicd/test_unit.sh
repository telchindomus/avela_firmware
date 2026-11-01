#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# test_unit.sh — the commit gates of the Avela firmware: `idf.py build` (strict warnings) and tools/check_code.sh
# (clang-format, clang-tidy, the unit tests, the skin API and docs checks)
#
# Usage:
#   cicd/test_unit.sh                              test a fresh clone of origin's devel
#   cicd/test_unit.sh --tag v1.0.0                 test a fresh clone of that tag (or branch)
#   cicd/test_unit.sh --source <path> --tag <ref>  clone from another repository (release.sh: this one's release branch)
#
# Always a fresh clone in $CLONES_DIR/test-<time>, never the working copy. Needs an activated ESP-IDF environment and
# the host tools in docs/developer/BUILD.md (Checks before a commit). A clone is a full one (the skin API check
# compares with the last release tag) and builds from sdkconfig.defaults; it is removed after a pass and kept after a
# failure, for a look.
# ---------------------------------------------------------------------------
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
die() { echo "ERROR: $*" >&2; exit 1; }

[[ -f "$REPO_ROOT/cicd/.env" ]] && { set -a; source "$REPO_ROOT/cicd/.env"; set +a; }
: "${BUILD_DIR:=$REPO_ROOT/cicd/build}"
: "${CLONES_DIR:=$BUILD_DIR/clones}"
: "${REPO_URL:=$(git -C "$REPO_ROOT" remote get-url origin 2> /dev/null || true)}"

SOURCE="$REPO_URL"
REF="devel"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --tag) [[ $# -ge 2 ]] || die "--tag requires a value"; REF="$2"; shift 2 ;;
        --source) [[ $# -ge 2 ]] || die "--source requires a value"; SOURCE="$2"; shift 2 ;;
        *) die "Unknown argument: $1" ;;
    esac
done
[[ -n "$SOURCE" ]] || die "No REPO_URL (cicd/.env) and no origin remote"
command -v idf.py > /dev/null || die "No idf.py: activate the ESP-IDF environment first."

mkdir -p "$CLONES_DIR"
CLONE_DIR="$CLONES_DIR/test-$(date +%Y%m%d-%H%M%S)"
PASSED=false
trap 'if $PASSED; then rm -rf "$CLONE_DIR"; else echo "The failed clone is kept: $CLONE_DIR" >&2; fi' EXIT
echo "Cloning $SOURCE ($REF) → $CLONE_DIR"
git clone --branch "$REF" --quiet "$SOURCE" "$CLONE_DIR"
cd "$CLONE_DIR"
idf.py build
tools/check_code.sh
PASSED=true
echo "Gates passed on $REF"
