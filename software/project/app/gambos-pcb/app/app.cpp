/**
 * @file app.cpp
 * @brief Application / test code for custom board (C++17).
 * Startup: main.c → app_init() before osKernelStart() creates
 * app_startup task; flash/storage init runs there once the scheduler
 * is running.
 */

#include "app/app.hpp"
#include "actuator_handler/actuator_handler.hpp"
#include "board/board.hpp"
#include "button_handler/button_handler.hpp"
#include "delayable_handler/delayable_work.hpp"
#include "log.hpp"
#include "sd_handler/sd_handler.hpp"
#include "sensing_handler/sensing_handler.hpp"
#include "storage_handler/storage_handler.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kAppStartupStackWords = 1024U;
constexpr UBaseType_t kAppStartupPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 2U);

ButtonHandler g_button_handler;
ActuatorHandler g_actuator_handler;
StorageHandler g_storage_handler{board::Flash()};
SdHandler g_sd_handler{board::Sd()};
SensingHandler g_sensing_handler{
    board::Imu(), board::Magnetometer(), board::Baro()};

void AppStartupTask(void *pvParameters) {
    (void)pvParameters;

    configASSERT(board::InitDevices());
    configASSERT(g_actuator_handler.Initialize());
    configASSERT(g_storage_handler.Initialize());
    configASSERT(g_sd_handler.Initialize());
    configASSERT(g_sensing_handler.Initialize());

    LOG("app_startup: free heap %u bytes\r\n",
        static_cast<unsigned>(xPortGetFreeHeapSize()));
    LOG("app_startup: starting FreeRTOS tasks\r\n");
    DelayableWork::Start();
    g_button_handler.Start();
    g_actuator_handler.Start();
    g_storage_handler.Start();
    g_sensing_handler.Start();
    LOG("app_startup: tasks started, free heap %u bytes\r\n",
        static_cast<unsigned>(xPortGetFreeHeapSize()));

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
