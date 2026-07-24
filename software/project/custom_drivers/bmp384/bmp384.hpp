#ifndef BMP384_HPP
#define BMP384_HPP

#include "bus.hpp"

#include <cstdint>

struct BaroSample {
    int32_t pressure_pa{0};
    int16_t temperature_centi_c{0};
};

/**
 * Bosch BMP384 — pressure + temperature on one I2C device.
 *
 * Init loads NVM calibration, then runs normal mode at 25 Hz with
 * pressure x8 / temperature x1 oversampling. ReadPressureTemperature
 * returns false if press+temp data-ready bits are not set; on success
 * it returns compensated Pa and °C×100.
 */
class Bmp384 {
  public:
    Bmp384() = delete;
    Bmp384(I2cBus &bus, uint8_t addr7);

    bool Init(void);
    bool ReadPressureTemperature(BaroSample &out);

  private:
    struct CalibData {
        uint16_t par_t1{0U};
        uint16_t par_t2{0U};
        int8_t par_t3{0};
        int16_t par_p1{0};
        int16_t par_p2{0};
        int8_t par_p3{0};
        int8_t par_p4{0};
        uint16_t par_p5{0U};
        uint16_t par_p6{0U};
        int8_t par_p7{0};
        int8_t par_p8{0};
        int16_t par_p9{0};
        int8_t par_p10{0};
        int8_t par_p11{0};
        int64_t t_lin{0};
    };

    bool SoftReset(void);
    bool LoadCalibration(void);
    int16_t CompensateTemperature(uint32_t uncomp_temp);
    int32_t CompensatePressure(uint32_t uncomp_press);

    I2cBus *_bus{nullptr};
    const uint8_t _addr7{0U};
    CalibData _calib{};
};

#endif /* BMP384_HPP */
