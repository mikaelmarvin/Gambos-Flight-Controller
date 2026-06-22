#include "iis2mdctr.hpp"

Iis2mdctr::Iis2mdctr(I2cBus &bus, const uint8_t addr7)
    : _bus(&bus), _addr7(addr7) {}

bool Iis2mdctr::Init(void) {
    return (_bus != nullptr) && _bus->IsInitialized();
}

bool Iis2mdctr::ReadSample(MagSample &out) {
    (void)out;
    return false;
}
