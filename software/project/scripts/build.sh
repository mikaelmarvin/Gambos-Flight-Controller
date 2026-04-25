#!/usr/bin/env bash
# Configure (if needed) and build. Run from repo: ./software/project/scripts/build.sh [devkit]
set -euo pipefail
PRESET="${1:-devkit}"
if [[ "$PRESET" != "devkit" ]]; then
    echo "Usage: $0 [devkit]" >&2
    exit 1
fi
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
GEN_DIR="${ROOT}/build/${PRESET}/generated"
"${ROOT}/scripts/gen-board-sources.sh" "${PRESET}" "${GEN_DIR}"
cmake --preset "${PRESET}"
cmake --build "build/${PRESET}" --parallel
echo "OK: build/${PRESET}/gambos.elf"
