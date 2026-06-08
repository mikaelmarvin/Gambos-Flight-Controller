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

#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32f4xx_hal_gpio.h"
#include "task.h"

StaticSemaphore_t ButtonHandler::button_semaphore_buffer;
SemaphoreHandle_t ButtonHandler::button_semaphore;

namespace {

constexpr uint32_t kButtonHandlerTaskStackSize =
    1536U; /* nested printf via Messaging::Publish → subscriber LOG */
constexpr uint32_t kButtonHandlerTaskPriority =
    (tskIDLE_PRIORITY + 2U);
constexpr uint32_t kButtonHandlerTaskDelay = 20U;

} // namespace

bool ButtonHandler::Initialize(void) {
    if ((button_semaphore = xSemaphoreCreateBinaryStatic(
             &button_semaphore_buffer)) == nullptr) {
        return false;
    }

    uint32_t special_number = 12345U;
    _delayed_press_work.Initialize([special_number]() {
        LOG("THIS FUNCTION WAS DELAYED AND THE SPECIAL NUMBER IS "
            "%u\r\n",
            special_number);
    });

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

    while (true) {
        if (xSemaphoreTake(button_semaphore, portMAX_DELAY) !=
            pdTRUE) {
            continue;
        }

        const GPIO_PinState level =
            HAL_GPIO_ReadPin(USR_BTN_GPIO_Port, USR_BTN_Pin);

        // Adjust if the schematic is active-high.
        const uint8_t pressed = (level == GPIO_PIN_RESET) ? 1U : 0U;

        topics::ButtonInfo topic{};
        topic.button_id = static_cast<uint8_t>(ButtonId::USER_BUTTON);
        topic.button_state = pressed;

        LOG("B1 pressed=%u\r\n", (unsigned)pressed);
        (void)Messaging::Publish<topics::ButtonInfo>(topic);

        self->_delayed_press_work.ScheduleOnce(1000U);

        vTaskDelay(pdMS_TO_TICKS(kButtonHandlerTaskDelay));
    }
}

void ButtonHandler::CallbackFromISR(void) {
    if (ButtonHandler::button_semaphore == nullptr) {
        return;
    }

    BaseType_t higher_priority_woken = pdFALSE;
    (void)xSemaphoreGiveFromISR(ButtonHandler::button_semaphore,
                                &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == USR_BTN_Pin) {
        ButtonHandler::CallbackFromISR();
        return;
    }
}
