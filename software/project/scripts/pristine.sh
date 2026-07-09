#!/usr/bin/env bash
# Remove CMake build output under software/project/build/.
#
# Usage:
#   ./software/project/scripts/pristine.sh              # delete entire build/
#   ./software/project/scripts/pristine.sh all          # same as no argument
#   ./software/project/scripts/pristine.sh gambos-pcb   # only build/gambos-pcb/
#
# Then reconfigure with: ./software/project/scripts/build.sh
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

usage() {
    echo "Usage: $0 [ gambos-pcb | all ]" >&2
    echo "  (no argument) or all  — remove entire software/project/build/" >&2
    echo "  gambos-pcb            — remove only build/gambos-pcb/" >&2
    exit 1
}

if [[ $# -eq 0 ]]; then
    rm -rf build
    echo "OK: removed software/project/build/."
    echo "Run: ./software/project/scripts/build.sh"
    exit 0
fi

[[ $# -eq 1 ]] || usage

case "$1" in
    all)
        rm -rf build
        echo "OK: removed software/project/build/."
        echo "Run: ./software/project/scripts/build.sh"
        ;;
    gambos-pcb)
        rm -rf "build/${1}"
        echo "OK: removed build/${1}/."
        echo "Run: ./software/project/scripts/build.sh"
        ;;
    *)
        usage
        ;;
esac
