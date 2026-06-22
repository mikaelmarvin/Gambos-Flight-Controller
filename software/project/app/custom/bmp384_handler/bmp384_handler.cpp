/**
 * @file bmp384_handler.cpp
 * @brief One task for BMP384 — combined pressure + temperature
 * read/publish.
 */

#include "bmp384_handler.hpp"
#include "messaging/messaging.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kTaskStackWords = 384U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);
constexpr uint32_t kTaskPeriodMs = 50U; /* 20 Hz */

} // namespace

bool Bmp384Handler::Initialize(void) { return _device.Init(); }

void Bmp384Handler::Start(void) {
    configASSERT(xTaskCreate(&Bmp384Handler::TaskFunction,
                             "bmp384_handler",
                             kTaskStackWords,
                             this,
                             kTaskPriority,
                             nullptr) == pdPASS);
}

bool Bmp384Handler::ReadAndPublish(void) {
    BaroSample sample{};
    if (!_device.ReadPressureTemperature(sample)) {
        return false;
    }

    topics::BaroSample topic{};
    topic.pressure_pa = sample.pressure_pa;
    topic.temperature_centi_c = sample.temperature_centi_c;

    return Messaging::Publish<topics::BaroSample>(topic);
}

void Bmp384Handler::TaskFunction(void *pvParameters) {
    Bmp384Handler *const self =
        static_cast<Bmp384Handler *>(pvParameters);

    while (true) {
        (void)self->ReadAndPublish();
        vTaskDelay(pdMS_TO_TICKS(kTaskPeriodMs));
    }
}
