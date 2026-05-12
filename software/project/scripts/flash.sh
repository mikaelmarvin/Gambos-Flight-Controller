#!/usr/bin/env bash
# Flash gambos.elf via OpenOCD (ST-Link + SWD).
#
# Build first: ./software/project/scripts/build.sh [devkit|custom]
#
# Usage:
#   ./software/project/scripts/flash.sh           # build/devkit/gambos.elf
#   ./software/project/scripts/flash.sh custom    # build/custom/gambos.elf
# Override binary path: GAMBOS_FLASH_ELF=/path/to/gambos.elf
# Or: GAMBOS_BOARD=custom
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PRESET="${GAMBOS_BOARD:-devkit}"
if [[ -n "${1:-}" ]]; then
    case "$1" in
        devkit|custom) PRESET="$1" ;;
        *)
            echo "Usage: $0 [devkit|custom]" >&2
            exit 1
            ;;
    esac
fi

if [[ -n "${GAMBOS_FLASH_ELF:-}" ]]; then
    ELF="${GAMBOS_FLASH_ELF}"
else
    ELF="${ROOT}/build/${PRESET}/gambos.elf"
fi

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Run: ./software/project/scripts/build.sh ${PRESET}" >&2
    exit 1
fi

TARGET="stm32f4x.cfg"
echo "Flashing ${ELF} (preset=${PRESET}, target=${TARGET})"
exec openocd -f interface/stlink.cfg -f "target/${TARGET}" \
    -c "program ${ELF} verify reset exit"
