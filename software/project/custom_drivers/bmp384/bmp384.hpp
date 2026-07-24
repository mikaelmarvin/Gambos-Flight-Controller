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
 * Init loads NVM calibration and starts normal mode (press×2, temp×1,
 * ODR 25 Hz). ReadPressureTemperature waits for DRDY then returns
 * compensated Pa and °C×100.
 */
class Bmp384 {
  public:
    Bmp384() = delete;
    Bmp384(I2cBus &bus, uint8_t addr7);

    bool Init(void);
    bool ReadPressureTemperature(BaroSample &out);

  private:
    struct CalibData {
        float par_t1{0.0f};
        float par_t2{0.0f};
        float par_t3{0.0f};
        float par_p1{0.0f};
        float par_p2{0.0f};
        float par_p3{0.0f};
        float par_p4{0.0f};
        float par_p5{0.0f};
        float par_p6{0.0f};
        float par_p7{0.0f};
        float par_p8{0.0f};
        float par_p9{0.0f};
        float par_p10{0.0f};
        float par_p11{0.0f};
        float t_lin{0.0f};
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
