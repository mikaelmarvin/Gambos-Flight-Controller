# Roadmap

Open firmware and documentation work. Hardware work (v1.0 bring-up and next PCB revisions) is tracked in the [hardware roadmap](https://github.com/mikaelmarvin/gambos_hardware/blob/main/docs/roadmap.md).

## Firmware

- Complete Gambos PCB peripheral bring-up on `gambos-pcb` preset (I2C sensors, SPI flash, SD)
- RTOS task layout, drivers, and messaging (see [Software architecture](software-architecture.md))
- Sensor fusion and control loops for fixed-wing flight
- In-flight logging on external flash; post-flight copy to SD
- Telemetry protocol over nRF24L01+

## Documentation

- Software architecture page: layer diagram, tasks, drivers structure

---

[Repository README](../../README.md) · [Documentation index](../index.md)
