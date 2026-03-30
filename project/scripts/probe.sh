#!/usr/bin/env bash
# Probe ST-Link + MCU over SWD (OpenOCD). Exits 0 if the adapter and target respond.
# Usage: ./project/scripts/probe.sh [preset]
# Presets: custom | devkit (STM32F4) — default custom; bluepill (STM32F1)
set -euo pipefail
PRESET="${1:-custom}"
case "$PRESET" in
    custom | devkit)
        TARGET="stm32f4x.cfg"
        ;;
    bluepill)
        TARGET="stm32f1x.cfg"
        ;;
    *)
        echo "Usage: $0 [custom|devkit|bluepill]" >&2
        exit 1
        ;;
esac

exec openocd -f interface/stlink.cfg -f "target/${TARGET}" -c "init" -c "exit"
