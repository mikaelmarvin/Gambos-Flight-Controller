#include "bmp384.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr TickType_t kI2cTimeout = pdMS_TO_TICKS(100U);

constexpr uint8_t kRegChipId = 0x00U;
constexpr uint8_t kRegErr = 0x02U;
constexpr uint8_t kRegStatus = 0x03U;
constexpr uint8_t kRegData0 = 0x04U;
constexpr uint8_t kRegPwrCtrl = 0x1BU;
constexpr uint8_t kRegOsr = 0x1CU;
constexpr uint8_t kRegOdr = 0x1DU;
constexpr uint8_t kRegFifoConfig1 = 0x17U;
constexpr uint8_t kRegConfig = 0x1FU;
constexpr uint8_t kRegCalib = 0x31U;
constexpr uint8_t kRegCmd = 0x7EU;

constexpr uint8_t kChipIdValue = 0x50U;
constexpr uint8_t kCmdSoftReset = 0xB6U;
constexpr uint8_t kErrConf = 0x04U;

constexpr uint8_t kStatusCmdRdy = 0x10U;
constexpr uint8_t kStatusDrdyPress = 0x20U;
constexpr uint8_t kStatusDrdyTemp = 0x40U;

/** OSR: press ×2, temp ×1. */
constexpr uint8_t kOsrPress2Temp1 = 0x01U;
constexpr uint8_t kOdr25Hz = 0x03U;

/** press_en | temp_en | sleep. */
constexpr uint8_t kPwrPressTempSleep = 0x03U;
/** press_en | temp_en | normal. */
constexpr uint8_t kPwrPressTempNormal = 0x33U;

constexpr uint8_t kCalibBytes = 21U;

constexpr float kMinTempC = -40.0f;
constexpr float kMaxTempC = 85.0f;
constexpr float kMinPressPa = 30000.0f;
constexpr float kMaxPressPa = 125000.0f;

uint16_t ConcatU16(uint8_t msb, uint8_t lsb) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(msb) << 8U) | static_cast<uint16_t>(lsb));
}

uint32_t UnpackU24(const uint8_t *p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8U) |
           (static_cast<uint32_t>(p[2]) << 16U);
}

bool WriteReg(I2cBus *bus, uint8_t addr7, uint8_t reg, uint8_t value) {
    return bus->MemWrite(addr7, reg, &value, 1U, kI2cTimeout);
}

bool ReadReg(I2cBus *bus, uint8_t addr7, uint8_t reg, uint8_t &value) {
    return bus->MemRead(addr7, reg, &value, 1U, kI2cTimeout);
}

bool WaitStatusBit(I2cBus *bus,
                   uint8_t addr7,
                   uint8_t mask,
                   uint32_t timeout_ms) {
    const TickType_t deadline =
        xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);
    while (xTaskGetTickCount() < deadline) {
        uint8_t status = 0U;
        if (!ReadReg(bus, addr7, kRegStatus, status)) {
            return false;
        }
        if ((status & mask) == mask) {
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1U));
    }
    return false;
}

} // namespace

Bmp384::Bmp384(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Bmp384::SoftReset(void) {
    if (!WriteReg(_bus, _addr7, kRegCmd, kCmdSoftReset)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(10U));

    uint8_t status = 0U;
    if (!ReadReg(_bus, _addr7, kRegStatus, status)) {
        return false;
    }
    return (status & kStatusCmdRdy) != 0U;
}

bool Bmp384::LoadCalibration(void) {
    uint8_t raw[kCalibBytes] = {};
    if (!_bus->MemRead(_addr7, kRegCalib, raw, kCalibBytes, kI2cTimeout)) {
        return false;
    }

    const uint16_t nvm_t1 = ConcatU16(raw[1], raw[0]);
    const uint16_t nvm_t2 = ConcatU16(raw[3], raw[2]);
    const int8_t nvm_t3 = static_cast<int8_t>(raw[4]);
    const int16_t nvm_p1 = static_cast<int16_t>(ConcatU16(raw[6], raw[5]));
    const int16_t nvm_p2 = static_cast<int16_t>(ConcatU16(raw[8], raw[7]));
    const int8_t nvm_p3 = static_cast<int8_t>(raw[9]);
    const int8_t nvm_p4 = static_cast<int8_t>(raw[10]);
    const uint16_t nvm_p5 = ConcatU16(raw[12], raw[11]);
    const uint16_t nvm_p6 = ConcatU16(raw[14], raw[13]);
    const int8_t nvm_p7 = static_cast<int8_t>(raw[15]);
    const int8_t nvm_p8 = static_cast<int8_t>(raw[16]);
    const int16_t nvm_p9 = static_cast<int16_t>(ConcatU16(raw[18], raw[17]));
    const int8_t nvm_p10 = static_cast<int8_t>(raw[19]);
    const int8_t nvm_p11 = static_cast<int8_t>(raw[20]);

    if ((nvm_t1 == 0U) || (nvm_t2 == 0U) || (nvm_p5 == 0U)) {
        return false;
    }

    /* Quantize NVM → float coeffs (Bosch BMP3 API / datasheet §9.1). */
    _calib.par_t1 = static_cast<float>(nvm_t1) * 256.0f;
    _calib.par_t2 = static_cast<float>(nvm_t2) / 1073741824.0f;
    _calib.par_t3 = static_cast<float>(nvm_t3) / 281474976710656.0f;
    _calib.par_p1 =
        (static_cast<float>(nvm_p1) - 16384.0f) / 1048576.0f;
    _calib.par_p2 =
        (static_cast<float>(nvm_p2) - 16384.0f) / 536870912.0f;
    _calib.par_p3 = static_cast<float>(nvm_p3) / 4294967296.0f;
    _calib.par_p4 = static_cast<float>(nvm_p4) / 137438953472.0f;
    _calib.par_p5 = static_cast<float>(nvm_p5) * 8.0f;
    _calib.par_p6 = static_cast<float>(nvm_p6) / 64.0f;
    _calib.par_p7 = static_cast<float>(nvm_p7) / 256.0f;
    _calib.par_p8 = static_cast<float>(nvm_p8) / 32768.0f;
    _calib.par_p9 = static_cast<float>(nvm_p9) / 281474976710656.0f;
    _calib.par_p10 = static_cast<float>(nvm_p10) / 281474976710656.0f;
    _calib.par_p11 =
        static_cast<float>(static_cast<double>(nvm_p11) /
                           36893488147419103232.0);
    return true;
}

bool Bmp384::Init(void) {
    if (_bus == nullptr || !_bus->IsInitialized()) {
        return false;
    }

    uint8_t chip_id = 0U;
    if (!ReadReg(_bus, _addr7, kRegChipId, chip_id) ||
        (chip_id != kChipIdValue)) {
        return false;
    }

    if (!SoftReset() || !LoadCalibration()) {
        return false;
    }

    if (!WriteReg(_bus, _addr7, kRegOsr, kOsrPress2Temp1) ||
        !WriteReg(_bus, _addr7, kRegOdr, kOdr25Hz) ||
        !WriteReg(_bus, _addr7, kRegFifoConfig1, 0x00U) ||
        !WriteReg(_bus, _addr7, kRegConfig, 0x00U)) {
        return false;
    }

    /* Enable sensors in sleep, then enter normal mode. */
    if (!WriteReg(_bus, _addr7, kRegPwrCtrl, kPwrPressTempSleep)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(5U));
    if (!WriteReg(_bus, _addr7, kRegPwrCtrl, kPwrPressTempNormal)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(40U));

    uint8_t pwr = 0U;
    if (!ReadReg(_bus, _addr7, kRegPwrCtrl, pwr) ||
        (pwr != kPwrPressTempNormal)) {
        return false;
    }

    uint8_t err = 0U;
    if (ReadReg(_bus, _addr7, kRegErr, err) && ((err & kErrConf) != 0U)) {
        return false;
    }
    return true;
}

int16_t Bmp384::CompensateTemperature(const uint32_t uncomp_temp) {
    const float partial_data1 =
        static_cast<float>(uncomp_temp) - _calib.par_t1;
    const float partial_data2 = partial_data1 * _calib.par_t2;
    _calib.t_lin =
        partial_data2 + (partial_data1 * partial_data1) * _calib.par_t3;

    float temperature = _calib.t_lin;
    if (temperature < kMinTempC) {
        temperature = kMinTempC;
    } else if (temperature > kMaxTempC) {
        temperature = kMaxTempC;
    }
    return static_cast<int16_t>(temperature * 100.0f);
}

int32_t Bmp384::CompensatePressure(const uint32_t uncomp_press) {
    const float uncomp = static_cast<float>(uncomp_press);
    const float t = _calib.t_lin;
    const float t2 = t * t;
    const float t3 = t2 * t;

    const float partial_out1 = _calib.par_p5 + (_calib.par_p6 * t) +
                               (_calib.par_p7 * t2) + (_calib.par_p8 * t3);
    const float partial_out2 =
        uncomp * (_calib.par_p1 + (_calib.par_p2 * t) + (_calib.par_p3 * t2) +
                  (_calib.par_p4 * t3));
    const float partial_data3 =
        (uncomp * uncomp) * (_calib.par_p9 + _calib.par_p10 * t);
    const float partial_data4 =
        partial_data3 + (uncomp * uncomp * uncomp) * _calib.par_p11;

    float pressure = partial_out1 + partial_out2 + partial_data4;
    if (pressure < kMinPressPa) {
        pressure = kMinPressPa;
    } else if (pressure > kMaxPressPa) {
        pressure = kMaxPressPa;
    }
    return static_cast<int32_t>(pressure);
}

bool Bmp384::ReadPressureTemperature(BaroSample &out) {
    if (_bus == nullptr) {
        return false;
    }

    if (!WaitStatusBit(_bus, _addr7, kStatusDrdyPress | kStatusDrdyTemp,
                       50U)) {
        return false;
    }

    uint8_t raw[6] = {};
    if (!_bus->MemRead(_addr7, kRegData0, raw, 6U, kI2cTimeout)) {
        return false;
    }

    const uint32_t uncomp_press = UnpackU24(&raw[0]);
    const uint32_t uncomp_temp = UnpackU24(&raw[3]);
    out.temperature_centi_c = CompensateTemperature(uncomp_temp);
    out.pressure_pa = CompensatePressure(uncomp_press);
    return true;
}
