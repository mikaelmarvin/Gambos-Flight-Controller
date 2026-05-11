# Hardware Overview

## Purpose

The Gambos hardware is a custom STM32F446-based flight controller board designed as a portfolio embedded project.
It is intended to support end-to-end engineering workflow: schematic design, PCB layout, manufacturing, assembly, and staged bring-up.

## Current Revision Status

- PCB revision: Rev A (first manufactured version)
- Manufacturing: completed
- Assembly: pending
- Bring-up: pending (planned staged validation)

## Main Hardware Blocks

- **MCU core**: STM32F446 microcontroller and supporting clock/reset/debug circuitry
- **Power subsystem**: input regulation and local rails for digital/analog domains
- **Sensors and storage**: onboard sensor interfaces and non-volatile memory support
- **Connectivity**: SWD/debug header and external I/O connectors for peripherals/integration

## Design Goals

- Create a realistic custom embedded board from scratch
- Keep subsystem boundaries clear for easier debugging and iteration
- Support reliable firmware development with repeatable bring-up checkpoints
- Produce documentation and validation artifacts suitable for portfolio review

## Repository Pointers

- KiCad project and manufacturing files: `hardware/`
- Assembly planning: `docs/hardware/assembly-plan.md`
- Bring-up procedure: `docs/hardware/bringup-plan.md`
- Validation checklist: `docs/hardware/test-matrix.md`

## Next Steps

1. Assemble Rev A boards and perform visual/electrical inspection
2. Validate power rails and debug access (SWD/clock/reset)
3. Run staged peripheral checks and log results
4. Capture photos, measurements, and known issues for Rev B planning