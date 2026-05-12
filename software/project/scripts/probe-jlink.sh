#!/usr/bin/env bash
# Probe SEGGER J-Link + STM32F4 over SWD (container-friendly).
#
# Uses JLinkExe when installed (see Dockerfile); otherwise OpenOCD fallback.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEVICE="${GAMBOS_JLINK_DEVICE:-STM32F446RE}"
SPEED="${GAMBOS_JLINK_SPEED:-4000}"

OPENOCD_BOARD="${ROOT}/openocd"
OPENOCD_SYSTEM="/usr/share/openocd/scripts"

probe_with_jlink_exe() {
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
    echo "Probing via JLinkExe (device=${DEVICE}, SWD)"
    exec JLinkExe -NoGui 1 -ExitOnError 1 -CommandFile "$tmp"
}

probe_with_openocd() {
    echo "JLinkExe not found — probing with OpenOCD (J-Link + SWD)." >&2
    echo "Install SEGGER tools in the image: Dev Containers → Rebuild Container (Dockerfile installs JLinkExe)." >&2
    echo "OpenOCD + libjaylink often reports \"No J-Link device found\" in Docker even when lsusb shows the probe." >&2
    exec openocd \
        -s "${OPENOCD_BOARD}" \
        -s "${OPENOCD_SYSTEM}" \
        -f "interface-jlink-swd.cfg" \
        -f "target/stm32f4x.cfg" \
        -c "init" \
        -c "exit"
}

if command -v JLinkExe >/dev/null 2>&1; then
    probe_with_jlink_exe
else
    probe_with_openocd
fi
