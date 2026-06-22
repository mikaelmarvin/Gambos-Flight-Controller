/**
 * @file tx_handler.cpp
 * @brief nRF24 / TX path — FreeRTOS task (native API).
 */

#include "tx_handler.hpp"
#include "board_buses.hpp"
#include "log.hpp"
#include "main.h"
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

    if (!_nrf24l01p.RegisterCePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin) ||
        !_nrf24l01p.RegisterCsnPin(NRF24_CS_GPIO_Port,
                                   NRF24_CS_Pin)) {
        return false;
    }

    return true;
}

void TxHandler::Start(void) {
    if (!Spi2().IsInitialized()) {
        LOG("tx_handler: SPI2 bus not initialized\r\n");
    } else if (!_nrf24l01p.Init(Spi2().Spi(),
                                Nrf24l01p::PrimaryRole::Ptx,
                                &Spi2())) {
        LOG("tx_handler: nRF24 init failed (task still runs)\r\n");
    }

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

    while (true) {
        LOG("Hello from tx_handler\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}
