#!/bin/bash
# Check that the devkit (or any ST-Link + STM32F4x target) is connected.
# Run from repo root. Uses the same OpenOCD config as flash.sh.
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$REPO_ROOT"

echo "Probing for ST-Link + STM32F4x..."
if openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init" -c "exit" 2>&1; then
  echo "Target found: devkit/board is connected."
  exit 0
else
  echo "No target found. Check USB (ST-Link) and try again."
  exit 1
fi
