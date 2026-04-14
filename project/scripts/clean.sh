#!/usr/bin/env bash
# Run CMake clean target for a preset build directory.
# Usage: ./project/scripts/clean.sh [preset]
set -euo pipefail
PRESET="${1:-custom}"
case "$PRESET" in
    custom | devkit) ;;
    *)
        echo "Usage: $0 [custom|devkit]" >&2
        exit 1
        ;;
esac
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
BUILD="build/${PRESET}"
if [[ ! -d "$BUILD" ]]; then
    echo "Nothing to clean: ${BUILD} missing (run build.sh first)." >&2
    exit 0
fi
cmake --build "$BUILD" --target clean
echo "OK: clean target for ${PRESET}"
