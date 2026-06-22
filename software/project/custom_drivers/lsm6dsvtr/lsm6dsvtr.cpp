#include "lsm6dsvtr.hpp"

Lsm6dsvtr::Lsm6dsvtr(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Lsm6dsvtr::Init(void) {
    return (_bus != nullptr) && _bus->IsInitialized();
}

bool Lsm6dsvtr::ReadAccelGyro(AccelSample &accel, GyroSample &gyro) {
    (void)accel;
    (void)gyro;
    return false;
}
