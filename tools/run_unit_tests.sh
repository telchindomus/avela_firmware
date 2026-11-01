#!/usr/bin/env bash
# Builds and runs every unit test project (tests/*/: Catch2 on IDF_TARGET linux, ESP-IDF's way) and fails if one
# fails. Needs an activated ESP-IDF, a native g++ and Ruby (ESP-IDF's FreeRTOS mock is made by CMock).
# Each project builds in its own build/ (gitignored); its compile commands are what tools/check_code.sh tidies.
set -euo pipefail
cd "$(dirname "$0")/.."

mapfile -t projects < <(find tests -mindepth 2 -maxdepth 2 -name CMakeLists.txt -printf '%h\n' | sort)
failed=()
for project in "${projects[@]}"; do
    name=$(sed -n 's/^project(\([A-Za-z0-9_]*\))$/\1/p' "$project/CMakeLists.txt")
    echo "run_unit_tests: $project"
    [[ -f "$project/sdkconfig" ]] || idf.py -C "$project" --preview set-target linux > /dev/null
    idf.py -C "$project" build > /dev/null
    "$project/build/$name.elf" || failed+=("$project")
done
if (( ${#failed[@]} )); then
    printf 'run_unit_tests: FAILED %s\n' "${failed[@]}" >&2
    exit 1
fi
echo "run_unit_tests: ${#projects[@]} projects passed"
