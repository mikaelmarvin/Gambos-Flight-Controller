#!/usr/bin/env bash
# Run CMake clean target for the devkit build directory.
# Usage: ./software/project/scripts/clean.sh [devkit]
set -euo pipefail
PRESET="${1:-devkit}"
if [[ "$PRESET" != "devkit" ]]; then
    echo "Usage: $0 [devkit]" >&2
    exit 1
fi
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
BUILD="build/${PRESET}"
if [[ ! -d "$BUILD" ]]; then
    echo "Nothing to clean: ${BUILD} missing (run build.sh first)." >&2
    exit 0
fi
cmake --build "$BUILD" --target clean
echo "OK: clean target for ${PRESET}"
