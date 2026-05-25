/**
 * @file lsm6dsvtr_handler.cpp
 * @brief One task for LSM6DSVTR — combined accel+gyro read, dual publish rates.
 */

#include "lsm6dsvtr_handler.hpp"
#include "messaging/messaging.hpp"
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kTaskStackWords = 384U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);
constexpr uint32_t kTaskPeriodMs = 10U; /* 100 Hz */

/** Publish gyro every N accel cycles (10 → 10 Hz gyro, 100 Hz accel). */
constexpr uint32_t kGyroPublishDecimation = 10U;

} // namespace

bool Lsm6dsvtrHandler::Initialize(void) {
    return _device.Init(&hi2c1);
}

void Lsm6dsvtrHandler::Start(void) {
    configASSERT(xTaskCreate(&Lsm6dsvtrHandler::TaskFunction,
                             "lsm6_handler",
                             kTaskStackWords,
                             this,
                             kTaskPriority,
                             nullptr) == pdPASS);
}

bool Lsm6dsvtrHandler::ReadAndPublish(void) {
    AccelSample accel{};
    GyroSample gyro{};
    if (!_device.ReadAccelGyro(accel, gyro)) {
        return false;
    }

    topics::AccelSample accel_topic{};
    accel_topic.x = accel.x;
    accel_topic.y = accel.y;
    accel_topic.z = accel.z;
    (void)Messaging::Publish<topics::AccelSample>(accel_topic);

    _loop_count++;
    if ((_loop_count % kGyroPublishDecimation) != 0U) {
        return true;
    }

    topics::GyroSample gyro_topic{};
    gyro_topic.x = gyro.x;
    gyro_topic.y = gyro.y;
    gyro_topic.z = gyro.z;
    return Messaging::Publish<topics::GyroSample>(gyro_topic);
}

void Lsm6dsvtrHandler::TaskFunction(void *pvParameters) {
    Lsm6dsvtrHandler *const self =
        static_cast<Lsm6dsvtrHandler *>(pvParameters);

    for (;;) {
        (void)self->ReadAndPublish();
        vTaskDelay(pdMS_TO_TICKS(kTaskPeriodMs));
    }
}
