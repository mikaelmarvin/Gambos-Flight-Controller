# Hardware overview

## Purpose

The Gambos hardware is a custom **STM32F446** flight controller designed as a portfolio embedded project. It supports an end-to-end workflow: schematic design, PCB layout, manufacturing, assembly, and staged bring-up.

## Current revision status

| Item | Status |
|------|--------|
| PCB revision | Rev A (first manufactured version) |
| Manufacturing | Complete |
| Assembly | Pending |
| Bring-up | Pending (planned staged validation) |

## Main hardware blocks

- **MCU core** — STM32F446, clock/reset, SWD
- **Power** — ESC BEC input, high-side switch, 3.3 V LDO, servo headers
- **Sensors and storage** — I2C IMU/mag/barometer; SPI flash + microSD
- **User I/O** — nRF24L01+, button, LEDs
- **Actuation** — PWM servos and ESC motor output

## Design goals

- Realistic custom board from scratch, not a module stack
- Clear subsystem boundaries for debug and iteration
- Repeatable firmware bring-up checkpoints
- Documentation suitable for portfolio review

## Detailed documentation

1. [Introduction](introduction.md)
2. [System architecture](architecture.md)
3. [Physical design](physical-design.md)
4. [Power](power.md)
5. [Storage](storage.md)
6. [Sensing](sensing.md)
7. [User interface](user-interface.md)
8. [Future improvements](future-improvements.md)

## Repository pointers

- KiCad project: [`hardware/gambos-pcb.kicad_pro`](../../hardware/gambos-pcb.kicad_pro)
- Firmware: [`software/`](../../software/)
- Documentation hub: [`docs/index.md`](../index.md)
