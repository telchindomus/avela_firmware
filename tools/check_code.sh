#!/usr/bin/env bash
# Code-quality gate for main/, components/avela_* and tests/: clang-format (check, or rewrite with --fix), then clang-tidy (.clang-tidy).
# Every skin is checked, not only the chosen one (each other skin in a clang build of its own, build_clang_<skin>).
# Needs an activated ESP-IDF and its clang toolchain:  python "$IDF_PATH/tools/idf_tools.py" install esp-clang
# The strict compiler warnings run in every normal `idf.py build` (cmake/strict_warnings.cmake). Then the unit tests
# (tools/run_unit_tests.sh), whose sources are tidied with their own builds' compile commands, and last
# tools/check_skin_api.sh (SKIN.md per component, skin API version kept with skin-facing changes) and
# tools/check_docs.sh (every file a doc names exists).
set -euo pipefail
cd "$(dirname "$0")/.."

clang_bin=$(ls -d "${IDF_TOOLS_PATH:-$HOME/.espressif}"/tools/esp-clang/*/esp-clang/bin | tail -n 1)
# our sources: not the test projects' builds and downloaded components (Catch2, the codec)
mapfile -t sources < <(find main components tests \( -name build -o -name 'build_clang*' -o -name managed_components \) \
    -prune -o \( -name '*.cpp' -o -name '*.hpp' \) -print | sort)
mapfile -t translation_units < <(printf '%s\n' "${sources[@]}" | grep '\.cpp$' | grep -v '^tests/')
mapfile -t unit_test_units < <(printf '%s\n' "${sources[@]}" | grep '^tests/.*\.cpp$')

if [[ "${1:-}" == "--fix" ]]; then
    "$clang_bin/clang-format" -i "${sources[@]}"
else
    "$clang_bin/clang-format" --dry-run --Werror "${sources[@]}"
fi

# clang-tidy reads the compile commands of a clang-configured build (configure only, nothing is compiled there).
# Only the chosen skin is in a build, so every other skin gets a build of its own: build_clang_<skin>, configured
# with that skin chosen (its Kconfig entry instead of the chosen one in a copy of sdkconfig) and every optional part
# on. When sdkconfig switches an optional part off, the sources it leaves out of build_clang (its component's and
# the chosen skin's pages for it) go to build_clang_optional, configured with every optional part on. A build is
# configured again when its sdkconfig, a CMakeLists.txt or a Kconfig is newer than its compile commands.
configure() {  # configure <build directory> [<sdkconfig>]
    local directory=$1 database="$1/compile_commands.json"
    local sdkconfig_argument=()
    [[ -z "${2:-}" ]] || sdkconfig_argument=(-D SDKCONFIG="$2")
    if [[ ! -f "$database" || "${2:-sdkconfig}" -nt "$database" ||
          -n "$(find CMakeLists.txt main components -name CMakeLists.txt -newer "$database")" ||
          -n "$(find components -name Kconfig -newer "$database")" ]]; then
        PATH="$clang_bin:$PATH" idf.py -B "$directory" -D IDF_TOOLCHAIN=clang \
            "${sdkconfig_argument[@]}" reconfigure > /dev/null
    fi
}

# check_config <file>: stdin into the file, only when it differs (a new date would configure the build again)
check_config() {
    cat > "$1.new"
    if cmp -s "$1.new" "$1"; then rm "$1.new"; else mv "$1.new" "$1"; fi
}

# in_build <build directory> <translation unit>: is the unit in that build's compile commands
in_build() {
    grep -qF "$PWD/$2" "$1/compile_commands.json"
}

# optional parts: config symbols whose sources (in their own component and in the skins) compile only with them on
optional_symbols=(AVELA_WEATHER)
# with_optional_on: sdkconfig on stdin, every optional part switched on
with_optional_on() {
    grep -v -E "^(# )?CONFIG_($(IFS='|'; echo "${optional_symbols[*]}"))[= ]"
    printf 'CONFIG_%s=y\n' "${optional_symbols[@]}"
}

# tidy <build directory> <translation units...> (CLANG_TIDY: which clang-tidy, default esp-clang's): every unit must be in that build (stderr is silenced below, so else
# clang-tidy would only guess flags); "N warnings generated" on stderr counts the libraries' suppressed warnings,
# findings in our code go to stdout
tidy() {
    local directory=$1
    shift
    for unit in "$@"; do
        if ! in_build "$directory" "$unit"; then
            echo "check_code: $unit is not in any CMakeLists.txt SRCS" >&2
            exit 1
        fi
    done
    printf '%s\n' "$@" | xargs -P "$(nproc)" -n 1 "${CLANG_TIDY:-$clang_bin/clang-tidy}" -p "$directory" --quiet \
        2> /dev/null
}

configure build_clang
chosen_skin=$(sed -n 's/^CONFIG_AVELA_SKIN_COMPONENT="\(.*\)"$/\1/p' sdkconfig)
mapfile -t other_skins < <(find components -maxdepth 1 -name 'avela_skin_*' -printf '%f\n' | grep -vx "$chosen_skin" |
    sort)
switched_off=false
for symbol in "${optional_symbols[@]}"; do
    grep -qx "CONFIG_$symbol=y" sdkconfig || switched_off=true
done
units_here=()
units_optional=()
for unit in "${translation_units[@]}"; do
    skin=$(sed -n 's|^components/\(avela_skin_[^/]*\)/.*|\1|p' <<< "$unit")
    [[ -z "$skin" || "$skin" == "$chosen_skin" ]] || continue
    if ! $switched_off || in_build build_clang "$unit"; then
        units_here+=("$unit")
    else
        units_optional+=("$unit")
    fi
done
tidy build_clang "${units_here[@]}"

if (( ${#units_optional[@]} > 0 )); then
    directory=build_clang_optional
    mkdir -p "$directory"
    with_optional_on < sdkconfig | check_config "$directory/sdkconfig.check"
    configure "$directory" "$PWD/$directory/sdkconfig.check"
    tidy "$directory" "${units_optional[@]}"
fi

for skin in "${other_skins[@]}"; do
    # the skin's own entry in the Skin choice: its first config symbol that isn't one of the two values it sets
    symbol=$(grep -oE '^\s*config AVELA_SKIN_[A-Z0-9_]+' "components/$skin/Kconfig" | awk '{print $2}' |
        grep -vxE 'AVELA_SKIN_(COMPONENT|API_VERSION)' | head -n 1)
    directory="build_clang_$skin"
    mkdir -p "$directory"
    { grep -v -E '^(# )?CONFIG_AVELA_SKIN_' sdkconfig | with_optional_on; echo "CONFIG_$symbol=y"; } |
        check_config "$directory/sdkconfig.check"
    configure "$directory" "$PWD/$directory/sdkconfig.check"
    mapfile -t skin_units < <(printf '%s\n' "${translation_units[@]}" | grep "^components/$skin/")
    tidy "$directory" "${skin_units[@]}"
done
# The unit tests build with the native GCC for this machine, so the native clang-tidy (esp-clang is a cross compiler
# and doesn't know the host's C++ headers; same LLVM 19), on their compile commands without GCC's own flags.
host_clang_tidy=$(command -v clang-tidy-19 || command -v clang-tidy)
tools/run_unit_tests.sh
for unit in "${unit_test_units[@]}"; do
    project=$(sed -n 's|^\(tests/[^/]*\)/.*|\1|p' <<< "$unit")
    mkdir -p "$project/build/clang_tidy"
    sed -E 's/ -(fno-tree-switch-conversion|fstrict-volatile-bitfields|Wuseless-cast|Wduplicated-cond|Wduplicated-branches|Wlogical-op)\b//g' \
        "$project/build/compile_commands.json" > "$project/build/clang_tidy/compile_commands.json"
    CLANG_TIDY="$host_clang_tidy" tidy "$project/build/clang_tidy" "$unit"
done
tools/check_skin_api.sh
tools/check_docs.sh
echo "check_code: clean"
