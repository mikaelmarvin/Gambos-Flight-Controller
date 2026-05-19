#!/usr/bin/env bash
# Run CMake clean for one preset. First argument is required (no default).
#
# Usage:
#   ./software/project/scripts/clean.sh devkit
#   ./software/project/scripts/clean.sh custom
#
set -euo pipefail

usage() {
    echo "Usage: $0 devkit | custom" >&2
    exit 1
}

[[ $# -eq 1 ]] || usage
[[ "${1}" == "devkit" || "${1}" == "custom" ]] || usage

PRESET="$1"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
BUILD="build/${PRESET}"
if [[ ! -d "$BUILD" ]]; then
    echo "Nothing to clean: ${BUILD} missing (run build.sh ${PRESET} first)." >&2
    exit 0
fi
cmake --build "$BUILD" --target clean
echo "OK: clean target for ${PRESET}"
