#!/usr/bin/env bash
# Flash gambos.elf via OpenOCD (ST-Link + SWD).
#
# Presets select CMake BOARD=… and thus which app is linked:
#   devkit  — app/devkit (C++: button, messaging, app_run) — default
#   custom  — app/custom/app.c only
#
# Build the same preset first: ./software/project/scripts/build.sh [devkit|custom]
#
# Override binary path (optional): GAMBOS_FLASH_ELF=/path/to/gambos.elf $0
set -euo pipefail
PRESET="${1:-devkit}"
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
if [[ -n "${GAMBOS_FLASH_ELF:-}" ]]; then
    ELF="${GAMBOS_FLASH_ELF}"
else
    ELF="${ROOT}/build/${PRESET}/gambos.elf"
fi

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Run: ./software/project/scripts/build.sh ${PRESET}" >&2
    echo "(Preset devkit links app/devkit; custom links app/custom only.)" >&2
    exit 1
fi

echo "Flashing ${ELF} (preset=${PRESET} target=${TARGET})"
exec openocd -f interface/stlink.cfg -f "target/${TARGET}" \
    -c "program ${ELF} verify reset exit"
