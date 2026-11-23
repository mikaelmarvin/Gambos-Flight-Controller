#ifndef ELRS_RECEIVER_HPP
#define ELRS_RECEIVER_HPP

#include "FreeRTOS.h"

#include "bus.hpp"
#include "crsf_helper.hpp"

#include <cstdint>

/*
 * ExpressLRS receiver — CRSF frames received over UART.
 *
 * ReadRcChannels() drains the UART ring buffer through the streaming
 * CRSF parser (FeedByte) and hands back the newest valid RC-channels
 * frame together with how fresh it is.
 */
class ElrsReceiver {

  public:
    /** Result of ReadRcChannels(). */
    enum class RcStatus : uint8_t {
        kFresh,         ///< channels filled with a recent frame
        kStale,         ///< last frame too old (link lost?)
        kNeverReceived, ///< no valid RC frame since boot
    };

    ElrsReceiver() = delete;
    explicit ElrsReceiver(UartBus &bus) : _bus(&bus) {}

    // Owns a DMA target buffer: a copy would not be the buffer the
    // DMA writes into.
    ElrsReceiver(const ElrsReceiver &) = delete;
    ElrsReceiver &operator=(const ElrsReceiver &) = delete;

    bool Initialize(void);

    /** Processes all received bytes, then copies the newest RC frame
     * into channels only when the result is kFresh; otherwise
     * channels is left unchanged. */
    RcStatus ReadRcChannels(crsf::RcChannels &channels);

    /** Tick at which the newest valid RC frame passed its CRC. */
    TickType_t LastRcFrameTick(void) const {
        return _last_rc_frame_tick;
    }

    void FeedByte(uint8_t b);

  private:
    void HandleFrame(void);

    enum class ParserState : uint8_t {
        kWaitSync,
        kWaitLen,
        kBody,
    };

    // DMA ring buffer, sized for the worst case between two
    // ReadRcChannels() calls: CRSF runs at 420000 baud 8N1 = 42000
    // bytes/s, so a 20 ms task period can see up to ~840 bytes. If
    // the DMA laps the reader, data is silently overwritten.
    static constexpr uint16_t kRxBufferSize = 1024U;

    UartBus *_bus{nullptr};
    uint8_t _rx_buffer[kRxBufferSize]{};

    ParserState _state = ParserState::kWaitSync;
    uint8_t _frame[crsf::kMaxFrameLen]{};
    // Expected type + payload + crc
    uint8_t _frame_len{0U};
    // Bytes stored after the len field.
    uint8_t _idx{0U};

    crsf::RcChannels _rc_frame{};
    TickType_t _last_rc_frame_tick{0U};
    // Set once the first valid RC frame arrives; never cleared.
    bool _rc_frame_received{false};
    uint32_t _rc_frame_count{0U};
};

#endif /* ELRS_RECEIVER_HPP */
