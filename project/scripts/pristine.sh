#!/usr/bin/env bash
# Remove a preset build directory (or all of project/build). Reconfigure with build.sh or cmake --preset.
# Usage: ./project/scripts/pristine.sh [preset|all]
#   preset — custom | devkit | bluepill (default: custom)
#   all    — delete entire project/build/
set -euo pipefail
MODE="${1:-custom}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ "$MODE" == "all" ]]; then
    rm -rf build
    echo "OK: removed project/build/. Run: cmake --preset <preset> (from project/)"
    exit 0
fi

PRESET="$MODE"
case "$PRESET" in
    custom | devkit | bluepill) ;;
    *)
        echo "Usage: $0 [custom|devkit|bluepill|all]" >&2
        exit 1
        ;;
esac

rm -rf "build/${PRESET}"
echo "OK: removed build/${PRESET}. Run: ./project/scripts/build.sh ${PRESET}"
