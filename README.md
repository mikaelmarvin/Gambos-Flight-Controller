# The Gambos Flight Controller Project

Custom embedded system project built to practice and demonstrate complete hardware + firmware workflow:
from schematic and PCB design to STM32 firmware bring-up.

## Why this repository exists

This is a portfolio-oriented engineering project focused on:

- Designing a real custom board in KiCad
- Building firmware for STM32F446 with CMake + FreeRTOS
- Managing an end-to-end embedded workflow in a reproducible Dev Container setup
- Driven by curiosity

## Current status

- Hardware design files and manufacturing outputs are in the repository
- PCBs have been ordered
- Assembly and bring-up are the next phase
- Firmware build environment is set up and documented

## Repository structure


| Directory                | Purpose                                                                                  |
| ------------------------ | ---------------------------------------------------------------------------------------- |
| `[hardware/](hardware/)` | KiCad project files, schematics, PCB layout, libraries, 3D models, and generated Gerbers |
| `[software/](software/)` | STM32 firmware project, build scripts, dev container workflow, debug/flash scripts       |


## Start here

- Hardware assets: open `hardware/gambos-pcb.kicad_pro` in KiCad
- Firmware setup/build/flash: see `[software/README.md](software/README.md)`
- Board and app internals: `software/project/board/README.md` and `software/project/app/README.md`

## Engineering highlights

- STM32F446-based firmware project with a structured CMake build
- FreeRTOS-based software stack and board/application separation for easier code generation flow
- Custom hardware design split by subsystem (MCU, power, sensors/flash, connectors)
- Reproducible development environment via Docker/Dev Container

## Next milestones

- Assemble and inspect first PCB revision
- Execute staged bring-up (power rails, SWD/debug access, clocks, peripherals)
- Publish validation artifacts (photos, measurements, test notes)

