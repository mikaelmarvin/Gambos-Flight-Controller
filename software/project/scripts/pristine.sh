#!/usr/bin/env bash
# Remove one preset's build directory (or all of software/project/build). Reconfigure with build.sh.
# Usage: ./software/project/scripts/pristine.sh [devkit|custom|all]
#   devkit (default) — delete software/project/build/devkit/
#   custom             — delete software/project/build/custom/
#   all                — delete entire software/project/build/
set -euo pipefail
MODE="${1:-devkit}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ "$MODE" == "all" ]]; then
    rm -rf build
    echo "OK: removed software/project/build/. Run: ./software/project/scripts/build.sh devkit (or custom)"
    exit 0
fi

if [[ "$MODE" != "devkit" && "$MODE" != "custom" ]]; then
    echo "Usage: $0 [devkit|custom|all]" >&2
    exit 1
fi

rm -rf "build/${MODE}"
echo "OK: removed build/${MODE}. Run: ./software/project/scripts/build.sh ${MODE}"
