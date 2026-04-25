#!/usr/bin/env bash
# Remove the devkit build directory (or all of software/project/build). Reconfigure with build.sh or cmake --preset.
# Usage: ./software/project/scripts/pristine.sh [devkit|all]
#   devkit (default) — delete software/project/build/devkit/
#   all              — delete entire software/project/build/
set -euo pipefail
MODE="${1:-devkit}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ "$MODE" == "all" ]]; then
    rm -rf build
    echo "OK: removed software/project/build/. Run: cmake --preset devkit (from software/project/)"
    exit 0
fi

if [[ "$MODE" != "devkit" ]]; then
    echo "Usage: $0 [devkit|all]" >&2
    exit 1
fi

rm -rf "build/${MODE}"
echo "OK: removed build/${MODE}. Run: ./software/project/scripts/build.sh"
