#!/usr/bin/env bash
#
# Probe SWD attach for devkit (ST-Link) or custom (J-Link). You must pass the target explicitly.
#
# Usage:
#   ./software/project/scripts/probe.sh devkit    # Nucleo on-board ST-Link + OpenOCD
#   ./software/project/scripts/probe.sh custom    # SEGGER J-Link — JLinkExe if installed, else OpenOCD
#
# Environment (custom only, optional): GAMBOS_JLINK_DEVICE, GAMBOS_JLINK_SPEED
#
set -euo pipefail

usage() {
    echo "Usage: $0 devkit | custom" >&2
    echo "  devkit  — ST-Link on STM32 Nucleo-F446RE (OpenOCD)" >&2
    echo "  custom  — SEGGER J-Link + Gambos PCB (JLinkExe preferred)" >&2
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

probe_devkit() {
    exec openocd -f interface/stlink.cfg -f "target/stm32f4x.cfg" -c "init" -c "exit"
}

probe_custom_jlink_exe() {
    local tmp
    tmp="$(mktemp)"
    cleanup() { rm -f "$tmp"; }
    trap cleanup EXIT

    cat >"$tmp" <<EOF
device ${DEVICE}
si 1
speed ${SPEED}
connect
exit
EOF
    echo "Probe custom (J-Link): device=${DEVICE} SWD=${SPEED}kHz"
    exec JLinkExe -NoGui 1 -ExitOnError 1 -CommandFile "$tmp"
}

probe_custom_openocd() {
    echo "JLinkExe not found — probing with OpenOCD (J-Link + SWD)." >&2
    echo "Rebuild the Dev Container so the Dockerfile installs SEGGER J-Link if possible." >&2
    echo "OpenOCD + libjaylink often fails in Docker even when lsusb shows the probe." >&2
    exec openocd \
        -s "${OPENOCD_BOARD}" \
        -s "${OPENOCD_SYSTEM}" \
        -f "interface-jlink-swd.cfg" \
        -f "target/stm32f4x.cfg" \
        -c "init" \
        -c "exit"
}

case "$BOARD" in
    devkit)
        probe_devkit
        ;;
    custom)
        if command -v JLinkExe >/dev/null 2>&1; then
            probe_custom_jlink_exe
        else
            probe_custom_openocd
        fi
        ;;
esac
