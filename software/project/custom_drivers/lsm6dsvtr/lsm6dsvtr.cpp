#include "lsm6dsvtr.hpp"

bool Lsm6dsvtr::Init(I2C_HandleTypeDef *i2c) {
    _i2c = i2c;
    // TODO: register config (ODR, ranges, BDU) via *_i2c.
    return (_i2c != nullptr);
}

bool Lsm6dsvtr::ReadAccelGyro(AccelSample &accel, GyroSample &gyro) {
    // TODO: e.g. auto-increment burst from OUTX_L_G / OUTX_L_A.
    (void)accel;
    (void)gyro;
    return false;
}
