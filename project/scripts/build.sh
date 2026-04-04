#!/usr/bin/env bash
# Configure (if needed) and build. Run from repo: ./project/scripts/build.sh [preset]
# Presets: custom (default), devkit
set -euo pipefail
PRESET="${1:-custom}"
case "$PRESET" in
    custom | devkit) ;;
    *)
        echo "Usage: $0 [custom|devkit]" >&2
        exit 1
        ;;
esac
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
cmake --preset "${PRESET}"
cmake --build "build/${PRESET}" --parallel
echo "OK: build/${PRESET}/gambos.elf"
