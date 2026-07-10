#!/usr/bin/env bash
# Configure (if needed) + build for the Gambos PCB target.
#
# Usage:
#   ./software/project/scripts/build.sh              # sync board tree, then build
#   ./software/project/scripts/build.sh --no-sync    # build only (day-to-day)
#
set -euo pipefail

NO_SYNC=0
for arg in "$@"; do
    if [[ "${arg}" == "--no-sync" ]]; then
        NO_SYNC=1
    elif [[ "${arg}" == "-h" || "${arg}" == "--help" ]]; then
        echo "Usage: $0 [--no-sync]" >&2
        echo "  (default)  sync board/gambos-pcb from upstream + patches, then build" >&2
        echo "  --no-sync  skip sync; use existing board/gambos-pcb/" >&2
        exit 0
    elif [[ -n "${arg}" ]]; then
        echo "Usage: $0 [--no-sync]" >&2
        exit 1
    fi
done

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BOARD_TREE="${ROOT}/board/gambos-pcb"
cd "$ROOT"

if [[ "${NO_SYNC}" -eq 0 ]]; then
    "${ROOT}/scripts/sync-cubemx-gambos-pcb.sh"
elif [[ ! -f "${BOARD_TREE}/cmake/stm32cubemx/CMakeLists.txt" ]]; then
    echo "Missing ${BOARD_TREE}/ (not synced)." >&2
    echo "Run: ./software/project/scripts/sync-cubemx-gambos-pcb.sh" >&2
    echo "  or: ./software/project/scripts/build.sh" >&2
    exit 1
fi

cmake --preset gambos-pcb
cmake --build build/gambos-pcb --parallel
echo "OK: build/gambos-pcb/gambos.elf"
