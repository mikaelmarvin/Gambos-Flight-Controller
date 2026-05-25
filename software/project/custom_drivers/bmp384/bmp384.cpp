#include "bmp384.hpp"

bool Bmp384::Init(I2C_HandleTypeDef *i2c) {
    _i2c = i2c;
    // TODO: oversampling and normal mode config via *_i2c.
    return (_i2c != nullptr);
}

bool Bmp384::ReadPressureTemperature(BaroSample &out) {
    (void)out;
    return false;
}
