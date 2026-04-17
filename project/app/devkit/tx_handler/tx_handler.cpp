/**
 * @file tx_handler.cpp
 * @brief nRF24 / TX path — FreeRTOS task (native API).
 */

#include "tx_handler/tx_handler.hpp"
#include "log.hpp"
#include "messaging/messaging.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kTxHandlerTaskStackSize = 256U;
constexpr uint32_t kTxHandlerTaskPriority = (tskIDLE_PRIORITY + 1U);

} // namespace

bool TxHandler::Initialize(void) {
    Messaging::Subscribe<topics::ButtonInfo>(
        [](const topics::ButtonInfo &button_info) {
            LOG("Button pressed: %u\r\n", button_info.button_state);
        });

    nrf24l01p_.Init(Nrf24l01p::PrimaryRole::Ptx);
    return true;
}

void TxHandler::Start(void) {
    configASSERT(xTaskCreate(&TxHandler::TaskFunction,
                             "tx_handler",
                             kTxHandlerTaskStackSize,
                             this,
                             kTxHandlerTaskPriority,
                             NULL) == pdPASS);
}

void TxHandler::TaskFunction(void *pvParameters) {
    TxHandler *const self = static_cast<TxHandler *>(pvParameters);
    (void)self;

    for (;;) {
        LOG("Hello from tx_handler\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}
