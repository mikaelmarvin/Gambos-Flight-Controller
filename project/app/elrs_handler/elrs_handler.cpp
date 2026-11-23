/**
 * @file elrs_handler.cpp
 * @brief ExpressLRS receiver handler.
 */

#include "elrs_handler.hpp"
#include "crsf_helper.hpp"
#include "log.hpp"
#include "messaging/messaging.hpp"
#include <cstdint>

namespace {

constexpr uint32_t kElrsHandlerTaskStackSize = 512U;
constexpr uint32_t kElrsHandlerTaskPriority = (tskIDLE_PRIORITY + 2U);
constexpr uint32_t kElrsHandlerTaskDelayMs = 20U;

// ExpressLRS default channel order is AETR (CRSF channels 0..3).
// Change here if the transmitter is set up differently.
uint16_t Aileron(const crsf::RcChannels &c) { return c.channel_0; }
uint16_t Elevator(const crsf::RcChannels &c) { return c.channel_1; }
uint16_t Throttle(const crsf::RcChannels &c) { return c.channel_2; }
uint16_t Rudder(const crsf::RcChannels &c) { return c.channel_3; }

} // namespace

bool ElrsHandler::Initialize(void) {
    if (!_elrs_receiver.Initialize()) {
        LOG("ERROR: elrs receiver initialization failed\r\n");
        return false;
    }
    return true;
}

void ElrsHandler::Start(void) {
    const BaseType_t created = xTaskCreate(&ElrsHandler::TaskFunction,
                                           "elrs_handler",
                                           kElrsHandlerTaskStackSize,
                                           this,
                                           kElrsHandlerTaskPriority,
                                           &_task_handle);
    configASSERT(created == pdPASS);
}

void ElrsHandler::TaskFunction(void *pvParameters) {
    ElrsHandler *const self =
        static_cast<ElrsHandler *>(pvParameters);

    using RcStatus = ElrsReceiver::RcStatus;

    TickType_t last_wake = xTaskGetTickCount();
    crsf::RcChannels channels{};
    topics::RcChannels topic{};
    RcStatus last_status = RcStatus::kNeverReceived;

    while (true) {
        const RcStatus status =
            self->_elrs_receiver.ReadRcChannels(channels);

        // Only fresh data is published: subscribers detect link loss
        // by the topic going quiet (compare frame_tick with now).
        if (status == RcStatus::kFresh) {
            topic.frame_tick = self->_elrs_receiver.LastRcFrameTick();
            topic.throttle = Throttle(channels);
            topic.aileron = Aileron(channels);
            topic.elevator = Elevator(channels);
            topic.rudder = Rudder(channels);
            Messaging::Publish<topics::RcChannels>(topic);
        }

        // Log link transitions once, not every cycle.
        if (status != last_status) {
            if (status == RcStatus::kFresh) {
                LOG("elrs: link up\r\n");
            } else if (status == RcStatus::kStale) {
                LOG("WARN: elrs: link lost (frames stale)\r\n");
            }
            last_status = status;
        }

        vTaskDelayUntil(&last_wake,
                        pdMS_TO_TICKS(kElrsHandlerTaskDelayMs));
    }
}
