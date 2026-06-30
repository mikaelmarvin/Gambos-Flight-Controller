#ifndef LSM6DSVTR_HPP
#define LSM6DSVTR_HPP

#include "bus.hpp"

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
    Lsm6dsvtr(I2cBus &bus, uint8_t addr7);

    bool Init(void);
    /** One blocking I2C transaction for both accel and gyro raw
     * samples. */
    bool ReadAccelGyro(AccelSample &accel, GyroSample &gyro);

  private:
    I2cBus *_bus{nullptr};
    uint8_t _addr7{0U};
};

#endif /* LSM6DSVTR_HPP */
