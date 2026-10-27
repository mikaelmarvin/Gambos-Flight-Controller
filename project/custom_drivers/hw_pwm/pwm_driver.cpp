#include "pwm_driver.hpp"
#include "stm32f4xx_hal_tim.h"

namespace {

constexpr uint32_t kTimCounterClockHz = 90'000'000U;

} // namespace

PwmDriver::PwmDriver(TIM_HandleTypeDef *htim, uint32_t channel)
    : _htim(htim), _channel(channel), _duty_cycle(0U) {}

bool PwmDriver::Start(void) {
    if (_htim == nullptr) {
        return false;
    }

    return HAL_TIM_PWM_Start(_htim, _channel) == HAL_OK;
}

bool PwmDriver::Stop(void) {
    if (_htim == nullptr) {
        return false;
    }

    // Makes CCxE = 0 or stops the whole timer if all channels are
    // disabled
    return HAL_TIM_PWM_Stop(_htim, _channel) == HAL_OK;
}

bool PwmDriver::SetDutyCycle(uint32_t duty_cycle) {
    if (_htim == nullptr) {
        return false;
    }

    if (duty_cycle > 100U) {
        duty_cycle = 100U;
    }

    const uint32_t arr = __HAL_TIM_GET_AUTORELOAD(_htim);
    const uint32_t ccr = (duty_cycle * (arr + 1U)) / 100U;
    __HAL_TIM_SET_COMPARE(_htim, _channel, ccr);
    _duty_cycle = duty_cycle;
    return true;
}

bool PwmDriver::SetPulse(float pulse_ms) {
    if (_htim == nullptr || pulse_ms <= 0.0f) {
        return false;
    }

    const uint32_t psc = _htim->Instance->PSC;
    const uint32_t arr = __HAL_TIM_GET_AUTORELOAD(_htim);
    const float tick_hz = static_cast<float>(kTimCounterClockHz) /
                          static_cast<float>(psc + 1U);
    const float pulse_ticks_f = (pulse_ms / 1000.0f) * tick_hz;

    if (pulse_ticks_f < 1.0f ||
        pulse_ticks_f > static_cast<float>(arr + 1U)) {
        return false;
    }

    const uint32_t ccr = static_cast<uint32_t>(pulse_ticks_f + 0.5f);
    __HAL_TIM_SET_COMPARE(_htim, _channel, ccr);
    _duty_cycle = (ccr * 100U) / (arr + 1U);
    return true;
}