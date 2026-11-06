#!/usr/bin/env bash
# Configure (if needed) + build for the Gambos target.
#
# Usage:
#   ./project/scripts/build.sh
#
set -euo pipefail

if [[ $# -gt 0 ]]; then
    echo "Error: The script $0 expects zero arguments" >&2
    exit 1
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BOARD_TREE="${ROOT}/board"
cd "$ROOT"

if [[ ! -f "${BOARD_TREE}/cmake/stm32cubemx/CMakeLists.txt" ]]; then
    echo "Missing ${BOARD_TREE}/cmake/stm32cubemx/CMakeLists.txt" >&2
    exit 1
fi

cmake --preset gambos
cmake --build build --parallel
echo "OK: build/gambos.elf"
