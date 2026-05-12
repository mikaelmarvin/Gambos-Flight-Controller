#!/usr/bin/env bash
# Configure (if needed) + build. First argument is required (no default).
#
# Usage:
#   ./software/project/scripts/build.sh devkit
#   ./software/project/scripts/build.sh custom
#
# Flash/probe use the same preset name: ./software/project/scripts/flash.sh devkit|custom
#
set -euo pipefail

usage() {
    echo "Usage: $0 devkit | custom" >&2
    echo "  devkit  — Nucleo-F446RE preset" >&2
    echo "  custom  — Gambos PCB preset" >&2
    exit 1
}

[[ $# -eq 1 ]] || usage
[[ "${1}" == "devkit" || "${1}" == "custom" ]] || usage

PRESET="$1"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
GEN_DIR="${ROOT}/build/${PRESET}/generated"
"${ROOT}/scripts/gen-board-sources.sh" "${PRESET}" "${GEN_DIR}"
cmake --preset "${PRESET}"
cmake --build "build/${PRESET}" --parallel
echo "OK: build/${PRESET}/gambos.elf"
