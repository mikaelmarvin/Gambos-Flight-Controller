#!/usr/bin/env bash
#
# Probe SWD attach for devkit (ST-Link) or custom (J-Link). You must pass the target explicitly.
#
# Usage:
#   ./software/project/scripts/probe.sh devkit    # Nucleo on-board ST-Link + OpenOCD
#   ./software/project/scripts/probe.sh custom    # SEGGER J-Link via JLinkExe
#
# Environment (custom only, optional): GAMBOS_JLINK_DEVICE, GAMBOS_JLINK_SPEED
#
set -euo pipefail

usage() {
    echo "Usage: $0 devkit | custom" >&2
    echo "  devkit  — ST-Link on STM32 Nucleo-F446RE (OpenOCD)" >&2
    echo "  custom  — SEGGER J-Link + Gambos PCB (JLinkExe)" >&2
    exit 1
}

[[ $# -eq 1 ]] || usage
[[ "${1}" == "devkit" || "${1}" == "custom" ]] || usage
BOARD="$1"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEVICE="${GAMBOS_JLINK_DEVICE:-STM32F446RE}"
SPEED="${GAMBOS_JLINK_SPEED:-4000}"

probe_devkit() {
    exec openocd -f interface/stlink.cfg -f "target/stm32f4x.cfg" -c "init" -c "exit"
}

probe_custom() {
    if ! command -v JLinkExe >/dev/null 2>&1; then
        echo "JLinkExe not found. Rebuild the Dev Container (SEGGER J-Link is installed from the Dockerfile on amd64/arm64)." >&2
        exit 1
    fi

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

case "$BOARD" in
    devkit) probe_devkit ;;
    custom) probe_custom ;;
esac
