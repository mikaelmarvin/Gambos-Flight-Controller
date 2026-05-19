# Gambos documentation

Technical documentation for the Gambos flight controller: custom hardware (KiCad) and STM32 firmware (CMake + FreeRTOS).

## Project status

| Area | Status |
|------|--------|
| Hardware design and assembly | Complete (v1.0 manufactured) |
| Bring-up | In progress |
| Firmware | In progress (devkit + custom board) |

## Hardware

1. [Introduction](hardware/introduction.md) — goals and feature summary
2. [System architecture](hardware/architecture.md) — buses, actuation, debug interfaces
3. [Physical design](hardware/physical-design.md) — stackup, dimensions, PCB layout
4. [Power section](hardware/power.md) — input switching, LDO, servo rail
5. [Storage section](hardware/storage.md) — external flash and microSD
6. [Sensing section](hardware/sensing.md) — IMU, magnetometer, barometer
7. [User interface](hardware/user-interface.md) — RF module, button, LEDs
8. [Future improvements](hardware/future-improvements.md) — planned hardware and firmware work

See also [Hardware overview](hardware/hardware_overview.md) for revision status and repository pointers.

## Software

- [Software overview](software/index.md) — build, flash, and project layout (links to `software/README.md`)

## Repository entry points

- [Repository README](../README.md)
- [KiCad project](../hardware/gambos-pcb.kicad_pro)
- [Firmware](../software/)
