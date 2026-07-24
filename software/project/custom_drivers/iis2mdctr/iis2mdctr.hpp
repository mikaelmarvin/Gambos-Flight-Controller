#ifndef IIS2MDCTR_HPP
#define IIS2MDCTR_HPP

#include "bus.hpp"

#include <cstdint>

struct MagSample {
    int16_t x{0};
    int16_t y{0};
    int16_t z{0};
};

/**
 * STMicro IIS2MDCTR magnetometer.
 *
 * Init configures continuous high-resolution mode at 10 Hz with BDU
 * and temperature compensation. ReadSample returns false if ZYXDA is
 * not set.
 */
class Iis2mdctr {
  public:
    Iis2mdctr() = delete;
    Iis2mdctr(I2cBus &bus, uint8_t addr7);

    bool Init(void);
    bool ReadSample(MagSample &out);

  private:
    I2cBus *_bus{nullptr};
    const uint8_t _addr7{0U};
};

#endif /* IIS2MDCTR_HPP */
