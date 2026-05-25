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

    bool Init(I2C_HandleTypeDef *i2c);
    /** One blocking read for pressure and temperature. */
    bool ReadPressureTemperature(BaroSample &out);

  private:
    I2C_HandleTypeDef *_i2c{nullptr};
};

#endif /* BMP384_HPP */
