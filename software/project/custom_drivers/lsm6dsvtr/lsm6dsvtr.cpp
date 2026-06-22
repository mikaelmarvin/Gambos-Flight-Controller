#include "lsm6dsvtr.hpp"

#include "board_buses.hpp"

bool Lsm6dsvtr::Init(void) {
    // TODO: register config (ODR, ranges, BDU) via I2c1().
    return I2c1().IsInitialized();
}

bool Lsm6dsvtr::ReadAccelGyro(AccelSample &accel, GyroSample &gyro) {
    // TODO: e.g. auto-increment burst from OUTX_L_G / OUTX_L_A.
    (void)accel;
    (void)gyro;
    return false;
}
