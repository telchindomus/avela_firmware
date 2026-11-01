#!/usr/bin/env bash
# Keeps the docs pointing at what exists; run by tools/check_code.sh. Every file or folder a doc names in backticks
# (`docs/developer/BUILD.md`, `avela_ui/view_concept.hpp`, `partitions.csv`, `tests/`) must be in the repository: a path
# is found when a tracked file's path ends with it, a bare file name when a tracked file has that name. Names with
# placeholders (<component>, *, ...) are patterns, not files, and are skipped; so are a bare extension (`.cpp`) and
# absolute paths (`/dev/ttyUSB0`: the host's, not the repository's).
# Named on purpose but never in the repository: what the build downloads or generates, and registry names.
outside=(managed_components/ build_clang/ build_clang_optional/ sdkconfig.h espressif/catch2
    bootloader/bootloader.bin partition_table/partition-table.bin cicd/.env cicd/build version.txt)
set -euo pipefail
cd "$(dirname "$0")/.."

mapfile -t docs < <(git ls-files 'README.md' 'AGENTS.md' 'docs/*.md' 'components/*/AGENTS.md' 'components/*/README.md' 'components/*/docs/*.md')
tracked=$(git ls-files --cached --others --exclude-standard)

failed=0
for doc in "${docs[@]}"; do
    # backticked tokens that look like a file or folder: a known extension, or a slash
    while IFS= read -r name; do
        [[ "$name" =~ [\<\>\*\$\{\}\ ]|\.\.\. || "$name" =~ ^\.[a-z]+$ || ! "$name" =~ [A-Za-z0-9] ]] && continue
        [[ "$name" =~ ^Avela/[0-9]+\.[0-9]+\.[0-9]+(-dev)?$ ]] && continue
        [[ "$name" == /* || " ${outside[*]} " == *" $name "* ]] && continue
        if [[ "$name" == */* ]]; then
            path=${name%/}
            grep -qE "(^|/)${path//./\\.}(/|$)" <<< "$tracked" && continue
        else
            grep -qE "(^|/)${name//./\\.}$" <<< "$tracked" && continue
        fi
        echo "check_docs: $doc names \`$name\`, which isn't in the repository" >&2
        failed=1
    done < <(grep -oE '`[^`]+`' "$doc" | tr -d '`' |
        grep -E '^[A-Za-z0-9_./-]+$' | grep -E '/|\.(md|hpp|cpp|h|c|sh|py|csv|txt|yml|json|cmake|argb)$' | sort -u)
done
if ! python3 tools/test_doc_links.py; then
    failed=1
fi
if ! python3 tools/check_doc_links.py "${docs[@]}"; then
    failed=1
fi
exit "$failed"
