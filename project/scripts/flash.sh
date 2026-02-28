#!/bin/bash
# Flash gambos.elf to the target via OpenOCD (ST-Link). Run from repo root.
set -e
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ELF="${REPO_ROOT}/project/build/Debug/gambos.elf"
[[ -f "$ELF" ]] || { echo "Not found: $ELF. Run build first."; exit 1; }
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program $ELF verify reset exit"
