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
    Messaging::Subscribe<ButtonInfo>(
        [](const ButtonInfo &button_info) {
            (void)button_info;
            LOG("This is from a subscriber!!");
        });

    return true;
}

void TxHandler::Start(void) {
    (void)xTaskCreate(&TxHandler::TaskFunction,
                      "tx_handler",
                      kTxHandlerTaskStackSize,
                      this,
                      kTxHandlerTaskPriority,
                      NULL);
}

void TxHandler::TaskFunction(void *pvParameters) {
    TxHandler *const self = static_cast<TxHandler *>(pvParameters);
    (void)self;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

static TxHandler g_tx_handler;

extern "C" void tx_handler_start(void) {
    (void)g_tx_handler.Initialize();
    g_tx_handler.Start();
}
