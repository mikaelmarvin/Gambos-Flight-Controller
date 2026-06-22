#include "board_buses.hpp"

Bus &Spi1(void) {
    static Bus bus;
    return bus;
}

Bus &Spi2(void) {
    static Bus bus;
    return bus;
}

Bus &I2c1(void) {
    static Bus bus;
    return bus;
}
