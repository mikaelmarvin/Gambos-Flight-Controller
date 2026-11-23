#ifndef CRSF_HELPER_HPP
#define CRSF_HELPER_HPP

#include <cstddef>
#include <cstdint>

/**
 * CRSF protocol helpers (frame constants, CRC, channel decoding).
 *
 * Frame on the wire: [sync][len][type][payload...][crc]
 * len counts type + payload + crc; crc covers type + payload.
 */
namespace crsf {

/**
 * @brief Decoded CRSF RC_CHANNELS_PACKED frame (type 0x16).
 *
 * Raw format (26 bytes):
 *   [sync 0xC8][len][type 0x16][payload: 22 bytes][crc]
 *
 * Payload: 16 channels x 11 bits, packed LSB first.
 * Value range: 172 / 992 / 1811 (min / centre / max), which maps
 * to 988 / 1500 / 2012 µs RC pulse widths.
 *
 * frame_tick: FreeRTOS tick count when this frame passed its CRC
 * check. Compare with xTaskGetTickCount() to detect a stale link
 * (failsafe).
 *
 * @see
 * https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md#0x16-rc-channels-packed-payload
 */

struct RcChannels {
    static constexpr uint8_t kFrameType = 0x16U;
    static constexpr uint8_t kChannelCount = 16U;
    static constexpr uint8_t kPayloadLen = 22U;
    static constexpr uint8_t kTypePayloadCrcLen = kPayloadLen + 2U;
    uint16_t channel_0;
    uint16_t channel_1;
    uint16_t channel_2;
    uint16_t channel_3;
    uint16_t channel_4;
    uint16_t channel_5;
    uint16_t channel_6;
    uint16_t channel_7;
    uint16_t channel_8;
    uint16_t channel_9;
    uint16_t channel_10;
    uint16_t channel_11;
    uint16_t channel_12;
    uint16_t channel_13;
    uint16_t channel_14;
    uint16_t channel_15;
};

constexpr uint8_t kSyncByte = 0xC8U;
constexpr uint8_t kMaxFrameLen = 64U;
/* Smallest valid len field: type + crc. */
constexpr uint8_t kMinFrameLen = 2U;
constexpr uint32_t kChannelBits = 11U;
constexpr uint32_t kChannelMask = (1U << kChannelBits) - 1U;

// The decode below writes channel_0..channel_15 as one array, which
// only works if they are back to back with no padding. Not guaranteed
// by the C++ standard, so check it at compile time.
static_assert(offsetof(RcChannels, channel_15) -
                      offsetof(RcChannels, channel_0) ==
                  (RcChannels::kChannelCount - 1U) * sizeof(uint16_t),
              "RcChannels channel fields must be contiguous");

/* Unpacks 16 x 11-bit channels (LSB first) from an RC_CHANNELS_PACKED
 * (0x16) payload. */
inline void crsf_decode_rc_channels(const uint8_t *packed,
                                    RcChannels &out) {
    // Treat channel_0..channel_15 as an array (see static_assert).
    uint16_t *const channels =
        reinterpret_cast<uint16_t *>(&out.channel_0);

    for (uint8_t ch = 0U; ch < RcChannels::kChannelCount; ++ch) {
        // Channel ch occupies payload bits [ch * 11, ch * 11 + 10].
        const uint32_t bit_position = ch * kChannelBits;
        const uint32_t chunk_start = bit_position / 8U;
        const uint32_t difference = bit_position % 8U;

        // LSB first: earlier bytes go in the lower bits of the chunk.
        // Imagine laying out the bits in reverse, like this: for 1,
        // 2, 3 -> |0, 0000 0000, 11|00 0000, 0001 0|000, 0000 0001|
        // commas mean 8bit allignment, vertical bar means 11 bit
        // allignment
        uint32_t chunk =
            static_cast<uint32_t>(packed[chunk_start]) |
            (static_cast<uint32_t>(packed[chunk_start + 1U]) << 8U);

        // 11 bits starting at `difference` need a third byte only if
        // they run past the first two (never for the last channel).
        if ((difference + kChannelBits) > 16U) {
            chunk |= static_cast<uint32_t>(packed[chunk_start + 2U])
                     << 16U;
        }

        channels[ch] = static_cast<uint16_t>((chunk >> difference) &
                                             kChannelMask);
    }
}

/* CRC-8/DVB-S2 (poly 0xD5) over type + payload, not sync/len. */
inline uint8_t crsf_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0U;

    while (len-- > 0U) {
        crc ^= *data++;
        for (uint8_t i = 0U; i < 8U; ++i) {
            if ((crc & 0x80U) != 0U) {
                crc = static_cast<uint8_t>(
                    (static_cast<uint32_t>(crc) << 1U) ^ 0xD5U);
            } else {
                crc = static_cast<uint8_t>(crc << 1U);
            }
        }
    }
    return crc;
}

} // namespace crsf

#endif /* CRSF_HELPER_HPP */
