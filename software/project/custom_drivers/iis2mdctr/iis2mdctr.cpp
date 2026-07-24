#include "iis2mdctr.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr TickType_t kI2cTimeout = pdMS_TO_TICKS(100U);

constexpr uint8_t kRegWhoAmI = 0x4FU;
constexpr uint8_t kRegCfgA = 0x60U;
constexpr uint8_t kRegCfgC = 0x62U;
constexpr uint8_t kRegStatus = 0x67U;
constexpr uint8_t kRegOutXL = 0x68U;

constexpr uint8_t kWhoAmIValue = 0x40U;

/**
 * CFG_REG_A:
 *   COMP_TEMP_EN = 1 (required for proper mag operation)
 *   LP = 0 (high-resolution)
 *   ODR = 00 (10 Hz)
 *   MD = 00 (continuous mode)
 */
constexpr uint8_t kCfgAContHr10Hz = 0x80U;

/** CFG_REG_C: BDU (bit4) — keep MSB/LSB of each axis coherent. */
constexpr uint8_t kCfgCBdu = 0x10U;

/** STATUS_REG: ZYXDA (bit3) — new X/Y/Z sample available. */
constexpr uint8_t kStatusZyxda = 0x08U;

constexpr uint8_t kMagRawBytes = 6U;

int16_t Le16(const uint8_t *p) {
    return static_cast<int16_t>(static_cast<uint16_t>(p[0]) |
                                (static_cast<uint16_t>(p[1]) << 8U));
}

} // namespace

Iis2mdctr::Iis2mdctr(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Iis2mdctr::Init(void) {
    if (_bus == nullptr || !_bus->IsInitialized()) {
        return false;
    }

    /* Confirm the device on this I2C address is an IIS2MDC (WHO_AM_I
     * = 0x40). */
    uint8_t who_am_i = 0U;
    if (!_bus->MemRead(
            _addr7, kRegWhoAmI, &who_am_i, 1U, kI2cTimeout)) {
        return false;
    }
    if (who_am_i != kWhoAmIValue) {
        return false;
    }

    /*
     * CFG_REG_C: BDU avoids tearing when reading multi-byte axis data
     * while the sensor updates outputs.
     */
    if (!_bus->MemWrite(
            _addr7, kRegCfgC, &kCfgCBdu, 1U, kI2cTimeout)) {
        return false;
    }

    /*
     * CFG_REG_A: leave idle, enable temperature compensation, start
     * continuous high-resolution sampling at 10 Hz (matches sensing
     * task).
     */
    if (!_bus->MemWrite(
            _addr7, kRegCfgA, &kCfgAContHr10Hz, 1U, kI2cTimeout)) {
        return false;
    }

    return true;
}

bool Iis2mdctr::ReadSample(MagSample &out) {
    if (_bus == nullptr) {
        return false;
    }

    uint8_t status = 0U;
    if (!_bus->MemRead(
            _addr7, kRegStatus, &status, 1U, kI2cTimeout)) {
        return false;
    }
    if ((status & kStatusZyxda) == 0U) {
        return false;
    }

    uint8_t raw[kMagRawBytes] = {};
    if (!_bus->MemRead(
            _addr7, kRegOutXL, raw, kMagRawBytes, kI2cTimeout)) {
        return false;
    }

    out.x = Le16(&raw[0]);
    out.y = Le16(&raw[2]);
    out.z = Le16(&raw[4]);
    return true;
}
