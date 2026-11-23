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
#include "elrs_handler/elrs_handler.hpp"
#include "log.hpp"
#include "sensing_handler/sensing_handler.hpp"
#include "storage_handler/storage_handler.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kAppStartupStackWords = 1024U;
constexpr UBaseType_t kAppStartupPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 2U);

ButtonHandler g_button_handler;
// ActuatorHandler g_actuator_handler;
StorageHandler g_storage_handler{board::Flash(), board::Sd()};
SensingHandler g_sensing_handler{
    board::Imu(), board::Magnetometer(), board::Baro()};
ElrsHandler g_elrs_handler{board::Receiver()};

// The AppStartupTask is only used in this source file, so it will be
// private/static
void AppStartupTask(void *pvParameters) {
    (void)pvParameters;

    if (!board::InitDevices()) {
        LOG("ERROR: app_startup: device init failed\r\n");
    }
    g_button_handler.Initialize();
    // g_actuator_handler.Initialize());
    g_storage_handler.Initialize();
    g_sensing_handler.Initialize();
    const bool elrs_ok = g_elrs_handler.Initialize();

    LOG("app_startup: free heap %u bytes\r\n",
        static_cast<unsigned>(xPortGetFreeHeapSize()));
    LOG("app_startup: starting FreeRTOS tasks\r\n");
    DelayableWork::Start();
    g_button_handler.Start();
    // g_actuator_handler.Start();
    g_storage_handler.Start();
    g_sensing_handler.Start();
    if (elrs_ok) {
        g_elrs_handler.Start();
    }
    LOG("app_startup: tasks started, free heap %u bytes\r\n",
        static_cast<unsigned>(xPortGetFreeHeapSize()));

    vTaskDelete(nullptr);
}
} // namespace

// We will probably need a separate file for interrupts, but not yet
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == USR_BTN_Pin) {
        ButtonHandler::CallbackFromISR();
        return;
    }
}

extern "C" void app_init(void) {
    // Keep going on failure so unrelated subsystems still start; the
    // handlers that need a missing bus fail their own Initialize().
    if (!board::InitBuses()) {
        LOG("ERROR: app_init: bus init failed\r\n");
    }
    xTaskCreate(AppStartupTask,
                "app_startup",
                kAppStartupStackWords,
                nullptr,
                kAppStartupPriority,
                nullptr);
}
