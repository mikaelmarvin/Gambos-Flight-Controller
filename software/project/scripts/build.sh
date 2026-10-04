#!/usr/bin/env bash
# Configure (if needed) + build for the Gambos PCB target.
#
# Usage:
#   ./software/project/scripts/build.sh
#
set -euo pipefail

if [[ $# -gt 0 ]]; then
    echo "Usage: $0" >&2
    exit 1
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BOARD_TREE="${ROOT}/board/gambos-pcb"
cd "$ROOT"

if [[ ! -f "${BOARD_TREE}/cmake/stm32cubemx/CMakeLists.txt" ]]; then
    echo "Missing ${BOARD_TREE}/cmake/stm32cubemx/CMakeLists.txt" >&2
    exit 1
fi

cmake --preset gambos-pcb
cmake --build build/gambos-pcb --parallel
echo "OK: build/gambos-pcb/gambos.elf"
