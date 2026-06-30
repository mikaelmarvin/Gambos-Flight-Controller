/**
 * @file app.cpp
 * @brief Application / test code for custom board (C++17).
 * Startup: main.c → app_init() before osKernelStart() creates app_startup
 * task; flash/storage init runs there once the scheduler is running.
 */

#include "app/app.hpp"
#include "board/board.hpp"
#include "button_handler/button_handler.hpp"
#include "delayable_handler/delayable_work.hpp"
#include "log.hpp"
#include "storage_handler/storage_handler.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kAppStartupStackWords = 512U;
constexpr UBaseType_t kAppStartupPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 2U);

ButtonHandler g_button_handler;
StorageHandler g_storage_handler{board::Flash()};

void AppStartupTask(void *pvParameters) {
    (void)pvParameters;

    configASSERT(board::InitDevices());
    configASSERT(g_storage_handler.Initialize());

    LOG("app_startup: starting FreeRTOS tasks\r\n");
    DelayableWork::Start();
    g_button_handler.Start();
    g_storage_handler.Start();

    vTaskDelete(nullptr);
}

} // namespace

extern "C" void app_init(void) {
    configASSERT(board::InitBuses());
    configASSERT(g_button_handler.Initialize());

    configASSERT(xTaskCreate(AppStartupTask,
                             "app_startup",
                             kAppStartupStackWords,
                             nullptr,
                             kAppStartupPriority,
                             nullptr) == pdPASS);
}
