#include "bmp384.hpp"

#include "board_buses.hpp"

bool Bmp384::Init(void) {
    // TODO: oversampling and normal mode config via I2c1().
    return I2c1().IsInitialized();
}

bool Bmp384::ReadPressureTemperature(BaroSample &out) {
    (void)out;
    return false;
}
