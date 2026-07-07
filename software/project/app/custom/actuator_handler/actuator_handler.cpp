#include "actuator_handler.hpp"

#include "log.hpp"
#include "tim.h"

namespace {

float ServoAngleToPulseMs(uint32_t angle_deg) {
    if (angle_deg > 180U) {
        angle_deg = 180U;
    }

    return 1.0f + (static_cast<float>(angle_deg) * 2.0f / 180.0f);
}

} // namespace

ActuatorHandler::ActuatorHandler()
    : _motor(&htim3, TIM_CHANNEL_1), _servo1(&htim2, TIM_CHANNEL_1),
      _servo3(&htim3, TIM_CHANNEL_4), _servo4(&htim3, TIM_CHANNEL_3),
      _servo5(&htim3, TIM_CHANNEL_2) {}

bool ActuatorHandler::Initialize(void) {
    const float servo_center_pulse_ms = ServoAngleToPulseMs(90U);

    return _motor.SetDutyCycle(0U) &&
           _servo1.SetPulse(servo_center_pulse_ms) &&
           _servo3.SetPulse(servo_center_pulse_ms) &&
           _servo4.SetPulse(servo_center_pulse_ms) &&
           _servo5.SetPulse(servo_center_pulse_ms);
}

void ActuatorHandler::Start(void) {
    const bool motor_ok = _motor.Start();
    const bool servo1_ok = _servo1.Start();
    const bool servo3_ok = _servo3.Start();
    const bool servo4_ok = _servo4.Start();
    const bool servo5_ok = _servo5.Start();

    if (!motor_ok || !servo1_ok || !servo3_ok || !servo4_ok ||
        !servo5_ok) {
        LOG("ERROR: actuator_handler PWM start failed "
            "(motor=%u s1=%u s3=%u s4=%u s5=%u)\r\n",
            motor_ok,
            servo1_ok,
            servo3_ok,
            servo4_ok,
            servo5_ok);
    }
}

bool ActuatorHandler::SetServo1Angle(uint32_t angle_deg) {
    return _servo1.SetPulse(ServoAngleToPulseMs(angle_deg));
}

bool ActuatorHandler::SetServo3Angle(uint32_t angle_deg) {
    return _servo3.SetPulse(ServoAngleToPulseMs(angle_deg));
}

bool ActuatorHandler::SetServo4Angle(uint32_t angle_deg) {
    return _servo4.SetPulse(ServoAngleToPulseMs(angle_deg));
}

bool ActuatorHandler::SetServo5Angle(uint32_t angle_deg) {
    return _servo5.SetPulse(ServoAngleToPulseMs(angle_deg));
}
