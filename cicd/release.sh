#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# release.sh — GitFlow release of the Avela firmware
#
# Usage:
#   cicd/release.sh <version>          e.g. cicd/release.sh 1.0.0
#   cicd/release.sh major|minor|patch  bumps the corresponding component
#
# Flow:
#   devel (X.Y.Z-dev) → release/X.Y.Z → main (tag vX.Y.Z) → devel (X.Y.(Z+1)-dev)
#
# The version is PROJECT_VER in the top-level CMakeLists.txt (X.Y.Z-dev on devel, X.Y.Z on a release) and, as X.Y.Z,
# every official skin's AVELA_SKIN_API_VERSION; this script sets them together. When the release has another number
# than devel's (minor, major), the skin changes written for devel's number move with it: the "## <version>" section
# of docs/developer/SKIN_CHANGES.md and the lower bound of supported_skin_api_versions. Needs an activated ESP-IDF
# environment: the gates run on a fresh clone of the release branch (cicd/test_unit.sh) before it is tagged and merged.
# Pushes main, devel and the tag to origin at the end.
# ---------------------------------------------------------------------------
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CMAKELISTS="$REPO_ROOT/CMakeLists.txt"
SKIN_CHANGES="$REPO_ROOT/docs/developer/SKIN_CHANGES.md"

die() { echo "ERROR: $*" >&2; exit 1; }

# devel's version without "-dev": the release it leads to
current_version() { sed -n 's/^set(PROJECT_VER "\([0-9.]*\)\(-dev\)\{0,1\}")$/\1/p' "$CMAKELISTS"; }

bump() {
    local major minor patch
    IFS='.' read -r major minor patch <<< "$(current_version)"
    case "$1" in
        major) echo "$((major + 1)).0.0" ;;
        minor) echo "$major.$((minor + 1)).0" ;;
        patch) echo "$major.$minor.$patch" ;;  # devel's X.Y.Z-dev already targets X.Y.Z
    esac
}

next_dev() {
    local major minor patch
    IFS='.' read -r major minor patch <<< "$1"
    echo "$major.$minor.$((patch + 1))"
}

# set_version <X.Y.Z> <PROJECT_VER>: the firmware's version and the official skins' API version
set_version() {
    sed -i "s/^set(PROJECT_VER \".*\")$/set(PROJECT_VER \"$2\")/" "$CMAKELISTS"
    for kconfig in "$REPO_ROOT"/components/avela_skin_*/Kconfig; do
        awk -v version="$1" '/^config AVELA_SKIN_API_VERSION$/ { found = 1 }
            found && /^ *default "/ { sub(/"[^"]*"/, "\"" version "\""); found = 0 } { print }' \
            "$kconfig" > "$kconfig.new"
        mv "$kconfig.new" "$kconfig"
    done
}

# carry_skin_changes <from> <to>: devel's skin changes become the release's
carry_skin_changes() {
    sed -i "s/^## $1$/## $2/" "$SKIN_CHANGES"
    sed -i "s/^set(supported_skin_api_versions \">=$1\([^\"]*\)\")$/set(supported_skin_api_versions \">=$2\1\")/" \
        "$CMAKELISTS"
}

[[ $# -eq 1 ]] || die "Usage: $0 <version|major|minor|patch>"
case "$1" in
    major|minor|patch) TARGET_VERSION="$(bump "$1")" ;;
    [0-9]*.[0-9]*.[0-9]*) TARGET_VERSION="$1" ;;
    *) die "Argument must be a version (X.Y.Z) or one of: major minor patch" ;;
esac
[[ "$TARGET_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "Version must be X.Y.Z"
DEVEL_VERSION="$(current_version)"
DEV_VERSION="$(next_dev "$TARGET_VERSION")"
RELEASE_BRANCH="release/$TARGET_VERSION"
TAG="v$TARGET_VERSION"

cd "$REPO_ROOT"
[[ -n "$DEVEL_VERSION" ]] || die "Could not read PROJECT_VER from CMakeLists.txt"
[[ -z "$(git status --porcelain)" ]] || die "Working tree is dirty. Commit or stash changes before releasing."
[[ "$(git rev-parse --abbrev-ref HEAD)" == "devel" ]] || die "Must be on 'devel' to start a release."
command -v idf.py > /dev/null || die "No idf.py: activate the ESP-IDF environment first."
git config user.email > /dev/null || die "No git identity (git config user.name / user.email): commits would fail."
git fetch --quiet origin
[[ "$(git rev-parse devel)" == "$(git rev-parse origin/devel)" ]] ||
    die "Local 'devel' is not origin/devel. Pull or push first."
git tag | grep -qx "$TAG" && die "Tag $TAG already exists."
printf '%s\n' "$DEVEL_VERSION" "$TARGET_VERSION" | sort -V -C ||
    die "$TARGET_VERSION is older than devel's $DEVEL_VERSION."

echo "Releasing $TARGET_VERSION  (devel will become $DEV_VERSION-dev)"
git switch -c "$RELEASE_BRANCH"
set_version "$TARGET_VERSION" "$TARGET_VERSION"
[[ "$DEVEL_VERSION" == "$TARGET_VERSION" ]] || carry_skin_changes "$DEVEL_VERSION" "$TARGET_VERSION"
git add -A
git commit -m "chore: set version $TARGET_VERSION"
# the gates on a fresh clone of the release commit, never on this working copy
if ! "$REPO_ROOT/cicd/test_unit.sh" --source "$REPO_ROOT" --tag "$RELEASE_BRANCH"; then
    git switch --quiet devel
    git branch -D "$RELEASE_BRANCH"
    die "The gates failed on $TARGET_VERSION; nothing was released."
fi
git tag -a "$TAG" -m "$TAG"

git switch main
git pull --quiet origin main
git merge --no-ff "$RELEASE_BRANCH" -m "release: $TARGET_VERSION"

git switch devel
git merge --no-ff "$RELEASE_BRANCH" -m "release: merge $TARGET_VERSION back to devel"
set_version "$DEV_VERSION" "$DEV_VERSION-dev"
git add -A
git commit -m "chore: bump devel to $DEV_VERSION-dev"
git branch -d "$RELEASE_BRANCH"

git push origin main devel "$TAG"
echo "Done. Released $TAG. devel is now $DEV_VERSION-dev."
