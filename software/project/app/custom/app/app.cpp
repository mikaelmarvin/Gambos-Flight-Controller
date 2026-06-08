/**
 * @file app.cpp
 * @brief Application / test code for custom board (C++17).
 * Startup: main.c → app_init() before osKernelStart().
 */

#include "app/app.hpp"
#include "button_handler/button_handler.hpp"
#include "delayable_handler/delayable_work.hpp"
#include "log.hpp"

namespace {

ButtonHandler g_button_handler;

} // namespace

extern "C" void app_init(void) {
    configASSERT(g_button_handler.Initialize());

    LOG("app_init: starting FreeRTOS tasks\r\n");
    DelayableWork::Start();
    g_button_handler.Start();
}
