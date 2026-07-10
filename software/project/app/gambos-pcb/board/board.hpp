#ifndef BOARD_HPP
#define BOARD_HPP

#include "at25sf128a.hpp"
#include "bmp384.hpp"
#include "bus.hpp"
#include "iis2mdctr.hpp"
#include "lsm6dsvtr.hpp"
#include "nrf24l01p.hpp"

namespace board {

constexpr uint8_t kBmp384I2cAddr7 = 0x76U;
constexpr uint8_t kIis2mdctrI2cAddr7 = 0x1EU;
constexpr uint8_t kLsm6dsvtrI2cAddr7 = 0x6AU;

bool InitBuses(void);
bool InitDevices(void);

At25sf128a &Flash(void);
Bmp384 &Baro(void);
Iis2mdctr &Magnetometer(void);
Lsm6dsvtr &Imu(void);
Nrf24l01p &Radio(void);

} // namespace board

#endif /* BOARD_HPP */
