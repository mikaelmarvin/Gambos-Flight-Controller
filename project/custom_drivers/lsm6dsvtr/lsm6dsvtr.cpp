#include "lsm6dsvtr.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr TickType_t kI2cTimeout = pdMS_TO_TICKS(100U);

constexpr uint8_t kRegWhoAmI = 0x0FU;
constexpr uint8_t kRegCtrl1 = 0x10U;
constexpr uint8_t kRegCtrl2 = 0x11U;
constexpr uint8_t kRegCtrl3 = 0x12U;
constexpr uint8_t kRegCtrl6 = 0x15U;
constexpr uint8_t kRegCtrl8 = 0x17U;
constexpr uint8_t kRegStatus = 0x1EU;
constexpr uint8_t kRegOutXLG = 0x22U;

constexpr uint8_t kWhoAmIValue = 0x70U;

/** CTRL3: BDU (bit6) | IF_INC (bit2). */
constexpr uint8_t kCtrl3BduIfInc = 0x44U;

/**
 * CTRL1/CTRL2: OP_MODE = 000 (high-performance) | ODR = 0110 (120
 * Hz).
 */
constexpr uint8_t kCtrlHp120Hz = 0x06U;

/** CTRL6 FS_G_[3:0] = 0100 → ±2000 dps. */
constexpr uint8_t kCtrl6Fs2000Dps = 0x04U;

/** CTRL8 FS_XL_[1:0] = 10 → ±8 g. */
constexpr uint8_t kCtrl8Fs8g = 0x02U;

constexpr uint8_t kStatusXlda = 0x01U;
constexpr uint8_t kStatusGda = 0x02U;

constexpr uint8_t kAccelGyroRawBytes = 12U;

int16_t Le16(const uint8_t *p) {
    return static_cast<int16_t>(static_cast<uint16_t>(p[0]) |
                                (static_cast<uint16_t>(p[1]) << 8U));
}

} // namespace

Lsm6dsvtr::Lsm6dsvtr(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Lsm6dsvtr::Init(void) {
    if (_bus == nullptr || !_bus->IsInitialized()) {
        return false;
    }

    /* Confirm the device on this I2C address is an LSM6DSV (WHO_AM_I
     * = 0x70). */
    uint8_t who_am_i = 0U;
    if (!_bus->MemRead(
            _addr7, kRegWhoAmI, &who_am_i, 1U, kI2cTimeout)) {
        return false;
    }
    if (who_am_i != kWhoAmIValue) {
        return false;
    }

    /*
     * CTRL3: BDU keeps MSB/LSB of each axis coherent until both are
     * read; IF_INC auto-increments the register address so we can
     * burst-read.
     */
    if (!_bus->MemWrite(
            _addr7, kRegCtrl3, &kCtrl3BduIfInc, 1U, kI2cTimeout)) {
        return false;
    }

    /* CTRL8: accelerometer full-scale ±8 g (set before enabling the
     * sensor). */
    if (!_bus->MemWrite(
            _addr7, kRegCtrl8, &kCtrl8Fs8g, 1U, kI2cTimeout)) {
        return false;
    }

    /* CTRL6: gyroscope full-scale ±2000 dps (set before enabling the
     * sensor). */
    if (!_bus->MemWrite(
            _addr7, kRegCtrl6, &kCtrl6Fs2000Dps, 1U, kI2cTimeout)) {
        return false;
    }

    /*
     * CTRL1 / CTRL2: leave power-down and start sampling.
     * OP_MODE = high-performance, ODR = 120 Hz (same for accel and
     * gyro).
     */
    if (!_bus->MemWrite(
            _addr7, kRegCtrl1, &kCtrlHp120Hz, 1U, kI2cTimeout)) {
        return false;
    }
    if (!_bus->MemWrite(
            _addr7, kRegCtrl2, &kCtrlHp120Hz, 1U, kI2cTimeout)) {
        return false;
    }

    return true;
}

bool Lsm6dsvtr::ReadAccelGyro(AccelSample &accel, GyroSample &gyro) {
    if (_bus == nullptr) {
        return false;
    }

    uint8_t status = 0U;
    if (!_bus->MemRead(
            _addr7, kRegStatus, &status, 1U, kI2cTimeout)) {
        return false;
    }
    if ((status & (kStatusXlda | kStatusGda)) !=
        (kStatusXlda | kStatusGda)) {
        return false;
    }

    uint8_t raw[kAccelGyroRawBytes] = {};
    if (!_bus->MemRead(_addr7,
                       kRegOutXLG,
                       raw,
                       kAccelGyroRawBytes,
                       kI2cTimeout)) {
        return false;
    }

    gyro.x = Le16(&raw[0]);
    gyro.y = Le16(&raw[2]);
    gyro.z = Le16(&raw[4]);
    accel.x = Le16(&raw[6]);
    accel.y = Le16(&raw[8]);
    accel.z = Le16(&raw[10]);
    return true;
}
