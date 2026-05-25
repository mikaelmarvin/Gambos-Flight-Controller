/**
 * @file iis2mdctr_handler.cpp
 * @brief Periodic blocking magnetometer read → Messaging publish.
 */

#include "iis2mdctr_handler.hpp"
#include "messaging/messaging.hpp"
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kTaskStackWords = 384U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);
constexpr uint32_t kTaskPeriodMs = 100U; /* 10 Hz */

} // namespace

bool Iis2mdctrHandler::Initialize(void) {
    return _device.Init(&hi2c1);
}

void Iis2mdctrHandler::Start(void) {
    configASSERT(xTaskCreate(&Iis2mdctrHandler::TaskFunction,
                             "iis2mdc_handler",
                             kTaskStackWords,
                             this,
                             kTaskPriority,
                             nullptr) == pdPASS);
}

bool Iis2mdctrHandler::ReadAndPublish(void) {
    MagSample sample{};
    if (!_device.ReadSample(sample)) {
        return false;
    }

    topics::MagSample topic{};
    topic.x = sample.x;
    topic.y = sample.y;
    topic.z = sample.z;

    return Messaging::Publish<topics::MagSample>(topic);
}

void Iis2mdctrHandler::TaskFunction(void *pvParameters) {
    Iis2mdctrHandler *const self =
        static_cast<Iis2mdctrHandler *>(pvParameters);

    for (;;) {
        (void)self->ReadAndPublish();
        vTaskDelay(pdMS_TO_TICKS(kTaskPeriodMs));
    }
}
