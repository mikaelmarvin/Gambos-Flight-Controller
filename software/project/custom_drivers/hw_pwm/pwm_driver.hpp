#ifndef PWM_DRIVER_HPP
#define PWM_DRIVER_HPP

#include "tim.h"

#include <cstdint>

class PwmDriver {
  public:
    PwmDriver() = delete;
    PwmDriver(TIM_HandleTypeDef *htim, uint32_t channel);
    ~PwmDriver() = default;

    bool Start(void);
    bool Stop(void);
    bool SetDutyCycle(uint32_t duty_cycle);
    // Active pulse width (high time) in milliseconds.
    bool SetPulse(float pulse_ms);

  private:
    TIM_HandleTypeDef *_htim{nullptr};
    const uint32_t _channel{0U};
    uint32_t _duty_cycle{0U};
};

#endif /* PWM_DRIVER_HPP */
