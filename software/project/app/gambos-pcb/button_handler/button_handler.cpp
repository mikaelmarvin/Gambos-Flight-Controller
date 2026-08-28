/**
 * @file button_handler.cpp
 * @brief USR_BTN (PC0): EXTI → HAL_GPIO_EXTI_Callback gives binary
 * semaphore; task takes semaphore, reads pin, publishes (not in ISR).
 */

#include "button_handler.hpp"
#include "common.hpp"
#include "delayable_handler/delayable_work.hpp"
#include "log.hpp"
#include "main.h"
#include "messaging/messaging.hpp"

#include "stm32f4xx_hal_gpio.h"
#include "task.h"

namespace {

constexpr uint32_t kButtonHandlerTaskStackSize =
    768U; /* printf in task + publish → subscriber LOG */
constexpr uint32_t kButtonHandlerTaskPriority =
    (tskIDLE_PRIORITY + 2U);
constexpr uint32_t kButtonHandlerTaskDelay = 20U;

} // namespace

bool ButtonHandler::Initialize(void) {
    _instance = this;
    return true;
}

void ButtonHandler::Start(void) {
    configASSERT(xTaskCreate(&ButtonHandler::TaskFunction,
                             "button_handler",
                             kButtonHandlerTaskStackSize,
                             this,
                             kButtonHandlerTaskPriority,
                             NULL) == pdPASS);
}

void ButtonHandler::TaskFunction(void *pvParameters) {
    ButtonHandler *const self =
        static_cast<ButtonHandler *>(pvParameters);

    // The task handle is needed to notify the task from the ISR.
    self->_task_handle = xTaskGetCurrentTaskHandle();

    while (true) {

        uint32_t status = 0;
        xTaskNotifyWait(0, UINT32_MAX, &status, portMAX_DELAY);

        const bool pressed =
            (HAL_GPIO_ReadPin(USR_BTN_GPIO_Port, USR_BTN_Pin) !=
             GPIO_PIN_SET);

        topics::ButtonInfo topic{};
        topic.button_id = static_cast<uint8_t>(ButtonId::USER_BUTTON);
        topic.button_state = static_cast<uint8_t>(
            pressed ? ButtonState::PRESSED : ButtonState::RELEASED);

        LOG("User button pressed=%u\r\n", (unsigned)pressed);
        Messaging::Publish<topics::ButtonInfo>(topic);

        vTaskDelay(pdMS_TO_TICKS(kButtonHandlerTaskDelay));
    }
}

void ButtonHandler::CallbackFromISR(void) {
    if ((_instance == nullptr) ||
        (_instance->_task_handle == nullptr)) {
        return;
    }

    BaseType_t higher_priority_woken = pdFALSE;
    (void)xTaskNotifyFromISR(_instance->_task_handle,
                             0U,
                             eNoAction,
                             &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}
