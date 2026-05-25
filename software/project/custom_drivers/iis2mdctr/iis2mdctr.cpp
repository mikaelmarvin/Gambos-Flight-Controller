#include "iis2mdctr.hpp"

bool Iis2mdctr::Init(I2C_HandleTypeDef *i2c) {
    _i2c = i2c;
    // TODO: data rate and continuous mode config via *_i2c.
    return (_i2c != nullptr);
}

bool Iis2mdctr::ReadSample(MagSample &out) {
    (void)out;
    return false;
}
