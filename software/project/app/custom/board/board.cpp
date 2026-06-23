#include "board.hpp"

#include "log.hpp"
#include "main.h"

#include "i2c.h"
#include "spi.h"

namespace board {

namespace {

At25sf128a g_flash{Spi1(), {FLASH_CS_GPIO_Port, FLASH_CS_Pin}};
Bmp384 g_baro{I2c1(), kBmp384I2cAddr7};
Iis2mdctr g_magnetometer{I2c1(), kIis2mdctrI2cAddr7};
Lsm6dsvtr g_imu{I2c1(), kLsm6dsvtrI2cAddr7};
Nrf24l01p g_radio{};

} // namespace

bool InitBuses(void) {
    if (!Spi1().Init(&hspi1)) {
        LOG("ERROR: SPI1 bus init failed\r\n");
        return false;
    }

    if (!Spi2().Init(&hspi2)) {
        LOG("ERROR: SPI2 bus init failed\r\n");
        return false;
    }

    if (!I2c1().Init(&hi2c1)) {
        LOG("ERROR: I2C1 bus init failed\r\n");
        return false;
    }

    return true;
}

bool InitDevices(void) {
    if (!g_flash.Init()) {
        return false;
    }

    (void)g_baro.Init();
    (void)g_magnetometer.Init();
    (void)g_imu.Init();

    return true;
}

At25sf128a &Flash(void) { return g_flash; }
Bmp384 &Baro(void) { return g_baro; }
Iis2mdctr &Magnetometer(void) { return g_magnetometer; }
Lsm6dsvtr &Imu(void) { return g_imu; }
Nrf24l01p &Radio(void) { return g_radio; }

} // namespace board
