#!/usr/bin/env bash
# Configure (if needed) + build for the Gambos PCB target.
#
# Usage:
#   ./software/project/scripts/build.sh
#   ./software/project/scripts/build.sh gambos-pcb
#
set -euo pipefail

PRESET="${1:-gambos-pcb}"

if [[ "${PRESET}" != "gambos-pcb" ]]; then
    echo "Usage: $0 [gambos-pcb]" >&2
    echo "  gambos-pcb  — Gambos PCB preset (default)" >&2
    exit 1
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
GEN_DIR="${ROOT}/build/${PRESET}/generated"
"${ROOT}/scripts/gen-board-sources.sh" "${PRESET}" "${GEN_DIR}"
cmake --preset "${PRESET}"
cmake --build "build/${PRESET}" --parallel
echo "OK: build/${PRESET}/gambos.elf"
