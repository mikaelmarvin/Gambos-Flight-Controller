#ifndef IIS2MDCTR_HPP
#define IIS2MDCTR_HPP

#include "bus.hpp"

#include <cstdint>

struct MagSample {
    int16_t x{0};
    int16_t y{0};
    int16_t z{0};
};

/** STMicro IIS2MDCTR magnetometer. */
class Iis2mdctr {
  public:
    Iis2mdctr() = default;
    Iis2mdctr(I2cBus &bus, uint8_t addr7);

    bool Init(void);
    bool ReadSample(MagSample &out);

  private:
    I2cBus *_bus{nullptr};
    uint8_t _addr7{0U};
};

#endif /* IIS2MDCTR_HPP */
