#include "at25sf128a.hpp"

#include "spi_dma_blocking.hpp"

#include "stm32f4xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr TickType_t kSpiDmaTimeoutMs = 1000U;
constexpr TickType_t kEnsureIdleTimeoutMs = 5000U;

constexpr uint8_t kCmdRead = 0x03U;
constexpr uint8_t kCmdPageProgram = 0x02U;
constexpr uint8_t kCmdWriteEnable = 0x06U;
constexpr uint8_t kCmdReadStatus = 0x05U;
constexpr uint8_t kCmdSectorErase4k = 0x20U;
constexpr uint8_t kCmdEnableReset = 0x66U;
constexpr uint8_t kCmdReset = 0x99U;
constexpr uint8_t kCmdReadJedecId = 0x9FU;

constexpr uint8_t kStatusBusy = 0x01U;

constexpr uint8_t kJedecManufacturerId = 0x1FU;
constexpr uint8_t kJedecMemoryType = 0x89U;
constexpr uint8_t kJedecCapacityId = 0x18U;

bool IsValidRange(uint32_t address, uint32_t size) {
    return (size > 0U) &&
           ((address + size) <= At25sf128a::kCapacityBytes);
}

bool IsSectorAligned(uint32_t address) {
    return (address % At25sf128a::kSectorSizeBytes) == 0U;
}

} // namespace

bool At25sf128a::SpiTransmitDma(const uint8_t *data, uint16_t size) {
    if ((_spi == nullptr) || (data == nullptr) || (size == 0U)) {
        return false;
    }

    if (!SpiDmaBlockingBegin(_spi)) {
        return false;
    }

    if (HAL_SPI_Transmit_DMA(_spi, const_cast<uint8_t *>(data), size) !=
        HAL_OK) {
        SpiDmaBlockingAbort();
        return false;
    }

    return SpiDmaBlockingWait(pdMS_TO_TICKS(kSpiDmaTimeoutMs));
}

bool At25sf128a::SpiReceiveDma(uint8_t *data, uint16_t size) {
    if ((_spi == nullptr) || (data == nullptr) || (size == 0U)) {
        return false;
    }

    if (!SpiDmaBlockingBegin(_spi)) {
        return false;
    }

    if (HAL_SPI_Receive_DMA(_spi, data, size) != HAL_OK) {
        SpiDmaBlockingAbort();
        return false;
    }

    return SpiDmaBlockingWait(pdMS_TO_TICKS(kSpiDmaTimeoutMs));
}

bool At25sf128a::SpiTransmitReceiveDma(const uint8_t *tx,
                                       uint8_t *rx,
                                       uint16_t size) {
    if ((_spi == nullptr) || (tx == nullptr) || (rx == nullptr) ||
        (size == 0U)) {
        return false;
    }

    if (!SpiDmaBlockingBegin(_spi)) {
        return false;
    }

    if (HAL_SPI_TransmitReceive_DMA(_spi,
                                    const_cast<uint8_t *>(tx),
                                    rx,
                                    size) != HAL_OK) {
        SpiDmaBlockingAbort();
        return false;
    }

    return SpiDmaBlockingWait(pdMS_TO_TICKS(kSpiDmaTimeoutMs));
}

bool At25sf128a::Init(SPI_HandleTypeDef *spi,
                      GPIO_TypeDef *cs_port,
                      uint16_t cs_pin) {
    _spi = spi;
    _cs_port = cs_port;
    _cs_pin = cs_pin;

    if ((_spi == nullptr) || (_cs_port == nullptr)) {
        return false;
    }

    if (!SpiDmaBlockingEnsureSemaphore()) {
        return false;
    }

    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);

    if (!Reset()) {
        return false;
    }

    uint8_t id[3] = {};
    if (!JedecID(id)) {
        return false;
    }

    return (id[0] == kJedecManufacturerId) &&
           (id[1] == kJedecMemoryType) && (id[2] == kJedecCapacityId);
}

bool At25sf128a::Read(uint32_t address,
                      uint8_t *data,
                      uint32_t size) {
    if ((data == nullptr) || !IsValidRange(address, size)) {
        return false;
    }

    const uint8_t command[4] = {
        kCmdRead,
        static_cast<uint8_t>((address >> 16U) & 0xFFU),
        static_cast<uint8_t>((address >> 8U) & 0xFFU),
        static_cast<uint8_t>(address & 0xFFU),
    };

    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
    const bool ok =
        SpiTransmitDma(command, sizeof(command)) &&
        SpiReceiveDma(data, static_cast<uint16_t>(size));
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
    return ok;
}

bool At25sf128a::Write(uint32_t address,
                       const uint8_t *data,
                       uint32_t size) {
    if ((data == nullptr) || !IsValidRange(address, size)) {
        return false;
    }

    uint32_t remaining = size;
    uint32_t current_address = address;
    const uint8_t *current_data = data;

    while (remaining > 0U) {
        const uint32_t page_offset =
            current_address % kPageProgramBytes;
        uint32_t chunk = kPageProgramBytes - page_offset;
        if (chunk > remaining) {
            chunk = remaining;
        }

        const uint8_t command[4] = {
            kCmdPageProgram,
            static_cast<uint8_t>((current_address >> 16U) & 0xFFU),
            static_cast<uint8_t>((current_address >> 8U) & 0xFFU),
            static_cast<uint8_t>(current_address & 0xFFU),
        };

        {
            const uint8_t write_enable = kCmdWriteEnable;
            HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
            const bool enabled = SpiTransmitDma(&write_enable, 1U);
            HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
            if (!enabled) {
                return false;
            }
        }

        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
        const bool programmed =
            SpiTransmitDma(command, sizeof(command)) &&
            SpiTransmitDma(current_data, static_cast<uint16_t>(chunk));
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
        if (!programmed) {
            return false;
        }

        if (!EnsureIdle()) {
            return false;
        }

        current_address += chunk;
        current_data += chunk;
        remaining -= chunk;
    }

    return true;
}

bool At25sf128a::Erase(uint32_t address) {
    if ((address >= kCapacityBytes) || !IsSectorAligned(address)) {
        return false;
    }

    {
        const uint8_t write_enable = kCmdWriteEnable;
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
        const bool enabled = SpiTransmitDma(&write_enable, 1U);
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
        if (!enabled) {
            return false;
        }
    }

    const uint8_t command[4] = {
        kCmdSectorErase4k,
        static_cast<uint8_t>((address >> 16U) & 0xFFU),
        static_cast<uint8_t>((address >> 8U) & 0xFFU),
        static_cast<uint8_t>(address & 0xFFU),
    };

    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
    const bool erased = SpiTransmitDma(command, sizeof(command));
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
    if (!erased) {
        return false;
    }

    return EnsureIdle();
}

bool At25sf128a::EnsureIdle(void) {
    uint8_t status = 0U;
    TickType_t elapsed_ms = 0U;

    while (elapsed_ms < kEnsureIdleTimeoutMs) {
        if (!ReadStatus(&status)) {
            return false;
        }
        if ((status & kStatusBusy) == 0U) {
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(1U));
        ++elapsed_ms;
    }

    return false;
}

bool At25sf128a::Reset(void) {
    {
        const uint8_t enable_reset = kCmdEnableReset;
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
        const bool enabled = SpiTransmitDma(&enable_reset, 1U);
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
        if (!enabled) {
            return false;
        }
    }

    {
        const uint8_t reset = kCmdReset;
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
        const bool reset_ok = SpiTransmitDma(&reset, 1U);
        HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
        if (!reset_ok) {
            return false;
        }
    }

    return EnsureIdle();
}

bool At25sf128a::JedecID(uint8_t *id) {
    if (id == nullptr) {
        return false;
    }

    const uint8_t command[4] = {
        kCmdReadJedecId,
        0U,
        0U,
        0U,
    };
    uint8_t response[4] = {};

    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
    const bool ok =
        SpiTransmitReceiveDma(command, response, sizeof(command));
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
    if (!ok) {
        return false;
    }

    id[0] = response[1];
    id[1] = response[2];
    id[2] = response[3];
    return true;
}

bool At25sf128a::ReadStatus(uint8_t *status) {
    if (status == nullptr) {
        return false;
    }

    const uint8_t command[2] = {kCmdReadStatus, 0U};
    uint8_t response[2] = {};

    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_RESET);
    const bool ok =
        SpiTransmitReceiveDma(command, response, sizeof(command));
    HAL_GPIO_WritePin(_cs_port, _cs_pin, GPIO_PIN_SET);
    if (!ok) {
        return false;
    }

    *status = response[1];
    return true;
}
