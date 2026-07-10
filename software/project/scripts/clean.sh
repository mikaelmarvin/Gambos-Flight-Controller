#!/usr/bin/env bash
# Run CMake clean for the Gambos PCB preset.
#
# Usage:
#   ./software/project/scripts/clean.sh
#   ./software/project/scripts/clean.sh gambos-pcb
#
set -euo pipefail

PRESET="${1:-gambos-pcb}"

if [[ "${PRESET}" != "gambos-pcb" ]]; then
    echo "Usage: $0 [gambos-pcb]" >&2
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
