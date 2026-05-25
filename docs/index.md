# Gambos documentation

Technical documentation for the Gambos flight controller: custom hardware (KiCad) and STM32 firmware (CMake + FreeRTOS).

**Start here:** [repository README](../README.md) — project overview, block diagram, bring-up, and links into hardware and software docs.

**Schematic (v1.0):** [Gambos PCB schematic (PDF)](gambos-pcb.pdf) — full KiCad export for review.

## Hardware

Subsystem pages (read in order; each links to the next):

1. [Hardware architecture](hardware/hardware-architecture.md) — buses, actuation, debug interfaces
2. [Physical design](hardware/physical-design.md) — stackup, dimensions, PCB layout
3. [Power](hardware/power.md) — input switching, LDO, servo rail
4. [Storage](hardware/storage.md) — external flash and microSD
5. [Sensing](hardware/sensing.md) — IMU, magnetometer, barometer
6. [User interface](hardware/user-interface.md) — RF module, button, LEDs
7. [Roadmap](hardware/roadmap.md) — v1.0 open work and planned next hardware iterations

## Software

- [Software architecture](software/software-architecture.md) — firmware layers and placeholders for stack docs
- [Build, flash, debug](../software/README.md)

## Repository entry points

- [Repository README](../README.md)
- [KiCad project](../hardware/gambos-pcb.kicad_pro)
- [Firmware](../software/)
