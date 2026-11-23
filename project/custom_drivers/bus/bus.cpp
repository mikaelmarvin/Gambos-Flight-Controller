#include "bus.hpp"

#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_uart.h"

#include "semphr.h"
#include "task.h"
#include <cstdint>

namespace {

uint16_t ToHalDevAddress(const uint16_t dev_addr_7bit) {
    return static_cast<uint16_t>(dev_addr_7bit << 1U);
}

void SignalSpiDmaComplete(SPI_HandleTypeDef *hspi) {
    Spi1().SignalIfHandle(hspi);
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

uint32_t SpiBus::GetBaudRatePrescaler(void) const {
    if (!IsInitialized()) {
        return 0U;
    }
    return _hspi->Init.BaudRatePrescaler;
}

bool SpiBus::SetBaudRatePrescaler(const uint32_t prescaler,
                                  const TickType_t timeout) {
    if (!IsInitialized() || !IS_SPI_BAUDRATE_PRESCALER(prescaler) ||
        !EnsureBusMutex()) {
        return false;
    }

    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    // BR is only writable while SPE is cleared.
    __HAL_SPI_DISABLE(_hspi);
    MODIFY_REG(_hspi->Instance->CR1, SPI_CR1_BR, prescaler);
    _hspi->Init.BaudRatePrescaler = prescaler;
    __HAL_SPI_ENABLE(_hspi);

    (void)xSemaphoreGive(_bus_mutex);
    return true;
}

bool SpiBus::BeginDma(void) {
    if (!EnsureDmaSemaphore()) {
        return false;
    }

    // Drain the binary semaphore so "Take" can’t succeed on a
    // leftover "Give" from a previous transfer.
    while (xSemaphoreTake(_dma_sem, 0) == pdTRUE) {}

    return true;
}

void SpiBus::DrainRxOverrun(void) {
    if (_hspi == nullptr) {
        return;
    }
    // Full-duplex SPI still shifts RX during TX-only DMA; unread RX
    // sets OVR and breaks the next transfer.
    while (__HAL_SPI_GET_FLAG(_hspi, SPI_FLAG_RXNE) != RESET) {
        (void)_hspi->Instance->DR;
    }
    __HAL_SPI_CLEAR_OVRFLAG(_hspi);
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
    DrainRxOverrun();

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
            ok = WaitDma(timeout) &&
                 (HAL_I2C_GetState(_hi2c) == HAL_I2C_STATE_READY) &&
                 (_hi2c->ErrorCode == HAL_I2C_ERROR_NONE);
        }
    }

    if (!ok && (_hi2c->ErrorCode != HAL_I2C_ERROR_NONE)) {
        _hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
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
            ok = WaitDma(timeout) &&
                 (HAL_I2C_GetState(_hi2c) == HAL_I2C_STATE_READY) &&
                 (_hi2c->ErrorCode == HAL_I2C_ERROR_NONE);
        }
    }

    if (!ok && (_hi2c->ErrorCode != HAL_I2C_ERROR_NONE)) {
        _hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    }

    (void)xSemaphoreGive(_bus_mutex);
    return ok;
}

bool UartBus::Init(UART_HandleTypeDef *huart) {
    if (huart == nullptr) {
        return false;
    }

    if (IsInitialized() && (_huart == huart)) {
        return true;
    }

    _huart = huart;

    return true;
}

bool UartBus::IsInitialized(void) const { return _huart != nullptr; }

bool UartBus::StartReceiving(uint8_t *buffer, const uint16_t size) {

    if (!IsInitialized() || (buffer == nullptr) || (size == 0U)) {
        return false;
    }

    if (_started)
        return true;

    // Read() needs to know where the ring buffer is and how big it
    // is.
    _buffer_ptr = buffer;
    _buffer_size = size;
    _read_pos = 0U;

    const HAL_StatusTypeDef started =
        HAL_UART_Receive_DMA(_huart, buffer, size);

    _started = started == HAL_OK;

    return _started;
}

uint16_t UartBus::Read(uint8_t *out, const uint16_t max_len) {
    if (!_started || (out == nullptr) || (max_len == 0U)) {
        return 0U;
    }

    // Snapshot the DMA write index once. NDTR counts DOWN the bytes
    // remaining until the circular buffer wraps.
    uint16_t write_pos = static_cast<uint16_t>(
        _buffer_size - __HAL_DMA_GET_COUNTER(_huart->hdmarx));
    if (write_pos >= _buffer_size) {
        write_pos = 0U;
    }

    // Keep the compiler/CPU from reading buffer bytes before the NDTR
    // snapshot that says they are valid.
    __DMB();

    uint16_t count = 0U;
    while ((_read_pos != write_pos) && (count < max_len)) {
        out[count++] = _buffer_ptr[_read_pos];
        _read_pos = static_cast<uint16_t>(_read_pos + 1U);
        if (_read_pos == _buffer_size) {
            _read_pos = 0U;
        }
    }

    return count;
}

UartBus &Uart4(void) {
    static UartBus bus;
    return bus;
}

I2cBus &I2c1(void) {
    static I2cBus bus;
    return bus;
}

SpiBus &Spi1(void) {
    static SpiBus bus;
    return bus;
}
