#!/usr/bin/env bash
# Remove a preset build directory (or all of software/project/build). Reconfigure with build.sh or cmake --preset.
# Usage: ./software/project/scripts/pristine.sh [preset|all]
#   preset — custom | devkit (default: custom)
#   all    — delete entire software/project/build/
set -euo pipefail
MODE="${1:-custom}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ "$MODE" == "all" ]]; then
    rm -rf build
    echo "OK: removed software/project/build/. Run: cmake --preset <preset> (from software/project/)"
    exit 0
fi

PRESET="$MODE"
case "$PRESET" in
    custom | devkit) ;;
    *)
        echo "Usage: $0 [custom|devkit|all]" >&2
        exit 1
        ;;
esac

rm -rf "build/${PRESET}"
echo "OK: removed build/${PRESET}. Run: ./software/project/scripts/build.sh ${PRESET}"
