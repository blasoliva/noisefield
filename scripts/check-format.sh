#!/usr/bin/env bash
# Checks (or fixes, with --fix) clang-format compliance for Noisefield's own sources.
# JUCE, Catch2 and the build tree are never touched.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

mode="check"
if [[ "${1:-}" == "--fix" ]]; then
    mode="fix"
fi

clang_format="${CLANG_FORMAT:-clang-format}"
if ! command -v "$clang_format" >/dev/null 2>&1; then
    echo "error: $clang_format not found (set CLANG_FORMAT or install clang-format >= 14)" >&2
    exit 127
fi

mapfile -d '' files < <(find src tests -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \) -print0)

if [[ ${#files[@]} -eq 0 ]]; then
    echo "no source files found"
    exit 0
fi

if [[ "$mode" == "fix" ]]; then
    "$clang_format" -i "${files[@]}"
    echo "formatted ${#files[@]} file(s)"
else
    if ! "$clang_format" --dry-run --Werror "${files[@]}"; then
        echo >&2
        echo "error: formatting issues found -- run 'scripts/check-format.sh --fix'" >&2
        exit 1
    fi
    echo "formatting OK (${#files[@]} file(s))"
fi
