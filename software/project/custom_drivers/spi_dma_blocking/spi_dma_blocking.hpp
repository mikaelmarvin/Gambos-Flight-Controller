#ifndef SPI_DMA_BLOCKING_HPP
#define SPI_DMA_BLOCKING_HPP

#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"

bool SpiDmaBlockingEnsureSemaphore(void);
bool SpiDmaBlockingBegin(SPI_HandleTypeDef *spi);
bool SpiDmaBlockingWait(TickType_t timeout_ms);
void SpiDmaBlockingAbort(void);

#endif /* SPI_DMA_BLOCKING_HPP */
