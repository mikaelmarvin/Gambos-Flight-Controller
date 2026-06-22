/**
 * @file app.cpp
 * @brief Application / test code for custom board (C++17).
 * Startup: main.c → app_init() before osKernelStart().
 */

#include "app/app.hpp"
#include "board_buses.hpp"
#include "button_handler/button_handler.hpp"
#include "delayable_handler/delayable_work.hpp"
#include "log.hpp"
#include "storage_handler/storage_handler.hpp"

#include "i2c.h"
#include "spi.h"

namespace {

ButtonHandler g_button_handler;
StorageHandler g_storage_handler;

} // namespace

extern "C" void app_init(void) {
    configASSERT(Spi1().Init(&hspi1));
    configASSERT(Spi2().Init(&hspi2));
    configASSERT(I2c1().Init(&hi2c1));

    configASSERT(g_button_handler.Initialize());
    configASSERT(g_storage_handler.Initialize());

    LOG("app_init: starting FreeRTOS tasks\r\n");
    DelayableWork::Start();
    g_button_handler.Start();
    g_storage_handler.Start();
}
