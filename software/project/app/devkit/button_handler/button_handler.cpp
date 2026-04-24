/**
 * @file button_handler.cpp
 * @brief B1 (PC13): EXTI → HAL_GPIO_EXTI_Callback gives binary
 * semaphore; task takes semaphore, reads pin, publishes (not in ISR).
 */

#include "button_handler/button_handler.hpp"
#include "button_handler/common.hpp"
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

constexpr uint32_t kButtonHandlerTaskStackSize = 512U;
constexpr uint32_t kButtonHandlerTaskPriority =
    (tskIDLE_PRIORITY + 2U);
constexpr uint32_t kButtonHandlerTaskDelay = 20U;

} // namespace

bool ButtonHandler::Initialize(void) {
    button_semaphore =
        xSemaphoreCreateBinaryStatic(&button_semaphore_buffer);
    return button_semaphore != nullptr;
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
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(button_semaphore, portMAX_DELAY) !=
            pdTRUE) {
            continue;
        }

        const GPIO_PinState level =
            HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

        // Active low: pressed when line is low.
        const uint8_t pressed = (level == GPIO_PIN_RESET) ? 1U : 0U;

        topics::ButtonInfo topic{};
        topic.button_id = static_cast<uint8_t>(ButtonId::USER_BUTTON);
        topic.button_state = pressed;

        (void)Messaging::Publish<topics::ButtonInfo>(topic);
        LOG("B1 pressed=%u\r\n", (unsigned)pressed);

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
    if (GPIO_Pin == B1_Pin) {
        ButtonHandler::CallbackFromISR();
        return;
    }
}
