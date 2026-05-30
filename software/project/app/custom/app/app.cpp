/**
 * @file app.cpp
 * @brief Application / test code for custom board (C++17).
 * Startup: main.c → app_init() before osKernelStart();
 *          default task → app_run() after scheduler is running.
 */

#include "app/app.hpp"
#include "bmp384_handler/bmp384_handler.hpp"
#include "button_handler/button_handler.hpp"
#include "delayable_work/delayable_work.hpp"
#include "iis2mdctr_handler/iis2mdctr_handler.hpp"
#include "log.hpp"
#include "lsm6dsvtr_handler/lsm6dsvtr_handler.hpp"
#include "tx_handler/tx_handler.hpp"

#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"

namespace {

ButtonHandler g_button_handler;
// TxHandler g_tx_handler;
// Lsm6dsvtrHandler g_lsm6dsvtr_handler;
// Iis2mdctrHandler g_iis2mdctr_handler;
// Bmp384Handler g_bmp384_handler;

// bool g_tx_handler_ok = false;
// bool g_lsm6dsvtr_handler_ok = false;
// bool g_iis2mdctr_handler_ok = false;
// bool g_bmp384_handler_ok = false;

} // namespace

extern "C" void app_init(void) {
    /* Must not block on optional peripherals — app_init runs before
     * osKernelStart(); drivers that use FreeRTOS must init in
     * Start(). */
    configASSERT(g_button_handler.Initialize());
    // g_tx_handler_ok = g_tx_handler.Initialize();
    // g_lsm6dsvtr_handler_ok = g_lsm6dsvtr_handler.Initialize();
    // g_iis2mdctr_handler_ok = g_iis2mdctr_handler.Initialize();
    // g_bmp384_handler_ok = g_bmp384_handler.Initialize();
}

extern "C" void app_run(void) {
    LOG("app_run: starting tasks\r\n");
    DelayableWork::Start();
    g_button_handler.Start();
    // if (g_tx_handler_ok) {
    //     g_tx_handler.Start();
    // } else {
    //     LOG("app_run: tx_handler skipped (init failed)\r\n");
    // }
    // if (g_lsm6dsvtr_handler_ok) {
    //     g_lsm6dsvtr_handler.Start();
    // }
    // if (g_iis2mdctr_handler_ok) {
    //     g_iis2mdctr_handler.Start();
    // }
    // if (g_bmp384_handler_ok) {
    //     g_bmp384_handler.Start();
    // }

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
