#!/usr/bin/env bash
# Flash gambos.elf via OpenOCD (ST-Link + SWD).
#
# Build first: ./software/project/scripts/build.sh
#
# Override binary path (optional): GAMBOS_FLASH_ELF=/path/to/gambos.elf $0
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ -n "${GAMBOS_FLASH_ELF:-}" ]]; then
    ELF="${GAMBOS_FLASH_ELF}"
else
    ELF="${ROOT}/build/devkit/gambos.elf"
fi

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Run: ./software/project/scripts/build.sh" >&2
    exit 1
fi

TARGET="stm32f4x.cfg"
echo "Flashing ${ELF} (target=${TARGET})"
exec openocd -f interface/stlink.cfg -f "target/${TARGET}" \
    -c "program ${ELF} verify reset exit"
