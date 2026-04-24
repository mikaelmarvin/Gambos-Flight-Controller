/**
 * @file app.cpp
 * @brief Application / test code for devkit (C++17).
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
    (void)g_button_handler.Initialize();
    (void)g_tx_handler.Initialize();
}

extern "C" void app_run(void) {
    {
        static const uint8_t k_app[] = "\r\n[app] app_run entered (HAL_UART before printf)\r\n";
        (void)HAL_UART_Transmit(&huart2, k_app, sizeof(k_app) - 1U, HAL_MAX_DELAY);
    }
    LOG("app_run: starting tasks\r\n");
    g_button_handler.Start();
    g_tx_handler.Start();
    LOG("app_run: default task idle (button=B1/PC13, LD2 toggles on EXTI)\r\n");

    for (;;) {
        vTaskDelay(portMAX_DELAY);
    }
}
