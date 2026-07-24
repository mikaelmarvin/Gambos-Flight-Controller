#include "bmp384.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr TickType_t kI2cTimeout = pdMS_TO_TICKS(100U);

constexpr uint8_t kRegChipId = 0x00U;
constexpr uint8_t kRegStatus = 0x03U;
constexpr uint8_t kRegData0 = 0x04U;
constexpr uint8_t kRegPwrCtrl = 0x1BU;
constexpr uint8_t kRegOsr = 0x1CU;
constexpr uint8_t kRegOdr = 0x1DU;
constexpr uint8_t kRegCalib = 0x31U;
constexpr uint8_t kRegCmd = 0x7EU;

constexpr uint8_t kChipIdValue = 0x50U;
constexpr uint8_t kCmdSoftReset = 0xB6U;

/** STATUS: cmd_rdy (bit4), drdy_press (bit5), drdy_temp (bit6). */
constexpr uint8_t kStatusCmdRdy = 0x10U;
constexpr uint8_t kStatusDrdyPress = 0x20U;
constexpr uint8_t kStatusDrdyTemp = 0x40U;

/**
 * OSR: osr_p = 011 (×8), osr_t = 000 (×1).
 * Recommended pairing for ×8 pressure oversampling.
 */
constexpr uint8_t kOsrPress8Temp1 = 0x03U;

/** ODR: odr_sel = 0x03 → 25 Hz (closest above the 20 Hz sensing poll). */
constexpr uint8_t kOdr25Hz = 0x03U;

/**
 * PWR_CTRL: press_en | temp_en | mode=normal (bits[5:4]=11).
 */
constexpr uint8_t kPwrNormalPressTemp = 0x33U;

constexpr uint8_t kCalibBytes = 21U;
constexpr uint8_t kDataBytes = 6U;

constexpr int64_t kMinTempCenti = -4000;
constexpr int64_t kMaxTempCenti = 8500;
constexpr uint64_t kMinPressCentiPa = 3000000ULL;
constexpr uint64_t kMaxPressCentiPa = 12500000ULL;

uint16_t ConcatU16(uint8_t msb, uint8_t lsb) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(msb) << 8U) | static_cast<uint16_t>(lsb));
}

uint32_t UnpackU24(const uint8_t *p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8U) |
           (static_cast<uint32_t>(p[2]) << 16U);
}

} // namespace

Bmp384::Bmp384(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Bmp384::SoftReset(void) {
    const uint8_t cmd = kCmdSoftReset;
    if (!_bus->MemWrite(_addr7, kRegCmd, &cmd, 1U, kI2cTimeout)) {
        return false;
    }

    /* Soft-reset completes in ~2 ms; wait then require cmd_rdy. */
    vTaskDelay(pdMS_TO_TICKS(5U));

    uint8_t status = 0U;
    if (!_bus->MemRead(_addr7, kRegStatus, &status, 1U, kI2cTimeout)) {
        return false;
    }
    return (status & kStatusCmdRdy) != 0U;
}

bool Bmp384::LoadCalibration(void) {
    uint8_t raw[kCalibBytes] = {};
    if (!_bus->MemRead(_addr7, kRegCalib, raw, kCalibBytes, kI2cTimeout)) {
        return false;
    }

    /* Trim coefficients from NVM (datasheet Table 23 / Bosch BMP3 API). */
    _calib.par_t1 = ConcatU16(raw[1], raw[0]);
    _calib.par_t2 = ConcatU16(raw[3], raw[2]);
    _calib.par_t3 = static_cast<int8_t>(raw[4]);
    _calib.par_p1 = static_cast<int16_t>(ConcatU16(raw[6], raw[5]));
    _calib.par_p2 = static_cast<int16_t>(ConcatU16(raw[8], raw[7]));
    _calib.par_p3 = static_cast<int8_t>(raw[9]);
    _calib.par_p4 = static_cast<int8_t>(raw[10]);
    _calib.par_p5 = ConcatU16(raw[12], raw[11]);
    _calib.par_p6 = ConcatU16(raw[14], raw[13]);
    _calib.par_p7 = static_cast<int8_t>(raw[15]);
    _calib.par_p8 = static_cast<int8_t>(raw[16]);
    _calib.par_p9 = static_cast<int16_t>(ConcatU16(raw[18], raw[17]));
    _calib.par_p10 = static_cast<int8_t>(raw[19]);
    _calib.par_p11 = static_cast<int8_t>(raw[20]);
    return true;
}

bool Bmp384::Init(void) {
    if (_bus == nullptr || !_bus->IsInitialized()) {
        return false;
    }

    /* Confirm the device on this I2C address is a BMP384 (CHIP_ID = 0x50). */
    uint8_t chip_id = 0U;
    if (!_bus->MemRead(_addr7, kRegChipId, &chip_id, 1U, kI2cTimeout)) {
        return false;
    }
    if (chip_id != kChipIdValue) {
        return false;
    }

    if (!SoftReset()) {
        return false;
    }

    if (!LoadCalibration()) {
        return false;
    }

    /* Configure oversampling / ODR while still in sleep (after reset). */
    if (!_bus->MemWrite(_addr7, kRegOsr, &kOsrPress8Temp1, 1U, kI2cTimeout)) {
        return false;
    }
    if (!_bus->MemWrite(_addr7, kRegOdr, &kOdr25Hz, 1U, kI2cTimeout)) {
        return false;
    }

    /* Enter normal mode and enable pressure + temperature. */
    if (!_bus->MemWrite(_addr7, kRegPwrCtrl, &kPwrNormalPressTemp, 1U,
                        kI2cTimeout)) {
        return false;
    }

    return true;
}

int16_t Bmp384::CompensateTemperature(const uint32_t uncomp_temp) {
    /* Bosch BMP3 integer temperature compensation (°C × 100). */
    const int64_t partial_data1 =
        static_cast<int64_t>(uncomp_temp) -
        (static_cast<int64_t>(256) * _calib.par_t1);
    const int64_t partial_data2 =
        static_cast<int64_t>(_calib.par_t2) * partial_data1;
    const int64_t partial_data3 = partial_data1 * partial_data1;
    const int64_t partial_data4 =
        partial_data3 * static_cast<int64_t>(_calib.par_t3);
    const int64_t partial_data5 =
        (partial_data2 * static_cast<int64_t>(262144)) + partial_data4;
    const int64_t partial_data6 =
        partial_data5 / static_cast<int64_t>(4294967296LL);

    _calib.t_lin = partial_data6;

    int64_t comp_temp =
        (partial_data6 * static_cast<int64_t>(25)) /
        static_cast<int64_t>(16384);
    if (comp_temp < kMinTempCenti) {
        comp_temp = kMinTempCenti;
    } else if (comp_temp > kMaxTempCenti) {
        comp_temp = kMaxTempCenti;
    }
    return static_cast<int16_t>(comp_temp);
}

int32_t Bmp384::CompensatePressure(const uint32_t uncomp_press) {
    /* Bosch BMP3 integer pressure compensation (Pa × 100), then → Pa. */
    const int64_t uncomp = static_cast<int64_t>(uncomp_press);
    const int64_t t_lin = _calib.t_lin;

    const int64_t partial_data1 = t_lin * t_lin;
    const int64_t partial_data2 = partial_data1 / static_cast<int64_t>(64);
    const int64_t partial_data3 =
        (partial_data2 * t_lin) / static_cast<int64_t>(256);
    const int64_t partial_data4 =
        (static_cast<int64_t>(_calib.par_p8) * partial_data3) /
        static_cast<int64_t>(32);
    const int64_t partial_data5 =
        (static_cast<int64_t>(_calib.par_p7) * partial_data1) *
        static_cast<int64_t>(16);
    const int64_t partial_data6 =
        (static_cast<int64_t>(_calib.par_p6) * t_lin) *
        static_cast<int64_t>(4194304);
    const int64_t offset =
        (static_cast<int64_t>(_calib.par_p5) *
         static_cast<int64_t>(140737488355328LL)) +
        partial_data4 + partial_data5 + partial_data6;

    const int64_t partial_data2b =
        (static_cast<int64_t>(_calib.par_p4) * partial_data3) /
        static_cast<int64_t>(32);
    const int64_t partial_data4b =
        (static_cast<int64_t>(_calib.par_p3) * partial_data1) *
        static_cast<int64_t>(4);
    const int64_t partial_data5b =
        (static_cast<int64_t>(_calib.par_p2) -
         static_cast<int64_t>(16384)) *
        t_lin * static_cast<int64_t>(2097152);
    const int64_t sensitivity =
        ((static_cast<int64_t>(_calib.par_p1) -
          static_cast<int64_t>(16384)) *
         static_cast<int64_t>(70368744177664LL)) +
        partial_data2b + partial_data4b + partial_data5b;

    const int64_t term1 =
        (sensitivity / static_cast<int64_t>(16777216)) * uncomp;
    const int64_t partial_data2c =
        static_cast<int64_t>(_calib.par_p10) * t_lin;
    const int64_t partial_data3c =
        partial_data2c +
        (static_cast<int64_t>(65536) * static_cast<int64_t>(_calib.par_p9));
    const int64_t partial_data4c =
        (partial_data3c * uncomp) / static_cast<int64_t>(8192);

    /* /10 then *10 avoids overflow on (uncomp * partial_data4c). */
    int64_t partial_data5c =
        (uncomp * (partial_data4c / static_cast<int64_t>(10))) /
        static_cast<int64_t>(512);
    partial_data5c *= static_cast<int64_t>(10);

    const int64_t partial_data6c = uncomp * uncomp;
    const int64_t partial_data2d =
        (static_cast<int64_t>(_calib.par_p11) * partial_data6c) /
        static_cast<int64_t>(65536);
    const int64_t partial_data3d =
        (partial_data2d * uncomp) / static_cast<int64_t>(128);
    const int64_t partial_data4d =
        (offset / static_cast<int64_t>(4)) + term1 + partial_data5c +
        partial_data3d;

    uint64_t comp_press =
        (static_cast<uint64_t>(partial_data4d) *
         static_cast<uint64_t>(25ULL)) /
        static_cast<uint64_t>(1099511627776ULL);

    if (comp_press < kMinPressCentiPa) {
        comp_press = kMinPressCentiPa;
    } else if (comp_press > kMaxPressCentiPa) {
        comp_press = kMaxPressCentiPa;
    }

    return static_cast<int32_t>(comp_press / 100ULL);
}

bool Bmp384::ReadPressureTemperature(BaroSample &out) {
    if (_bus == nullptr) {
        return false;
    }

    uint8_t status = 0U;
    if (!_bus->MemRead(_addr7, kRegStatus, &status, 1U, kI2cTimeout)) {
        return false;
    }
    if ((status & (kStatusDrdyPress | kStatusDrdyTemp)) !=
        (kStatusDrdyPress | kStatusDrdyTemp)) {
        return false;
    }

    /* Burst DATA_0..DATA_5: pressure (24-bit) then temperature (24-bit). */
    uint8_t raw[kDataBytes] = {};
    if (!_bus->MemRead(_addr7, kRegData0, raw, kDataBytes, kI2cTimeout)) {
        return false;
    }

    const uint32_t uncomp_press = UnpackU24(&raw[0]);
    const uint32_t uncomp_temp = UnpackU24(&raw[3]);

    /* Temperature first: updates t_lin used by pressure compensation. */
    out.temperature_centi_c = CompensateTemperature(uncomp_temp);
    out.pressure_pa = CompensatePressure(uncomp_press);
    return true;
}
