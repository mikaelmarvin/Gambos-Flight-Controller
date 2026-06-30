#ifndef BMP384_HPP
#define BMP384_HPP

#include "bus.hpp"

#include <cstdint>

struct BaroSample {
    int32_t pressure_pa{0};
    int16_t temperature_centi_c{0};
};

/** Bosch BMP384 — pressure + temperature on one I2C device. */
class Bmp384 {
  public:
    Bmp384() = default;
    Bmp384(I2cBus &bus, uint8_t addr7);

    bool Init(void);
    /** One blocking read for pressure and temperature. */
    bool ReadPressureTemperature(BaroSample &out);

  private:
    I2cBus *_bus{nullptr};
    uint8_t _addr7{0U};
};

#endif /* BMP384_HPP */
