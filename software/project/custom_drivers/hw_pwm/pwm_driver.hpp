#ifndef PWM_DRIVER_HPP
#define PWM_DRIVER_HPP

#include <cstdint>

class PwmDriver {
  public:
    PwmDriver() = default;
    ~PwmDriver() = default;

    bool Initialize(void);
};

#endif /* PWM_DRIVER_HPP */
