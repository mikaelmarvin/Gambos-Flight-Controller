# Future improvements

Planned work for hardware, firmware, and documentation. Update this page as items complete.

## Hardware

- [ ] v1.0 assembly and staged bring-up (rails, SWD, clocks, I2C sensor scan)
- [ ] Power-section simulation and measured waveforms (PMOS switch, soft-start, LDO load step)
- [ ] Validation matrix with photos and scope captures in `docs/hardware/` (or linked from bring-up notes)
- [ ] v1.1 fixes from bring-up (if any)

## Firmware

- [ ] Complete custom-board peripheral bring-up (I2C sensors, SPI flash, SD, SPI2 RF)
- [ ] Sensor fusion and control loops for fixed-wing flight
- [ ] In-flight logging on external flash; post-flight copy to SD
- [ ] Telemetry protocol over nRF24L01+

## Documentation

- [ ] Assembly, bring-up, and test procedure pages
- [ ] Software architecture page (FreeRTOS tasks, drivers, messaging)
- [ ] BOM with manufacturer part numbers in repo or linked spreadsheet
