#!/usr/bin/env bash
# Flash gambos.elf via SEGGER J-Link (recommended from the dev container).
#
# Requires a rebuilt image that installs SEGGER J-Link (see repo Dockerfile).
# Build first: ./software/project/scripts/build.sh [devkit|custom]
#
# Usage:
#   ./software/project/scripts/flash-jlink.sh           # flashes build/devkit/gambos.elf
#   ./software/project/scripts/flash-jlink.sh custom  # flashes build/custom/gambos.elf
# Env:
#   GAMBOS_BOARD=custom  same as passing "custom" (if GAMBOS_FLASH_ELF is unset)
#   GAMBOS_FLASH_ELF=/path/to/gambos.elf  — overrides preset path
#   GAMBOS_JLINK_DEVICE   default STM32F446RE
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

DEVICE="${GAMBOS_JLINK_DEVICE:-STM32F446RE}"
SPEED="${GAMBOS_JLINK_SPEED:-4000}"

if [[ ! -f "$ELF" ]]; then
    echo "Missing: $ELF" >&2
    echo "Run: ./software/project/scripts/build.sh ${PRESET}" >&2
    exit 1
fi

OPENOCD_BOARD="${ROOT}/openocd"
OPENOCD_SYSTEM="/usr/share/openocd/scripts"

flash_with_jlink_exe() {
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
    echo "Flashing ${ELF} via JLinkExe (preset=${PRESET}, device=${DEVICE}, SWD)"
    exec JLinkExe -NoGui 1 -ExitOnError 1 -CommandFile "$tmp"
}

flash_with_openocd() {
    echo "JLinkExe not found — flashing via OpenOCD (J-Link + SWD)." >&2
    echo "For reliable J-Link flashing: Dev Containers → Rebuild Container (Dockerfile installs JLinkExe)." >&2
    exec openocd \
        -s "${OPENOCD_BOARD}" \
        -s "${OPENOCD_SYSTEM}" \
        -f "interface-jlink-swd.cfg" \
        -f "target/stm32f4x.cfg" \
        -c "program ${ELF} verify reset exit"
}

if command -v JLinkExe >/dev/null 2>&1; then
    flash_with_jlink_exe
else
    flash_with_openocd
fi
