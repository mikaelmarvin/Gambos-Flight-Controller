#ifndef LSM6DSVTR_HPP
#define LSM6DSVTR_HPP

#include "stm32f4xx_hal.h"

#include <cstdint>

struct AccelSample {
    int16_t x{0};
    int16_t y{0};
    int16_t z{0};
};

struct GyroSample {
    int16_t x{0};
    int16_t y{0};
    int16_t z{0};
};

/**
 * STMicro LSM6DSVTR — accelerometer + gyroscope on one I2C device.
 */
class Lsm6dsvtr {
  public:
    Lsm6dsvtr() = default;

    bool Init(I2C_HandleTypeDef *i2c);
    /** One blocking I2C transaction for both accel and gyro raw
     * samples. */
    bool ReadAccelGyro(AccelSample &accel, GyroSample &gyro);

  private:
    I2C_HandleTypeDef *_i2c{nullptr};
};

#endif /* LSM6DSVTR_HPP */
