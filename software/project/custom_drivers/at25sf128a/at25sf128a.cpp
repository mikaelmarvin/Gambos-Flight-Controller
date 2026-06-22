#include "at25sf128a.hpp"

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

At25sf128a::At25sf128a(SpiBus &bus, const CsPin cs)
    : _bus(&bus), _cs(cs) {}

bool At25sf128a::Init(void) {
    if ((_bus == nullptr) || (_cs.port == nullptr) ||
        !_bus->IsInitialized()) {
        return false;
    }

    HAL_GPIO_WritePin(_cs.port, _cs.pin, GPIO_PIN_SET);

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
    if ((data == nullptr) || !IsValidRange(address, size) ||
        (_bus == nullptr)) {
        return false;
    }

    const uint8_t command[4] = {
        kCmdRead,
        static_cast<uint8_t>((address >> 16U) & 0xFFU),
        static_cast<uint8_t>((address >> 8U) & 0xFFU),
        static_cast<uint8_t>(address & 0xFFU),
    };

    const TickType_t timeout = pdMS_TO_TICKS(kSpiDmaTimeoutMs);
    CsAssert cs(_cs);
    return _bus->TransmitDma(command, sizeof(command), timeout) &&
           _bus->ReceiveDma(data, static_cast<uint16_t>(size), timeout);
}

bool At25sf128a::Write(uint32_t address,
                       const uint8_t *data,
                       uint32_t size) {
    if ((data == nullptr) || !IsValidRange(address, size) ||
        (_bus == nullptr)) {
        return false;
    }

    const TickType_t timeout = pdMS_TO_TICKS(kSpiDmaTimeoutMs);

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
            CsAssert cs(_cs);
            if (!_bus->TransmitDma(&write_enable, 1U, timeout)) {
                return false;
            }
        }

        {
            CsAssert cs(_cs);
            if (!_bus->TransmitDma(command, sizeof(command), timeout) ||
                !_bus->TransmitDma(current_data,
                                   static_cast<uint16_t>(chunk),
                                   timeout)) {
                return false;
            }
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
    if ((address >= kCapacityBytes) || !IsSectorAligned(address) ||
        (_bus == nullptr)) {
        return false;
    }

    const TickType_t timeout = pdMS_TO_TICKS(kSpiDmaTimeoutMs);

    {
        const uint8_t write_enable = kCmdWriteEnable;
        CsAssert cs(_cs);
        if (!_bus->TransmitDma(&write_enable, 1U, timeout)) {
            return false;
        }
    }

    const uint8_t command[4] = {
        kCmdSectorErase4k,
        static_cast<uint8_t>((address >> 16U) & 0xFFU),
        static_cast<uint8_t>((address >> 8U) & 0xFFU),
        static_cast<uint8_t>(address & 0xFFU),
    };

    {
        CsAssert cs(_cs);
        if (!_bus->TransmitDma(command, sizeof(command), timeout)) {
            return false;
        }
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
    if (_bus == nullptr) {
        return false;
    }

    const TickType_t timeout = pdMS_TO_TICKS(kSpiDmaTimeoutMs);

    {
        const uint8_t enable_reset = kCmdEnableReset;
        CsAssert cs(_cs);
        if (!_bus->TransmitDma(&enable_reset, 1U, timeout)) {
            return false;
        }
    }

    {
        const uint8_t reset = kCmdReset;
        CsAssert cs(_cs);
        if (!_bus->TransmitDma(&reset, 1U, timeout)) {
            return false;
        }
    }

    return EnsureIdle();
}

bool At25sf128a::JedecID(uint8_t *id) {
    if ((id == nullptr) || (_bus == nullptr)) {
        return false;
    }

    const uint8_t command[4] = {
        kCmdReadJedecId,
        0U,
        0U,
        0U,
    };
    uint8_t response[4] = {};

    const TickType_t timeout = pdMS_TO_TICKS(kSpiDmaTimeoutMs);
    CsAssert cs(_cs);
    if (!_bus->TransmitReceiveDma(
            command, response, sizeof(command), timeout)) {
        return false;
    }

    id[0] = response[1];
    id[1] = response[2];
    id[2] = response[3];
    return true;
}

bool At25sf128a::ReadStatus(uint8_t *status) {
    if ((status == nullptr) || (_bus == nullptr)) {
        return false;
    }

    const uint8_t command[2] = {kCmdReadStatus, 0U};
    uint8_t response[2] = {};

    const TickType_t timeout = pdMS_TO_TICKS(kSpiDmaTimeoutMs);
    CsAssert cs(_cs);
    if (!_bus->TransmitReceiveDma(
            command, response, sizeof(command), timeout)) {
        return false;
    }

    *status = response[1];
    return true;
}
