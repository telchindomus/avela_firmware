#!/usr/bin/env bash
# Keeps the skin API honest (docs/developer/SKIN_ARCHITECTURE.md: Versions and skin compatibility); run by
# tools/check_code.sh.
# 1. Every component that defines a skin concept (include/**/*_skin_concept.hpp), and avela_ui, has a SKIN.md.
# 2. If skin-facing files changed since the last release tag (v*): the project version is past that release,
#    docs/developer/SKIN_CHANGES.md has a section for it, and unless that section says "Nothing to change", the lower
#    bound of supported_skin_api_versions is that version (both in the top-level CMakeLists.txt).
# 3. Every official skin (components/avela_skin_*) states the project version as its AVELA_SKIN_API_VERSION: it comes
#    with this firmware, and a skin built from a copy of it starts from the right version.
set -euo pipefail
cd "$(dirname "$0")/.."

failed=0
fail() {
    echo "check_skin_api: $*" >&2
    failed=1
}

# the skin concepts and the shared pieces: what skins implement or build on
mapfile -t skin_facing < <(find components -path 'components/avela_skin_*' -prune -o \
    \( -name '*_skin_concept.hpp' -o -path 'components/avela_ui/include/*' \) -type f -print | sort)

for component in $(printf '%s\n' "${skin_facing[@]}" | cut -d/ -f2 | sort -u); do
    [[ -f "components/$component/docs/developer/SKIN.md" ]] || fail "components/$component has skin-facing headers but no docs/developer/SKIN.md"
done

# the project version: PROJECT_VER in the top-level CMakeLists.txt, X.Y.Z or, on devel between releases, X.Y.Z-dev
project_ver=$(sed -n 's/^set(PROJECT_VER "\(.*\)")$/\1/p' CMakeLists.txt)
[[ "$project_ver" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-dev)?$ ]] ||
    fail "CMakeLists.txt: PROJECT_VER \"$project_ver\" must be X.Y.Z or X.Y.Z-dev"
version="${project_ver%-dev}"
for kconfig in components/avela_skin_*/Kconfig; do
    skin_api_version=$(awk '/^config AVELA_SKIN_API_VERSION$/ { found = 1; next }
        found && /^ *default / { gsub(/"/, "", $2); print $2; exit }' "$kconfig")
    [[ "$skin_api_version" == "$version" ]] ||
        fail "$kconfig: AVELA_SKIN_API_VERSION \"$skin_api_version\" must be the project version, $version"
done

release=$(git describe --tags --abbrev=0 --match 'v*' 2> /dev/null || true)
if [[ -n "$release" ]] && ! git diff --quiet "$release" -- "${skin_facing[@]}"; then
    lower_bound=$(sed -n 's/^set(supported_skin_api_versions ">=\([0-9.]*\).*/\1/p' CMakeLists.txt)
    section=$(awk -v heading="## $version" '$0 == heading { found = 1; next } /^## / { found = 0 } found' docs/developer/SKIN_CHANGES.md)
    if [[ "v$version" == "$release" ]]; then
        fail "skin-facing files changed since $release: set PROJECT_VER in CMakeLists.txt to the next version"
    elif [[ -z "$section" ]]; then
        fail "skin-facing files changed since $release: docs/developer/SKIN_CHANGES.md needs a '## $version' section"
    elif ! grep -q "Nothing to change" <<< "$section" && [[ "$lower_bound" != "$version" ]]; then
        fail "docs/developer/SKIN_CHANGES.md says skins must change for $version: supported_skin_api_versions must start at >=$version"
    fi
fi

exit "$failed"
