#ifndef ACTUATOR_HANDLER_HPP
#define ACTUATOR_HANDLER_HPP

#include "pwm_driver.hpp"

class ActuatorHandler {
  public:
    ActuatorHandler();

    bool Initialize(void);
    void Start(void);
    bool SetServo1Angle(uint32_t angle_deg);
    bool SetServo3Angle(uint32_t angle_deg);
    bool SetServo4Angle(uint32_t angle_deg);
    bool SetServo5Angle(uint32_t angle_deg);

  private:
    // TIM3 is shared by motor + S3/S4/S5; Stop() on one channel only
    // stops the counter when all TIM3 channels are stopped.
    PwmDriver _motor;
    PwmDriver _servo1;
    // Servo 2 is unavailable in the v1.0 hw version
    PwmDriver _servo3;
    PwmDriver _servo4;
    PwmDriver _servo5;
};

#endif /* ACTUATOR_HANDLER_HPP */
