/**
 * @file tx_handler.cpp
 * @brief nRF24 / TX path — FreeRTOS task (native API).
 */

#include "tx_handler.hpp"
#include "log.hpp"
#include "main.h"
#include "messaging/messaging.hpp"
#include "spi.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

namespace {

constexpr uint32_t kTxHandlerTaskStackSize = 256U;
constexpr uint32_t kTxHandlerTaskPriority = (tskIDLE_PRIORITY + 1U);

StaticSemaphore_t spi_semaphore_buffer;
SemaphoreHandle_t spi_semaphore = nullptr;

} // namespace

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    (void)hspi;
    BaseType_t hpw = pdFALSE;
    if (spi_semaphore != nullptr) {
        (void)xSemaphoreGiveFromISR(spi_semaphore, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}

bool TxHandler::Initialize(void) {
    Messaging::Subscribe<topics::ButtonInfo>(
        [](const topics::ButtonInfo &button_info) {
            LOG("Button pressed: %u\r\n", button_info.button_state);
        });

    if (!_nrf24l01p.RegisterCePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin) ||
        !_nrf24l01p.RegisterCsnPin(NRF24_CS_GPIO_Port, NRF24_CS_Pin)) {
        return false;
    }

    spi_semaphore =
        xSemaphoreCreateBinaryStatic(&spi_semaphore_buffer);
    if (spi_semaphore == NULL) {
        return false;
    }

    return _nrf24l01p.Init(&hspi2, Nrf24l01p::PrimaryRole::Ptx);
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
