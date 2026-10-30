# Gambos flight controller

Custom **STM32F446** flight-controller PCB and firmware — schematic through layout, manufacture, assembly, bring-up, application and driver development.

## At a glance


|               |                                                                        |
| ---------------| ------------------------------------------------------------------------|
| **MCU**       | STM32F446RET6 (Cortex-M4 + FPU)                                        |
| **Sensors**   | Accelerometer, gyroscope, magnetometer, barometer, temperature         |
| **Storage**   | External flash + microSD                                               |
| **Actuation** | 5× hobby servo PWM, ESC motor PWM                                      |
| **Wireless**  | nRF24L01+ telemetry / command link                                     |
| **Debug**     | SWD + UART (SEGGER J-Link)                                             |
| **PCB**       | 4-layer, 75 × 50 mm — KiCad, manufactured **v1.0**                     |
| **Firmware**  | CMake, FreeRTOS — `gambos-pcb` target                                  |
| **Status**    | v1.0 built; hardware bring-up in progress; flight software in progress |
| **Schematic** | [Gambos PCB schematic (PDF)](https://github.com/mikaelmarvin/gambos_hardware/blob/main/docs/gambos-pcb.pdf) — KiCad export, v1.0 |

## Schematic (v1.0)

The full board schematic is published as a PDF:

**[Gambos PCB schematic (PDF)](https://github.com/mikaelmarvin/gambos_hardware/blob/main/docs/gambos-pcb.pdf)**

The KiCad design, the schematic PDF and the hardware documentation live in a separate repository, [gambos_hardware](https://github.com/mikaelmarvin/gambos_hardware).

## Render and Bring-up setup

<p align="center">
  <table cellspacing="0" cellpadding="12">
    <tr>
      <td align="center" valign="top">
        <img src="docs/assets/3d-pcb-render.png" alt="3D PCB render" style="height:240px; width:auto; max-width:none;">
        <br>
        <sub>3D PCB render</sub>
      </td>
      <td align="center" valign="top">
        <img src="docs/assets/bringup-setup.jpg" alt="Bring-up setup on the bench" style="height:240px; width:auto; max-width:none;">
        <br>
        <sub>Testing the power section on the bench</sub>
      </td>
    </tr>
  </table>
</p>

Render of v1.0 layout in KiCad (left) and early bench bring-up (right), started with the power section before moving to testing other peripherals.

## Hardware block diagram

<p align="center">
  <img src="docs/assets/block-diagram.png" alt="System block diagram" style="height:312px; width:auto; max-width:none;">
  <br>
  <sub>Hardware block diagram</sub>
</p>

Buses, actuation, and interfaces are described in [Hardware architecture](https://github.com/mikaelmarvin/gambos_hardware/blob/main/docs/hardware-architecture.md).

## Software architecture

*(diagram pending)* — layer stack, drivers, RTOS tasks, and flight logic are described in [Software architecture](docs/software/software-architecture.md).

## Documentation

Full index: **[docs/index.md](docs/index.md)**

**Hardware:** documentation for the v1.0 board lives in [gambos_hardware](https://github.com/mikaelmarvin/gambos_hardware#documentation-v10) — architecture, physical design, power, storage, sensing, user interface, and the hardware roadmap.

**Software:** [docs/software/software-architecture.md](docs/software/software-architecture.md) → [project/README.md](project/README.md) · [Firmware roadmap](docs/software/roadmap.md)

## Repository layout


| Directory                | Purpose                                         |
| ------------------------ | ----------------------------------------------- |
| `[project/](project/)`   | STM32 firmware (CMake, FreeRTOS)                |
| `[docs/](docs/)`         | Software documentation, figures                 |
