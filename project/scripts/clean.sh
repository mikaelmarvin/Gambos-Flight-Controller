#!/usr/bin/env bash
# Run CMake clean for the Gambos build.
#
# Usage:
#   ./project/scripts/clean.sh
#
set -euo pipefail

if [[ $# -gt 0 ]]; then
    echo "Error: The script $0 expects zero arguments" >&2
    exit 1
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
BUILD="build"
if [[ ! -d "$BUILD" ]]; then
    echo "Nothing to clean: ${BUILD} missing (run build.sh first)." >&2
    exit 0
fi
cmake --build "$BUILD" --target clean
echo "OK: clean target for ${BUILD}"
