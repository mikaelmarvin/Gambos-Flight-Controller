#!/usr/bin/env bash
# Probe ST-Link + MCU over SWD (OpenOCD). Exits 0 if the adapter and target respond.
# Usage: ./software/project/scripts/probe.sh
set -euo pipefail
exec openocd -f interface/stlink.cfg -f "target/stm32f4x.cfg" -c "init" -c "exit"
