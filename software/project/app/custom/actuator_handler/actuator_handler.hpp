#ifndef ACTUATOR_HANDLER_HPP
#define ACTUATOR_HANDLER_HPP

#include "pwm_driver.hpp"

class ActuatorHandler {
  public:
    explicit ActuatorHandler();

    bool Initialize(void);
    void Start(void);

  private:
    PwmDevice &_motor;
    PwmDevice &_servo1;
    // Servo 2 is unavailable in the v1.0 hw version
    PwmDevice &_servo3;
    PwmDevice &_servo4;
    PwmDevice &_servo5;
};

#endif /* ACTUATOR_HANDLER_HPP */