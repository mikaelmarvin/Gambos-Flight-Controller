#include "board.hpp"

#include "log.hpp"
#include "main.h"

#include "i2c.h"
#include "spi.h"

namespace board {

namespace {

At25sf128a g_flash{Spi2(), {FLASH_CS_GPIO_Port, FLASH_CS_Pin}};
SdCard g_sd{Spi2(), {SD_CS_GPIO_Port, SD_CS_Pin}};
Bmp384 g_baro{I2c1(), kBmp384I2cAddr7};
Iis2mdctr g_magnetometer{I2c1(), kIis2mdctrI2cAddr7};
Lsm6dsvtr g_imu{I2c1(), kLsm6dsvtrI2cAddr7};

} // namespace

bool InitBuses(void) {
    if (!Spi2().Init(&hspi2)) {
        LOG("ERROR: SPI2 bus init failed\r\n");
        return false;
    }

    // if (!Spi1().Init(&hspi1)) {
    //     LOG("ERROR: SPI1 bus init failed\r\n");
    //     return false;
    // }

    if (!I2c1().Init(&hi2c1)) {
        LOG("ERROR: I2C1 bus init failed\r\n");
        return false;
    }

    return true;
}

bool InitDevices(void) {
    // SD shares SPI2 with flash; Cube leaves SD_CS asserted low.
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

    if (!g_flash.Init()) {
        LOG("ERROR: FLASH init failed\r\n");
        return false;
    }

    /* I2C sensors are initialized by SensingHandler. */
    /* SD card is initialized by SdHandler (FatFs mount). */
    return true;
}

At25sf128a &Flash(void) { return g_flash; }
SdCard &Sd(void) { return g_sd; }
Bmp384 &Baro(void) { return g_baro; }
Iis2mdctr &Magnetometer(void) { return g_magnetometer; }
Lsm6dsvtr &Imu(void) { return g_imu; }

} // namespace board
