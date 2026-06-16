#include "spi_dma_blocking.hpp"

#include "semphr.h"
#include "task.h"

namespace {

StaticSemaphore_t spi_dma_semaphore_buffer;
SemaphoreHandle_t spi_dma_semaphore = nullptr;
SPI_HandleTypeDef *spi_dma_owner = nullptr;

void SignalSpiDmaCompleteFromIsr(void) {
    if (spi_dma_semaphore == nullptr) {
        return;
    }
    BaseType_t hpw = pdFALSE;
    (void)xSemaphoreGiveFromISR(spi_dma_semaphore, &hpw);
    portYIELD_FROM_ISR(hpw);
}

void SpiDmaComplete(SPI_HandleTypeDef *hspi) {
    if (spi_dma_owner != nullptr && hspi == spi_dma_owner) {
        SignalSpiDmaCompleteFromIsr();
    }
}

} // namespace

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    SpiDmaComplete(hspi);
}

extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    SpiDmaComplete(hspi);
}

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    SpiDmaComplete(hspi);
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    SpiDmaComplete(hspi);
}

bool SpiDmaBlockingEnsureSemaphore(void) {
    if (spi_dma_semaphore == nullptr) {
        spi_dma_semaphore =
            xSemaphoreCreateBinaryStatic(&spi_dma_semaphore_buffer);
    }
    return spi_dma_semaphore != nullptr;
}

bool SpiDmaBlockingBegin(SPI_HandleTypeDef *spi) {
    if ((spi == nullptr) || !SpiDmaBlockingEnsureSemaphore()) {
        return false;
    }

    while (xSemaphoreTake(spi_dma_semaphore, 0) == pdTRUE) {}

    spi_dma_owner = spi;
    return true;
}

bool SpiDmaBlockingWait(TickType_t timeout_ms) {
    if (spi_dma_semaphore == nullptr) {
        spi_dma_owner = nullptr;
        return false;
    }

    const bool completed =
        xSemaphoreTake(spi_dma_semaphore, timeout_ms) == pdTRUE;
    spi_dma_owner = nullptr;
    return completed;
}

void SpiDmaBlockingAbort(void) { spi_dma_owner = nullptr; }
