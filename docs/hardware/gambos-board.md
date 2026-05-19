

# Gambos flight controller

Custom **STM32F446** flight-controller PCB — schematic through layout, manufacture, assembly, and firmware bring-up. The project exists to learn **embedded systems** end to end: power design, mixed-signal layout, peripheral drivers, RTOS structure, and validation on real hardware.

## At a glance


|               |                                                   |
| ------------- | ------------------------------------------------- |
| **MCU**       | STM32F446RET6 (Cortex-M4 + FPU)                   |
| **Sensors**   | 6-axis IMU, magnetometer, barometer               |
| **Storage**   | External SPI flash + microSD                      |
| **Actuation** | 5× hobby servo PWM, ESC motor PWM                 |
| **Wireless**  | nRF24L01+ telemetry / command link                |
| **Debug**     | SWD + UART                                        |
| **PCB**       | 4-layer, 75 × 50 mm — KiCad, manufactured v1.0    |
| **Firmware**  | CMake, FreeRTOS — devkit and custom-board targets |
| **Status**    | v1.0 built; staged bring-up in progress           |


## Bring-up





*Add a photo of your real board on the bench (power, SWD, scope) at `docs/assets/bringup-setup.jpg` and update the image path above — that is the strongest 30-second signal for reviewers.*

## What this demonstrates

- Full custom board in KiCad (not a dev-kit stack)
- Subsystem-oriented design (power, sensing, storage, RF, actuation)
- Portfolio-grade documentation with schematics and layout figures
- Reproducible firmware workflow (Dev Container, CMake, FreeRTOS)

## Documentation

Read in this order:

1. **Gambos board** (this page)
2. [System architecture](architecture.md)
3. [Physical design](physical-design.md)
4. [Power](power.md)
5. [Storage](storage.md)
6. [Sensing](sensing.md)
7. [User interface](user-interface.md)
8. [Future improvements](future-improvements.md)

## Repository

- KiCad: `[hardware/gambos-pcb.kicad_pro](../../hardware/gambos-pcb.kicad_pro)`
- Firmware: `[software/](../../software/)`
- Hub: `[docs/index.md](../index.md)`

---

**Next:** [System architecture →](architecture.md)

[Documentation index](../index.md)