#!/usr/bin/env bash
#
# Flash gambos.elf for the Gambos PCB via SEGGER J-Link.
#
# Usage:
#   ./software/project/scripts/flash.sh
#   ./software/project/scripts/flash.sh gambos-pcb
#
# Prerequisites: ./software/project/scripts/build.sh
#
# Optional override: GAMBOS_FLASH_ELF=/path/to/gambos.elf
# Optional env: GAMBOS_JLINK_DEVICE, GAMBOS_JLINK_SPEED
#
set -euo pipefail

BOARD="${1:-gambos-pcb}"

if [[ "${BOARD}" != "gambos-pcb" ]]; then
    echo "Usage: $0 [gambos-pcb]" >&2
    exit 1
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEVICE="${GAMBOS_JLINK_DEVICE:-STM32F446RE}"
SPEED="${GAMBOS_JLINK_SPEED:-4000}"

if [[ -n "${GAMBOS_FLASH_ELF:-}" ]]; then
    ELF="${GAMBOS_FLASH_ELF}"
else
    ELF="${ROOT}/build/${BOARD}/gambos.elf"
fi

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Build first: ./software/project/scripts/build.sh" >&2
    exit 1
fi

if ! command -v JLinkExe >/dev/null 2>&1; then
    echo "JLinkExe not found. Rebuild the Dev Container (SEGGER J-Link is installed from the Dockerfile on amd64/arm64)." >&2
    exit 1
fi

tmp="$(mktemp)"
cleanup() { rm -f "$tmp"; }
trap cleanup EXIT

cat >"$tmp" <<EOF
device ${DEVICE}
si 1
speed ${SPEED}
connect
loadfile ${ELF}
r
g
exit
EOF
echo "Flash gambos-pcb (J-Link): ${ELF}  device=${DEVICE} SWD=${SPEED}kHz"
exec JLinkExe -NoGui 1 -ExitOnError 1 -CommandFile "$tmp"
