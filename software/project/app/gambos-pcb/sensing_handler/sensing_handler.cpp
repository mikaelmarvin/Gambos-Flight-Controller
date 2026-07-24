/**
 * @file sensing_handler.cpp
 * @brief One task for LSM6DSVTR + IIS2MDCTR + BMP384 on shared I2C.
 */

#include "sensing_handler/sensing_handler.hpp"
#include "log.hpp"
#include "messaging/messaging.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace {

constexpr uint32_t kTaskStackWords = 768U;
constexpr UBaseType_t kTaskPriority =
    static_cast<UBaseType_t>(tskIDLE_PRIORITY + 1U);

/** Base tick ≈ IMU ODR (120 Hz). */
constexpr uint32_t kTaskPeriodMs = 8U;

/** Baro every 6 ticks → 20 Hz. */
constexpr uint32_t kBaroDecimation = 6U;

/** Mag every 12 ticks → 10 Hz. */
constexpr uint32_t kMagDecimation = 12U;

} // namespace

SensingHandler::SensingHandler(Lsm6dsvtr &imu,
                               Iis2mdctr &mag,
                               Bmp384 &baro)
    : _imu(imu), _mag(mag), _baro(baro) {}

bool SensingHandler::Initialize(void) {
    if (!_imu.Init()) {
        LOG("ERROR: LSM6DSVTR init failed\r\n");
        return false;
    }
    if (!_mag.Init()) {
        LOG("ERROR: IIS2MDCTR init failed\r\n");
        return false;
    }
    if (!_baro.Init()) {
        LOG("ERROR: BMP384 init failed\r\n");
        return false;
    }
    LOG("sensing: IMU + mag + baro init OK\r\n");
    return true;
}

void SensingHandler::Start(void) {
    const BaseType_t created =
        xTaskCreate(&SensingHandler::TaskFunction,
                    "sensing",
                    kTaskStackWords,
                    this,
                    kTaskPriority,
                    nullptr);
    if (created != pdPASS) {
        LOG("ERROR: sensing task create failed (heap=%u)\r\n",
            static_cast<unsigned>(xPortGetFreeHeapSize()));
        configASSERT(false);
    }
}

bool SensingHandler::ReadAndPublishImu(void) {
    AccelSample accel{};
    GyroSample gyro{};
    if (!_imu.ReadAccelGyro(accel, gyro)) {
        return false;
    }

    topics::AccelSample accel_topic{};
    accel_topic.x = accel.x;
    accel_topic.y = accel.y;
    accel_topic.z = accel.z;
    (void)Messaging::Publish<topics::AccelSample>(accel_topic);

    topics::GyroSample gyro_topic{};
    gyro_topic.x = gyro.x;
    gyro_topic.y = gyro.y;
    gyro_topic.z = gyro.z;
    return Messaging::Publish<topics::GyroSample>(gyro_topic);
}

bool SensingHandler::ReadAndPublishMag(void) {
    MagSample sample{};
    if (!_mag.ReadSample(sample)) {
        return false;
    }

    topics::MagSample topic{};
    topic.x = sample.x;
    topic.y = sample.y;
    topic.z = sample.z;
    return Messaging::Publish<topics::MagSample>(topic);
}

bool SensingHandler::ReadAndPublishBaro(void) {
    BaroSample sample{};
    if (!_baro.ReadPressureTemperature(sample)) {
        return false;
    }

    topics::BaroSample topic{};
    topic.pressure_pa = sample.pressure_pa;
    topic.temperature_centi_c = sample.temperature_centi_c;
    return Messaging::Publish<topics::BaroSample>(topic);
}

void SensingHandler::TaskFunction(void *pvParameters) {
    SensingHandler *const self =
        static_cast<SensingHandler *>(pvParameters);

    while (true) {
        (void)self->ReadAndPublishImu();

        self->_loop_count++;
        if ((self->_loop_count % kBaroDecimation) == 0U) {
            (void)self->ReadAndPublishBaro();
        }
        if ((self->_loop_count % kMagDecimation) == 0U) {
            (void)self->ReadAndPublishMag();
        }

        vTaskDelay(pdMS_TO_TICKS(kTaskPeriodMs));
    }
}
