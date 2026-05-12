#!/usr/bin/env bash
# Remove CMake build output under software/project/build/.
#
# Usage:
#   ./software/project/scripts/pristine.sh              # delete entire build/ (all presets)
#   ./software/project/scripts/pristine.sh all           # same as no argument
#   ./software/project/scripts/pristine.sh devkit        # only build/devkit/
#   ./software/project/scripts/pristine.sh custom        # only build/custom/
#
# Then reconfigure with: ./software/project/scripts/build.sh devkit | custom
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

usage() {
    echo "Usage: $0 [ devkit | custom | all ]" >&2
    echo "  (no argument) or all  — remove entire software/project/build/" >&2
    echo "  devkit                — remove only build/devkit/" >&2
    echo "  custom                — remove only build/custom/" >&2
    exit 1
}

if [[ $# -eq 0 ]]; then
    rm -rf build
    echo "OK: removed software/project/build/ (all presets)."
    echo "Run: ./software/project/scripts/build.sh devkit   # or custom"
    exit 0
fi

[[ $# -eq 1 ]] || usage

case "$1" in
    all)
        rm -rf build
        echo "OK: removed software/project/build/ (all presets)."
        echo "Run: ./software/project/scripts/build.sh devkit   # or custom"
        ;;
    devkit | custom)
        rm -rf "build/${1}"
        echo "OK: removed build/${1}/."
        echo "Run: ./software/project/scripts/build.sh ${1}"
        ;;
    *)
        usage
        ;;
esac
