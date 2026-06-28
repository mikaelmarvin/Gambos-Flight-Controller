#include "bus.hpp"

#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_i2c.h"

#include "semphr.h"
#include "task.h"

namespace {

uint16_t ToHalDevAddress(const uint16_t dev_addr_7bit) {
    return static_cast<uint16_t>(dev_addr_7bit << 1U);
}

void SignalSpiDmaComplete(SPI_HandleTypeDef *hspi) {
    Spi1().SignalIfHandle(hspi);
    Spi2().SignalIfHandle(hspi);
}

void SignalI2cDmaComplete(I2C_HandleTypeDef *hi2c) {
    I2c1().SignalIfHandle(hi2c);
}

} // namespace

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiDmaComplete(hspi);
}

extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiDmaComplete(hspi);
}

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiDmaComplete(hspi);
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiDmaComplete(hspi);
}

extern "C" void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cDmaComplete(hi2c);
}

extern "C" void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cDmaComplete(hi2c);
}

extern "C" void HAL_I2C_MasterTxCpltCallback(
    I2C_HandleTypeDef *hi2c) {
    SignalI2cDmaComplete(hi2c);
}

extern "C" void HAL_I2C_MasterRxCpltCallback(
    I2C_HandleTypeDef *hi2c) {
    SignalI2cDmaComplete(hi2c);
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cDmaComplete(hi2c);
}

CsAssert::CsAssert(const CsPin cs) : _cs(cs) {
    if (_cs.port != nullptr) {
        HAL_GPIO_WritePin(_cs.port, _cs.pin, GPIO_PIN_RESET);
    }
}

CsAssert::~CsAssert() {
    if (_cs.port != nullptr) {
        HAL_GPIO_WritePin(_cs.port, _cs.pin, GPIO_PIN_SET);
    }
}

bool SpiBus::EnsureDmaSemaphore(void) {
    if (_dma_sem == nullptr) {
        _dma_sem = xSemaphoreCreateBinaryStatic(&_dma_sem_buffer);
    }
    return _dma_sem != nullptr;
}

bool SpiBus::EnsureBusMutex(void) {
    if (_bus_mutex == nullptr) {
        _bus_mutex = xSemaphoreCreateMutexStatic(&_bus_mutex_buffer);
    }
    return _bus_mutex != nullptr;
}

bool SpiBus::Init(SPI_HandleTypeDef *hspi) {
    if (hspi == nullptr) {
        return false;
    }

    if (IsInitialized() && (_hspi == hspi)) {
        return true;
    }

    _hspi = hspi;

    if (!EnsureBusMutex() || !EnsureDmaSemaphore()) {
        _hspi = nullptr;
        return false;
    }

    return true;
}

bool SpiBus::IsInitialized(void) const { return _hspi != nullptr; }

bool SpiBus::BeginDma(void) {
    if (!EnsureDmaSemaphore()) {
        return false;
    }

    // Drain the binary semaphore so "Take" can’t succeed on a
    // leftover "Give" from a previous transfer.
    while (xSemaphoreTake(_dma_sem, 0) == pdTRUE) {}

    return true;
}

bool SpiBus::WaitDma(const TickType_t timeout) {
    if (_dma_sem == nullptr) {
        return false;
    }

    return xSemaphoreTake(_dma_sem, timeout) == pdTRUE;
}

void SpiBus::SignalFromIsr(void) {
    if (_dma_sem == nullptr) {
        return;
    }

    BaseType_t hpw = pdFALSE;
    (void)xSemaphoreGiveFromISR(_dma_sem, &hpw);
    portYIELD_FROM_ISR(hpw);
}

void SpiBus::SignalIfHandle(SPI_HandleTypeDef *hspi) {
    if ((_hspi != nullptr) && (_hspi == hspi)) {
        SignalFromIsr();
    }
}

bool SpiBus::TransmitDma(const uint8_t *tx,
                         const uint16_t len,
                         const TickType_t timeout) {
    if (!IsInitialized() || (tx == nullptr) || (len == 0U) ||
        !EnsureBusMutex()) {
        return false;
    }

    // Lock the bus mutex to prevent multiple tasks from accessing the
    // bus at the same time.
    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started =
            HAL_SPI_Transmit_DMA(_hspi, tx, len);
        if (started == HAL_OK) {
            ok = WaitDma(timeout);
        }
    }

    // Release the bus mutex to allow other tasks to access the bus.
    (void)xSemaphoreGive(_bus_mutex);
    return ok;
}

bool SpiBus::ReceiveDma(uint8_t *rx,
                        const uint16_t len,
                        const TickType_t timeout) {
    if (!IsInitialized() || (rx == nullptr) || (len == 0U) ||
        !EnsureBusMutex()) {
        return false;
    }

    // Lock the bus mutex to prevent multiple tasks from accessing the
    // bus at the same time.
    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started =
            HAL_SPI_Receive_DMA(_hspi, rx, len);
        if (started == HAL_OK) {
            ok = WaitDma(timeout);
        }
    }

    // Release the bus mutex to allow other tasks to access the bus.
    (void)xSemaphoreGive(_bus_mutex);
    return ok;
}

bool SpiBus::TransmitReceiveDma(const uint8_t *tx,
                                uint8_t *rx,
                                const uint16_t len,
                                const TickType_t timeout) {
    if (!IsInitialized() || (tx == nullptr) || (rx == nullptr) ||
        (len == 0U) || !EnsureBusMutex()) {
        return false;
    }

    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started =
            HAL_SPI_TransmitReceive_DMA(_hspi, tx, rx, len);
        if (started == HAL_OK) {
            ok = WaitDma(timeout);
        }
    }

    (void)xSemaphoreGive(_bus_mutex);
    return ok;
}

bool I2cBus::EnsureDmaSemaphore(void) {
    if (_dma_sem == nullptr) {
        _dma_sem = xSemaphoreCreateBinaryStatic(&_dma_sem_buffer);
    }
    return _dma_sem != nullptr;
}

bool I2cBus::EnsureBusMutex(void) {
    if (_bus_mutex == nullptr) {
        _bus_mutex = xSemaphoreCreateMutexStatic(&_bus_mutex_buffer);
    }
    return _bus_mutex != nullptr;
}

bool I2cBus::Init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == nullptr) {
        return false;
    }

    if (IsInitialized() && (_hi2c == hi2c)) {
        return true;
    }

    _hi2c = hi2c;

    if (!EnsureBusMutex() || !EnsureDmaSemaphore()) {
        _hi2c = nullptr;
        return false;
    }

    return true;
}

bool I2cBus::IsInitialized(void) const { return _hi2c != nullptr; }

bool I2cBus::BeginDma(void) {
    if (!EnsureDmaSemaphore()) {
        return false;
    }

    while (xSemaphoreTake(_dma_sem, 0) == pdTRUE) {}

    return true;
}

bool I2cBus::WaitDma(const TickType_t timeout) {
    if (_dma_sem == nullptr) {
        return false;
    }

    return xSemaphoreTake(_dma_sem, timeout) == pdTRUE;
}

void I2cBus::SignalFromIsr(void) {
    if (_dma_sem == nullptr) {
        return;
    }

    BaseType_t hpw = pdFALSE;
    (void)xSemaphoreGiveFromISR(_dma_sem, &hpw);
    portYIELD_FROM_ISR(hpw);
}

void I2cBus::SignalIfHandle(I2C_HandleTypeDef *hi2c) {
    if ((_hi2c != nullptr) && (_hi2c == hi2c)) {
        SignalFromIsr();
    }
}

bool I2cBus::MemRead(const uint16_t dev_addr_7bit,
                     const uint16_t reg,
                     uint8_t *data,
                     const uint16_t len,
                     const TickType_t timeout) {
    if (!IsInitialized() || (data == nullptr) || (len == 0U) ||
        !EnsureBusMutex()) {
        return false;
    }

    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started =
            HAL_I2C_Mem_Read_DMA(_hi2c,
                                 ToHalDevAddress(dev_addr_7bit),
                                 reg,
                                 I2C_MEMADD_SIZE_8BIT,
                                 data,
                                 len);
        if (started == HAL_OK) {
            ok = WaitDma(timeout);
        }
    }

    (void)xSemaphoreGive(_bus_mutex);
    return ok;
}

bool I2cBus::MemWrite(const uint16_t dev_addr_7bit,
                      const uint16_t reg,
                      const uint8_t *data,
                      const uint16_t len,
                      const TickType_t timeout) {
    if (!IsInitialized() || (data == nullptr) || (len == 0U) ||
        !EnsureBusMutex()) {
        return false;
    }

    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started =
            HAL_I2C_Mem_Write_DMA(_hi2c,
                                  ToHalDevAddress(dev_addr_7bit),
                                  reg,
                                  I2C_MEMADD_SIZE_8BIT,
                                  const_cast<uint8_t *>(data),
                                  len);
        if (started == HAL_OK) {
            ok = WaitDma(timeout);
        }
    }

    (void)xSemaphoreGive(_bus_mutex);
    return ok;
}

I2cBus &I2c1(void) {
    static I2cBus bus;
    return bus;
}

SpiBus &Spi1(void) {
    static SpiBus bus;
    return bus;
}

SpiBus &Spi2(void) {
    static SpiBus bus;
    return bus;
}
