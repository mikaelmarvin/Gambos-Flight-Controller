#!/usr/bin/env bash
# Flash gambos.elf via OpenOCD (ST-Link + SWD).
# Usage: ./project/scripts/flash.sh [preset]
# Build first: ./project/scripts/build.sh <same preset>
set -euo pipefail
PRESET="${1:-custom}"
case "$PRESET" in
    custom | devkit)
        TARGET="stm32f4x.cfg"
        ;;
    *)
        echo "Usage: $0 [custom|devkit]" >&2
        exit 1
        ;;
esac

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ELF="${ROOT}/build/${PRESET}/gambos.elf"

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Run: ./project/scripts/build.sh ${PRESET}" >&2
    exit 1
fi

echo "Flashing ${ELF} (${PRESET} / ${TARGET})"
exec openocd -f interface/stlink.cfg -f "target/${TARGET}" \
    -c "program ${ELF} verify reset exit"
