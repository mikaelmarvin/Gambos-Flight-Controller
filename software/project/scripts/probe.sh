#!/usr/bin/env bash
#
# Probe SWD attach for the Gambos PCB via SEGGER J-Link.
#
# Usage:
#   ./software/project/scripts/probe.sh
#   ./software/project/scripts/probe.sh gambos-pcb
#
# Environment (optional): GAMBOS_JLINK_DEVICE, GAMBOS_JLINK_SPEED
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
exit
EOF
echo "Probe gambos-pcb (J-Link): device=${DEVICE} SWD=${SPEED}kHz"
exec JLinkExe -NoGui 1 -ExitOnError 1 -CommandFile "$tmp"
