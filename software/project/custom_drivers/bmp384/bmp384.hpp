#ifndef BMP384_HPP
#define BMP384_HPP

#include "stm32f4xx_hal.h"

#include <cstdint>

struct BaroSample {
    int32_t pressure_pa{0};
    int16_t temperature_centi_c{0};
};

/** Bosch BMP384 — pressure + temperature on one I2C device. */
class Bmp384 {
  public:
    Bmp384() = default;

    bool Init(void);
    /** One blocking read for pressure and temperature. */
    bool ReadPressureTemperature(BaroSample &out);

  private:
};

#endif /* BMP384_HPP */
