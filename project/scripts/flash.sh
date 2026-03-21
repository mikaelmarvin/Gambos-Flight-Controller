#!/bin/bash
# Flash gambos.elf to the target via OpenOCD (ST-Link). Run from repo root.
# Usage: ./project/scripts/flash.sh [custom|devkit]   (default: custom)
set -e
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BOARD="${1:-custom}"
if [[ "$BOARD" != "custom" && "$BOARD" != "devkit" ]]; then
  echo "Usage: $0 [custom|devkit]"
  exit 1
fi
ELF="${REPO_ROOT}/project/build/${BOARD}/gambos.elf"
[[ -f "$ELF" ]] || { echo "Not found: $ELF. Build first: cd project && cmake --preset $BOARD && cmake --build build/$BOARD -j"; exit 1; }
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program $ELF verify reset exit"
