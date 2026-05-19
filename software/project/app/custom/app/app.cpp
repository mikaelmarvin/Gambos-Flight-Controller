/**
 * @file app.cpp
 * @brief Application / test code for custom board (C++17).
 * Startup: main.c → app_init() before osKernelStart();
 *          default task → app_run() after scheduler is running.
 */

#include "app/app.hpp"
#include "button_handler/button_handler.hpp"
#include "log.hpp"
#include "tx_handler/tx_handler.hpp"

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

namespace {

ButtonHandler g_button_handler;
TxHandler g_tx_handler;

} // namespace

extern "C" void app_init(void) {
    configASSERT(g_button_handler.Initialize());
    configASSERT(g_tx_handler.Initialize());
}

extern "C" void app_run(void) {
    LOG("app_run: starting tasks\r\n");
    g_button_handler.Start();
    g_tx_handler.Start();

    for (;;) {
        vTaskDelay(portMAX_DELAY);
    }
}
