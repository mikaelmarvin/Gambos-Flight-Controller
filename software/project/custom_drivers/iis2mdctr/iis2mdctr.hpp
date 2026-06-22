#ifndef IIS2MDCTR_HPP
#define IIS2MDCTR_HPP

#include "stm32f4xx_hal.h"

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

    bool Init(void);
    bool ReadSample(MagSample &out);

  private:
};

#endif /* IIS2MDCTR_HPP */
