# Introduction

**The Gambos Flight Controller** is an STM32F4-based board designed from scratch to learn flight dynamics and embedded systems. An **STM32F446RET6** runs state estimation and control, reading a gyro, accelerometer, magnetometer, and barometer over **I2C**.

## Hardware features and design choices

- **Processing core:** **STM32F446RET6** — high clock speed and FPU for real-time control loops.
- **Dual data storage:**
  - **MicroSD slot** — longer flight logs and convenient data export.
  - **External flash** — fast in-flight logging and configuration storage.
- **Actuation:** Five **PWM** outputs for standard hobby servos and flight surfaces.
- **Integrated power rail:** Power from the **ESC** BEC (5 V) feeds the high-current servo rail and a **3.3 V LDO** for the MCU and sensors.
- **Communication and debugging:**
  - **nRF24L01+** for telemetry and ground-station link.
  - **SWD and UART** headers for firmware flashing and development logs.

## Related documentation

- [System architecture](architecture.md)
- [Repository README](../../README.md)
