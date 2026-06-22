#include "bmp384.hpp"

Bmp384::Bmp384(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Bmp384::Init(void) {
    return (_bus != nullptr) && _bus->IsInitialized();
}

bool Bmp384::ReadPressureTemperature(BaroSample &out) {
    (void)out;
    return false;
}
