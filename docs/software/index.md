# Software overview

Firmware for Gambos lives under [`software/`](../../software/). The project targets:

- **STM32F446** on the custom board (`custom` build target)
- **STM32F446 Nucleo/devkit** for early development (`devkit` build target)

## Stack

- **CMake** build with board/application split
- **FreeRTOS**
- **Dev Container** at repo root for reproducible builds (see top-level [README](../../README.md))

## Getting started

Full build, flash, and debug instructions:

**[software/README.md](../../software/README.md)**

## Related documentation

- [Gambos board](../hardware/gambos-board.md)
- [System architecture](../hardware/architecture.md)
- [Future improvements](../hardware/future-improvements.md)
