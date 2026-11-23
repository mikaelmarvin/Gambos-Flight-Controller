/**
 * @file elrs_receiver.cpp
 * @brief Where commands are read with help of the UART driver,
 * tracked state of the CRSF packet fields and a list of commands is
 * given
 */

#include "elrs_receiver.hpp"

#include "FreeRTOS.h"
#include "crsf_helper.hpp"
#include "task.h"

#include <cstdint>
#include <cstring>

namespace {

constexpr uint32_t kMaxStaleFrameMs = 500U;

} // namespace

bool ElrsReceiver::Initialize(void) {
    if (!_bus->IsInitialized()) {
        return false;
    }

    if (!_bus->StartReceiving(_rx_buffer, sizeof(_rx_buffer))) {
        return false;
    }

    return true;
}

ElrsReceiver::RcStatus
ElrsReceiver::ReadRcChannels(crsf::RcChannels &channels) {
    // Chunk size only sets how many Read() passes the drain takes; the
    // loop below empties the ring regardless.
    uint8_t buffer[crsf::kMaxFrameLen];
    uint16_t bytes_read = 0U;

    // Empty the receiving buffer that is filled by circular DMA
    do {
        bytes_read = _bus->Read(buffer, sizeof(buffer));

        for (uint16_t i = 0; i < bytes_read; ++i) {
            FeedByte(buffer[i]);
        }
    } while (bytes_read != 0U);

    // A valid rc frame must arrive first
    if (!_rc_frame_received) {
        return RcStatus::kNeverReceived;
    }

    // Unsigned subtraction stays correct across tick-counter wrap.
    const TickType_t now = xTaskGetTickCount();
    const bool stale =
        (now - _last_rc_frame_tick) > pdMS_TO_TICKS(kMaxStaleFrameMs);
    if (stale) {
        return RcStatus::kStale;
    }

    channels = _rc_frame;
    return RcStatus::kFresh;
}

void ElrsReceiver::FeedByte(const uint8_t b) {
    switch (_state) {
    case ParserState::kWaitSync:
        if (b == crsf::kSyncByte) {
            _idx = 0U;
            _state = ParserState::kWaitLen;
        }
        break;

    case ParserState::kWaitLen:
        if ((b < crsf::kMinFrameLen) || (b > crsf::kMaxFrameLen)) {
            // A sync byte here may be the start of the next frame.
            _state = (b == crsf::kSyncByte) ? ParserState::kWaitLen
                                            : ParserState::kWaitSync;
            _idx = 0U;
            break;
        }

        _frame_len = b;
        _idx = 0U;
        _state = ParserState::kBody;
        break;

    case ParserState::kBody:
        _frame[_idx++] = b;
        if (_idx < _frame_len) {
            break;
        }

        HandleFrame();
        _state = ParserState::kWaitSync;
        break;

    default:
        _state = ParserState::kWaitSync;
        break;
    }
}

void ElrsReceiver::HandleFrame(void) {
    // _frame[0] = type, _frame[1..] = payload,
    // _frame[_frame_len - 1] = crc.
    const uint8_t crc_idx = static_cast<uint8_t>(_frame_len - 1U);
    if (crsf::crsf_crc8(_frame, crc_idx) != _frame[crc_idx]) {
        return;
    }

    // Decode RcChannels type of frame
    if ((_frame[0] == crsf::RcChannels::kFrameType) &&
        (_frame_len == crsf::RcChannels::kTypePayloadCrcLen)) {
        crsf::crsf_decode_rc_channels(&_frame[1], _rc_frame);
        _last_rc_frame_tick = xTaskGetTickCount();
        _rc_frame_received = true;
        _rc_frame_count++;
    }
}
