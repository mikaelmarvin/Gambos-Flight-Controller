#include "iis2mdctr.hpp"

#include "board_buses.hpp"

bool Iis2mdctr::Init(void) {
    // TODO: data rate and continuous mode config via I2c1().
    return I2c1().IsInitialized();
}

bool Iis2mdctr::ReadSample(MagSample &out) {
    (void)out;
    return false;
}
