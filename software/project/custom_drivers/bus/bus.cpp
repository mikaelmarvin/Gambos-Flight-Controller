#include "bus.hpp"

#include "semphr.h"
#include "task.h"

#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32f4xx_hal_i2c.h"
#endif

namespace {

constexpr size_t kMaxSpiBuses = 2U;

struct SpiRegistration {
    SPI_HandleTypeDef *handle;
    Bus *bus;
};

SpiRegistration spi_registry[kMaxSpiBuses]{};
size_t spi_registration_count = 0U;

#ifdef HAL_I2C_MODULE_ENABLED
constexpr size_t kMaxI2cBuses = 1U;

struct I2cRegistration {
    I2C_HandleTypeDef *handle;
    Bus *bus;
};

I2cRegistration i2c_registry[kMaxI2cBuses]{};
size_t i2c_registration_count = 0U;

uint16_t ToHalDevAddress(const uint16_t dev_addr_7bit) {
    return static_cast<uint16_t>(dev_addr_7bit << 1U);
}
#endif

void RegisterSpi(SPI_HandleTypeDef *spi, Bus *bus) {
    if ((spi == nullptr) || (bus == nullptr)) {
        return;
    }

    for (size_t i = 0U; i < spi_registration_count; ++i) {
        if (spi_registry[i].handle == spi) {
            spi_registry[i].bus = bus;
            return;
        }
    }

    if (spi_registration_count < kMaxSpiBuses) {
        spi_registry[spi_registration_count].handle = spi;
        spi_registry[spi_registration_count].bus = bus;
        ++spi_registration_count;
    }
}

#ifdef HAL_I2C_MODULE_ENABLED
void RegisterI2c(I2C_HandleTypeDef *hi2c, Bus *bus) {
    if ((hi2c == nullptr) || (bus == nullptr)) {
        return;
    }

    for (size_t i = 0U; i < i2c_registration_count; ++i) {
        if (i2c_registry[i].handle == hi2c) {
            i2c_registry[i].bus = bus;
            return;
        }
    }

    if (i2c_registration_count < kMaxI2cBuses) {
        i2c_registry[i2c_registration_count].handle = hi2c;
        i2c_registry[i2c_registration_count].bus = bus;
        ++i2c_registration_count;
    }
}
#endif

void SignalSpiComplete(SPI_HandleTypeDef *hspi) {
    for (size_t i = 0U; i < spi_registration_count; ++i) {
        if (spi_registry[i].handle == hspi) {
            spi_registry[i].bus->SignalFromIsr();
            return;
        }
    }
}

#ifdef HAL_I2C_MODULE_ENABLED
void SignalI2cComplete(I2C_HandleTypeDef *hi2c) {
    for (size_t i = 0U; i < i2c_registration_count; ++i) {
        if (i2c_registry[i].handle == hi2c) {
            i2c_registry[i].bus->SignalFromIsr();
            return;
        }
    }
}
#endif

} // namespace

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiComplete(hspi);
}

extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiComplete(hspi);
}

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiComplete(hspi);
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    SignalSpiComplete(hspi);
}

#ifdef HAL_I2C_MODULE_ENABLED
extern "C" void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cComplete(hi2c);
}

extern "C" void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cComplete(hi2c);
}

extern "C" void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cComplete(hi2c);
}

extern "C" void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cComplete(hi2c);
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    SignalI2cComplete(hi2c);
}
#endif

bool Bus::EnsureDmaSemaphore(void) {
    if (_dma_sem == nullptr) {
        _dma_sem = xSemaphoreCreateBinaryStatic(&_dma_sem_buffer);
    }
    return _dma_sem != nullptr;
}

bool Bus::Init(SPI_HandleTypeDef *spi) {
    if (spi == nullptr) {
        return false;
    }

    if (IsInitialized() && (_kind == Kind::Spi) && (_spi == spi)) {
        return true;
    }

    _kind = Kind::Spi;
    _spi = spi;
    _hi2c = nullptr;

    if (!EnsureDmaSemaphore()) {
        _kind = Kind::Uninitialized;
        return false;
    }

    RegisterSpi(_spi, this);
    return true;
}

#ifdef HAL_I2C_MODULE_ENABLED
bool Bus::Init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == nullptr) {
        return false;
    }

    if (IsInitialized() && (_kind == Kind::I2c) && (_hi2c == hi2c)) {
        return true;
    }

    _kind = Kind::I2c;
    _hi2c = hi2c;
    _spi = nullptr;

    if (_bus_mutex == nullptr) {
        _bus_mutex = xSemaphoreCreateMutexStatic(&_bus_mutex_buffer);
    }
    if (_bus_mutex == nullptr) {
        _kind = Kind::Uninitialized;
        return false;
    }

    if (!EnsureDmaSemaphore()) {
        _kind = Kind::Uninitialized;
        return false;
    }

    RegisterI2c(_hi2c, this);
    return true;
}
#else
bool Bus::Init(I2C_HandleTypeDef *hi2c) {
    (void)hi2c;
    return false;
}
#endif

bool Bus::IsInitialized(void) const {
    return _kind != Kind::Uninitialized;
}

Bus::Kind Bus::GetKind(void) const { return _kind; }

SPI_HandleTypeDef *Bus::Spi(void) const { return _spi; }

I2C_HandleTypeDef *Bus::I2c(void) const { return _hi2c; }

bool Bus::BeginDma(void) {
    if (!EnsureDmaSemaphore()) {
        return false;
    }

    while (xSemaphoreTake(_dma_sem, 0) == pdTRUE) {}

    return true;
}

bool Bus::WaitDma(TickType_t timeout) {
    if (_dma_sem == nullptr) {
        return false;
    }

    return xSemaphoreTake(_dma_sem, timeout) == pdTRUE;
}

void Bus::SignalFromIsr(void) {
    if (_dma_sem == nullptr) {
        return;
    }
    BaseType_t hpw = pdFALSE;
    (void)xSemaphoreGiveFromISR(_dma_sem, &hpw);
    portYIELD_FROM_ISR(hpw);
}

#ifdef HAL_I2C_MODULE_ENABLED
bool Bus::MemRead(const uint16_t dev_addr_7bit,
                  const uint16_t reg,
                  uint8_t *data,
                  const uint16_t len,
                  const TickType_t timeout) {
    if ((_kind != Kind::I2c) || (_hi2c == nullptr) || (data == nullptr) ||
        (len == 0U) || (_bus_mutex == nullptr)) {
        return false;
    }

    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started = HAL_I2C_Mem_Read_DMA(
            _hi2c,
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

bool Bus::MemWrite(const uint16_t dev_addr_7bit,
                   const uint16_t reg,
                   const uint8_t *data,
                   const uint16_t len,
                   const TickType_t timeout) {
    if ((_kind != Kind::I2c) || (_hi2c == nullptr) || (data == nullptr) ||
        (len == 0U) || (_bus_mutex == nullptr)) {
        return false;
    }

    if (xSemaphoreTake(_bus_mutex, timeout) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (BeginDma()) {
        const HAL_StatusTypeDef started = HAL_I2C_Mem_Write_DMA(
            _hi2c,
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
#else
bool Bus::MemRead(const uint16_t dev_addr_7bit,
                  const uint16_t reg,
                  uint8_t *data,
                  const uint16_t len,
                  const TickType_t timeout) {
    (void)dev_addr_7bit;
    (void)reg;
    (void)data;
    (void)len;
    (void)timeout;
    return false;
}

bool Bus::MemWrite(const uint16_t dev_addr_7bit,
                   const uint16_t reg,
                   const uint8_t *data,
                   const uint16_t len,
                   const TickType_t timeout) {
    (void)dev_addr_7bit;
    (void)reg;
    (void)data;
    (void)len;
    (void)timeout;
    return false;
}
#endif
