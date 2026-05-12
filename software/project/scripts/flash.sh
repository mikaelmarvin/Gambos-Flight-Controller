#!/usr/bin/env bash
#
# Flash gambos.elf for devkit (ST-Link) or custom (J-Link). You must pass the target explicitly.
#
# Usage:
#   ./software/project/scripts/flash.sh devkit    # build/devkit/gambos.elf via OpenOCD + ST-Link
#   ./software/project/scripts/flash.sh custom    # build/custom/gambos.elf via J-Link (JLinkExe preferred)
#
# Prerequisites: ./software/project/scripts/build.sh devkit   OR   ... build.sh custom
#
# Optional override: GAMBOS_FLASH_ELF=/path/to/gambos.elf (still requires devkit|custom for adapter choice)
# Custom/J-Link optional env: GAMBOS_JLINK_DEVICE, GAMBOS_JLINK_SPEED
#
set -euo pipefail

usage() {
    echo "Usage: $0 devkit | custom" >&2
    echo "  devkit  — program via ST-Link (Nucleo)" >&2
    echo "  custom  — program via SEGGER J-Link (custom PCB)" >&2
    exit 1
}

[[ $# -eq 1 ]] || usage
[[ "${1}" == "devkit" || "${1}" == "custom" ]] || usage
BOARD="$1"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEVICE="${GAMBOS_JLINK_DEVICE:-STM32F446RE}"
SPEED="${GAMBOS_JLINK_SPEED:-4000}"
OPENOCD_BOARD="${ROOT}/openocd"
OPENOCD_SYSTEM="/usr/share/openocd/scripts"

if [[ -n "${GAMBOS_FLASH_ELF:-}" ]]; then
    ELF="${GAMBOS_FLASH_ELF}"
else
    ELF="${ROOT}/build/${BOARD}/gambos.elf"
fi

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Build first: ./software/project/scripts/build.sh ${BOARD}" >&2
    exit 1
fi

flash_devkit() {
    local TARGET="stm32f4x.cfg"
    echo "Flash devkit (ST-Link): ${ELF}"
    exec openocd -f interface/stlink.cfg -f "target/${TARGET}" \
        -c "program ${ELF} verify reset exit"
}

flash_custom_jlink_exe() {
    local tmp
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
    echo "Flash custom (J-Link): ${ELF}  device=${DEVICE} SWD=${SPEED}kHz"
    exec JLinkExe -NoGui 1 -ExitOnError 1 -CommandFile "$tmp"
}

flash_custom_openocd() {
    echo "JLinkExe not found — flashing via OpenOCD (J-Link + SWD)." >&2
    echo "Rebuild the Dev Container so the Dockerfile installs SEGGER J-Link if possible." >&2
    exec openocd \
        -s "${OPENOCD_BOARD}" \
        -s "${OPENOCD_SYSTEM}" \
        -f "interface-jlink-swd.cfg" \
        -f "target/stm32f4x.cfg" \
        -c "program ${ELF} verify reset exit"
}

case "$BOARD" in
    devkit)
        flash_devkit
        ;;
    custom)
        if command -v JLinkExe >/dev/null 2>&1; then
            flash_custom_jlink_exe
        else
            flash_custom_openocd
        fi
        ;;
esac
