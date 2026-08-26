/**
 * @file sd_card.cpp
 * @brief SPI SD card block driver (CMD17/CMD24) — no FatFs.
 */

#include "sd_card.hpp"

#include "stm32f4xx_hal_gpio.h"

#include "task.h"

#include <cstring>

namespace {

constexpr TickType_t kSpiDmaTimeoutMs = pdMS_TO_TICKS(1000U);
constexpr TickType_t kSpiDelayMs = pdMS_TO_TICKS(10U);
constexpr TickType_t kAcmd41TimeoutMs = pdMS_TO_TICKS(1000U);
constexpr unsigned kMaxWaitBytes = 16U;
constexpr unsigned kMaxCmd0Tries = 5U;
constexpr unsigned kDataResponseWaitBytes = 32U;
constexpr unsigned kWritePacketSize = 515U; // token + 512 + crc16

constexpr uint8_t kR1ValidMask = 0x80U;
constexpr uint8_t kR1Idle = 0x01U;
constexpr uint8_t kR1Ready = 0x00U;

constexpr uint8_t kR7ValidVoltageRange = 0x01U;
constexpr uint8_t kCmd8CheckPattern = 0xAAU;
// SD init / identification: fOD <= 400 kHz. APB1 = 45 MHz → /128 ≈
// 352 kHz.
constexpr uint32_t kSdInitPrescaler = SPI_BAUDRATEPRESCALER_128;
// Runtime SD transfers on shared SPI2. Cube default /2 = 22.5 MHz is
// too fast for this slot; /16 ≈ 2.8 MHz is a safe SPI-mode rate.
constexpr uint32_t kSdDataPrescaler = SPI_BAUDRATEPRESCALER_16;
constexpr uint8_t kCmd0GoIdle[6] = {
    0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x95U};
constexpr uint8_t kCmd8SendIfCond[6] = {
    0x48U, 0x00U, 0x00U, 0x01U, 0xAAU, 0x87U};
constexpr uint8_t kCmd55AppCmd[6] = {
    0x77U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U};
// ACMD41 = 0x40|41, arg HCS (bit30) for SDHC/SDXC in SPI mode.
constexpr uint8_t kAcmd41SendOpCond[6] = {
    0x69U, 0x40U, 0x00U, 0x00U, 0x00U, 0x01U};
constexpr uint8_t kCmd58ReadOcr[6] = {
    0x7AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U};
constexpr uint8_t kCmd16SetBlockLen512[6] = {
    0x50U, 0x00U, 0x00U, 0x02U, 0x00U, 0x01U};
constexpr uint8_t kCmd13SendStatus[6] = {
    0x4DU, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U};
constexpr uint8_t kDataBlockStartToken = 0xFEU;
constexpr uint8_t kDataResponseMask = 0x1FU;
constexpr uint8_t kDataResponseAccepted = 0x05U;
constexpr uint8_t kDataResponseTokenMask = 0x11U;
constexpr uint8_t kDataResponseTokenMatch = 0x01U;
constexpr uint8_t kOcrPowerUpStatusMask = 0x80U; // OCR bit31
constexpr uint8_t kOcrCardCapacityStatusMask =
    0x40U;                                          // OCR bit30 (CCS)
constexpr uint16_t kOcrVoltageWindowMask = 0xFF80U; // OCR bits23:15

} // namespace

SdCard::SdCard(SpiBus &bus, CsPin cs) : _bus(&bus), _cs(cs) {}

uint32_t SdCard::BlockAddress(uint32_t sector) const {
    // SDHC/SDXC use LBA; SDSC uses byte address.
    return _high_capacity ? sector : (sector * 512U);
}

void SdCard::CsLow(void) {
    HAL_GPIO_WritePin(_cs.port, _cs.pin, GPIO_PIN_RESET);
}

void SdCard::CsHigh(void) {
    HAL_GPIO_WritePin(_cs.port, _cs.pin, GPIO_PIN_SET);
}

bool SdCard::WaitForR1(uint8_t &r1, TickType_t timeout) {
    for (uint8_t i = 0U; i < kMaxWaitBytes; ++i) {
        const uint8_t tx = 0xFFU;
        uint8_t rx = 0xFFU;
        if (!_bus->TransmitReceiveDma(&tx, &rx, 1U, timeout)) {
            return false;
        }
        if ((rx & kR1ValidMask) == 0U) {
            r1 = rx;
            return true;
        }
    }
    return false;
}

bool SdCard::SendStartupDummyClocks(TickType_t timeout) {
    // >= 74 SCK with CS high (10 bytes * 8 = 80 clocks), MOSI idle
    // high.
    const uint8_t dummy_clocks[10] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return _bus->TransmitDma(
        dummy_clocks, sizeof(dummy_clocks), timeout);
}

bool SdCard::ReleaseBus(TickType_t timeout) {
    // 8 clocks with CS high between transactions (SPI mode
    // requirement).
    const uint8_t dummy = 0xFFU;
    return _bus->TransmitDma(&dummy, 1U, timeout);
}

bool SdCard::EnsureSdDataClock(TickType_t timeout) {
    if (_bus->GetBaudRatePrescaler() == kSdDataPrescaler) {
        return true;
    }
    return _bus->SetBaudRatePrescaler(kSdDataPrescaler, timeout);
}

bool SdCard::WaitReady(TickType_t timeout) {
    // Card must release DO before a new command (elm-chan
    // wait_ready).
    const TickType_t start = xTaskGetTickCount();
    do {
        const uint8_t tx = 0xFFU;
        uint8_t rx = 0xFFU;
        if (!_bus->TransmitReceiveDma(&tx, &rx, 1U, timeout)) {
            return false;
        }
        if (rx == 0xFFU) {
            return true;
        }
    } while ((xTaskGetTickCount() - start) < timeout);
    return false;
}

bool SdCard::SendCommandAndReadR1(const uint8_t (&command)[6],
                                  uint8_t &r1,
                                  TickType_t timeout) {
    // Leading 0xFF after CS assert helps some cards sync.
    const uint8_t lead = 0xFFU;
    if (!_bus->TransmitDma(&lead, 1U, timeout)) {
        return false;
    }
    if (!_bus->TransmitDma(command, sizeof(command), timeout)) {
        return false;
    }
    return WaitForR1(r1, timeout);
}

bool SdCard::WaitForDataResponse(uint8_t &response,
                                 TickType_t timeout) {
    // Data response is one byte: xxx0yyy1. Accepted when yyy == 010
    // (masked value 0x05).
    for (uint8_t i = 0U; i < kDataResponseWaitBytes; ++i) {
        const uint8_t tx = 0xFFU;
        uint8_t rx = 0xFFU;
        if (!_bus->TransmitReceiveDma(&tx, &rx, 1U, timeout)) {
            return false;
        }
        if ((rx & kDataResponseTokenMask) ==
            kDataResponseTokenMatch) {
            response = rx;
            return true;
        }
    }
    return false;
}

bool SdCard::WaitWhileBusy(TickType_t timeout) {
    // After an accepted write, the card holds DO low until
    // programming finishes.
    const TickType_t start = xTaskGetTickCount();
    do {
        const uint8_t tx = 0xFFU;
        uint8_t rx = 0xFFU;
        if (!_bus->TransmitReceiveDma(&tx, &rx, 1U, timeout)) {
            return false;
        }
        if (rx != 0x00U) {
            return true;
        }
    } while ((xTaskGetTickCount() - start) < timeout);

    return false;
}

bool SdCard::WaitForDataToken(TickType_t timeout) {
    // Wait for read start-block token (0xFE). Error tokens are 0x0X.
    const TickType_t start = xTaskGetTickCount();
    do {
        const uint8_t tx = 0xFFU;
        uint8_t rx = 0xFFU;
        if (!_bus->TransmitReceiveDma(&tx, &rx, 1U, timeout)) {
            return false;
        }
        if (rx == kDataBlockStartToken) {
            return true;
        }
        // Do not treat idle/busy bytes as hard errors; wait for 0xFE
        // or timeout.
    } while ((xTaskGetTickCount() - start) < timeout);

    return false;
}

bool SdCard::DiscardCrc16(TickType_t timeout) {
    const uint8_t tx[2] = {0xFFU, 0xFFU};
    uint8_t rx[2] = {0xFFU, 0xFFU};
    return _bus->TransmitReceiveDma(tx, rx, sizeof(tx), timeout);
}

bool SdCard::SendCmd13AndValidateR2(TickType_t timeout) {
    // CMD13 (SEND_STATUS) returns R2: programming-time errors show up
    // here.
    uint8_t r1 = 0xFFU;
    if (!SendCommandAndReadR1(kCmd13SendStatus, r1, timeout)) {
        return false;
    }

    const uint8_t tx = 0xFFU;
    uint8_t r2 = 0xFFU;
    if (!_bus->TransmitReceiveDma(&tx, &r2, 1U, timeout)) {
        return false;
    }

    return (r1 == kR1Ready) && (r2 == 0x00U);
}

bool SdCard::SendCmd8AndValidateR7(TickType_t timeout) {
    uint8_t r1 = 0xFFU;
    if (!SendCommandAndReadR1(kCmd8SendIfCond, r1, timeout)) {
        return false;
    }

    uint8_t r7[4] = {0xFFU, 0xFFU, 0xFFU, 0xFFU};
    const uint8_t tx[4] = {0xFFU, 0xFFU, 0xFFU, 0xFFU};
    if (!_bus->TransmitReceiveDma(tx, r7, 4U, timeout)) {
        return false;
    }

    return (r1 == kR1Idle) &&
           ((r7[2] & 0x0FU) == kR7ValidVoltageRange) &&
           (r7[3] == kCmd8CheckPattern);
}

bool SdCard::SendCmd58AndValidateOcr(TickType_t timeout) {
    uint8_t r1 = 0xFFU;
    if (!SendCommandAndReadR1(kCmd58ReadOcr, r1, timeout) ||
        (r1 != kR1Ready)) {
        return false;
    }

    uint8_t ocr[4] = {0xFFU, 0xFFU, 0xFFU, 0xFFU};
    const uint8_t tx[4] = {0xFFU, 0xFFU, 0xFFU, 0xFFU};
    if (!_bus->TransmitReceiveDma(tx, ocr, 4U, timeout)) {
        return false;
    }

    const bool power_up_done = (ocr[0] & kOcrPowerUpStatusMask) != 0U;
    const uint16_t voltage_window = static_cast<uint16_t>(
        (static_cast<uint16_t>(ocr[1]) << 8U) | ocr[2]);
    const bool voltage_supported =
        (voltage_window & kOcrVoltageWindowMask) != 0U;
    _high_capacity = (ocr[0] & kOcrCardCapacityStatusMask) != 0U;

    // SDSC cards require explicit 512-byte block length.
    if (!_high_capacity) {
        uint8_t cmd16_r1 = 0xFFU;
        if (!SendCommandAndReadR1(
                kCmd16SetBlockLen512, cmd16_r1, timeout) ||
            (cmd16_r1 != kR1Ready)) {
            return false;
        }
    }

    return power_up_done && voltage_supported;
}

bool SdCard::ReadBlock(uint32_t sector, uint8_t *block) {
    if ((_bus == nullptr) || (_cs.port == nullptr) ||
        !_bus->IsInitialized() || (block == nullptr)) {
        return false;
    }

    if (!EnsureSdDataClock(kSpiDmaTimeoutMs)) {
        return false;
    }

    const uint32_t address = BlockAddress(sector);

    // CMD17 (READ_SINGLE_BLOCK): read a single block from the card.
    CsLow();
    uint8_t r1 = 0xFFU;
    uint8_t cmd17ReadBlock[6] = {
        0x51U,
        static_cast<uint8_t>((address >> 24U) & 0xFFU),
        static_cast<uint8_t>((address >> 16U) & 0xFFU),
        static_cast<uint8_t>((address >> 8U) & 0xFFU),
        static_cast<uint8_t>(address & 0xFFU),
        0x01U};

    if (!SendCommandAndReadR1(cmd17ReadBlock, r1, kSpiDmaTimeoutMs) ||
        (r1 != kR1Ready)) {
        CsHigh();
        return false;
    }

    if (!WaitForDataToken(kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    // Full-duplex with 0xFF MOSI is more reliable than RX-only DMA
    // on this HAL for SD.
    static uint8_t tx_ones[512];
    static bool tx_ones_ready = false;
    if (!tx_ones_ready) {
        std::memset(tx_ones, 0xFF, sizeof(tx_ones));
        tx_ones_ready = true;
    }
    if (!_bus->TransmitReceiveDma(
            tx_ones, block, 512U, kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    if (!DiscardCrc16(kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    CsHigh();
    (void)ReleaseBus(kSpiDmaTimeoutMs);
    return true;
}

bool SdCard::WriteBlock(uint32_t sector, const uint8_t *block) {
    if ((_bus == nullptr) || (_cs.port == nullptr) ||
        !_bus->IsInitialized() || (block == nullptr)) {
        return false;
    }

    if (!EnsureSdDataClock(kSpiDmaTimeoutMs)) {
        return false;
    }

    const uint32_t address = BlockAddress(sector);

    // Prefer full-duplex for the data packet so RX is drained and SPI
    // OVR is not left set after a long TX-only DMA.
    static uint8_t write_packet[kWritePacketSize];
    static uint8_t write_rx_discard[kWritePacketSize];
    write_packet[0] = kDataBlockStartToken;
    std::memcpy(&write_packet[1], block, 512U);
    write_packet[513] = 0xFFU;
    write_packet[514] = 0xFFU;

    // CMD24 (WRITE_BLOCK): write a single block to the card.
    CsLow();
    if (!WaitReady(kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    uint8_t r1 = 0xFFU;
    uint8_t cmd24WriteBlock[6] = {
        0x58U,
        static_cast<uint8_t>((address >> 24U) & 0xFFU),
        static_cast<uint8_t>((address >> 16U) & 0xFFU),
        static_cast<uint8_t>((address >> 8U) & 0xFFU),
        static_cast<uint8_t>(address & 0xFFU),
        0x01U}; // CRC is not used.

    if (!SendCommandAndReadR1(
            cmd24WriteBlock, r1, kSpiDmaTimeoutMs) ||
        (r1 != kR1Ready)) {
        CsHigh();
        return false;
    }

    // N_WR gap before data packet.
    const uint8_t n_wr[2] = {0xFFU, 0xFFU};
    if (!_bus->TransmitDma(n_wr, sizeof(n_wr), kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    if (!_bus->TransmitReceiveDma(write_packet,
                                  write_rx_discard,
                                  kWritePacketSize,
                                  kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    // Data response often arrives on MISO during the CRC bytes.
    uint8_t data_response = 0xFFU;
    bool got_response = false;
    for (unsigned i = kWritePacketSize - 8U; i < kWritePacketSize;
         ++i) {
        const uint8_t b = write_rx_discard[i];
        if ((b & kDataResponseTokenMask) == kDataResponseTokenMatch) {
            data_response = b;
            got_response = true;
        }
    }
    if (!got_response &&
        (!WaitForDataResponse(data_response, kSpiDmaTimeoutMs))) {
        CsHigh();
        return false;
    }
    if ((data_response & kDataResponseMask) !=
        kDataResponseAccepted) {
        CsHigh();
        return false;
    }

    if (!WaitWhileBusy(kSpiDmaTimeoutMs)) {
        CsHigh();
        return false;
    }

    CsHigh();
    (void)ReleaseBus(kSpiDmaTimeoutMs);

    // CMD13 recommended; ignore flaky status after a good data
    // accept.
    CsLow();
    if (WaitReady(kSpiDmaTimeoutMs)) {
        (void)SendCmd13AndValidateR2(kSpiDmaTimeoutMs);
    }
    CsHigh();

    (void)ReleaseBus(kSpiDmaTimeoutMs);
    return true;
}

bool SdCard::Init(void) {
    if ((_bus == nullptr) || (_cs.port == nullptr) ||
        !_bus->IsInitialized()) {
        return false;
    }

    _high_capacity = false;

    CsHigh();

    if (!_bus->SetBaudRatePrescaler(kSdInitPrescaler,
                                    kSpiDmaTimeoutMs)) {
        return false;
    }

    bool ok = false;
    do {
        // Must run at <= ~400 kHz (see kSdInitPrescaler).
        if (!SendStartupDummyClocks(kSpiDmaTimeoutMs)) {
            break;
        }

        // CMD0 (GO_IDLE_STATE): enter SPI mode, expect R1 = 0x01.
        bool idle = false;
        for (unsigned attempt = 0U; attempt < kMaxCmd0Tries;
             ++attempt) {
            CsLow();
            uint8_t r1 = 0xFFU;
            if (SendCommandAndReadR1(
                    kCmd0GoIdle, r1, kSpiDmaTimeoutMs) &&
                (r1 == kR1Idle)) {
                idle = true;
            }
            CsHigh();
            (void)ReleaseBus(kSpiDmaTimeoutMs);
            if (idle) {
                break;
            }
            vTaskDelay(kSpiDelayMs);
        }
        if (!idle) {
            break;
        }

        // CMD8 (SEND_IF_COND): card must echo voltage/check
        // pattern.
        CsLow();
        if (!SendCmd8AndValidateR7(kSpiDmaTimeoutMs)) {
            CsHigh();
            break;
        }
        CsHigh();
        (void)ReleaseBus(kSpiDmaTimeoutMs);

        bool card_ready = false;
        uint8_t r1 = 0xFFU;
        const TickType_t acmd41_start = xTaskGetTickCount();
        while ((xTaskGetTickCount() - acmd41_start) <
               kAcmd41TimeoutMs) {
            // CMD55 (APP_CMD): prepare for ACMD41.
            CsLow();
            if (!SendCommandAndReadR1(
                    kCmd55AppCmd, r1, kSpiDmaTimeoutMs) ||
                ((r1 != kR1Idle) && (r1 != kR1Ready))) {
                CsHigh();
                card_ready = false;
                break;
            }
            CsHigh();
            (void)ReleaseBus(kSpiDmaTimeoutMs);

            // ACMD41 (SD_SEND_OP_COND) with HCS for SDHC/SDXC.
            CsLow();
            if (!SendCommandAndReadR1(
                    kAcmd41SendOpCond, r1, kSpiDmaTimeoutMs)) {
                CsHigh();
                card_ready = false;
                break;
            }
            CsHigh();
            (void)ReleaseBus(kSpiDmaTimeoutMs);

            if (r1 == kR1Ready) {
                card_ready = true;
                break;
            }
            if (r1 != kR1Idle) {
                card_ready = false;
                break;
            }

            vTaskDelay(kSpiDelayMs);
        }

        if (!card_ready) {
            break;
        }

        // CMD58 (READ_OCR): card must report a valid OCR.
        CsLow();
        if (!SendCmd58AndValidateOcr(kSpiDmaTimeoutMs)) {
            CsHigh();
            break;
        }
        CsHigh();
        (void)ReleaseBus(kSpiDmaTimeoutMs);

        ok = true;
    } while (false);

    if (ok) {
        // Stay moderate; do not restore Cube /2 (22.5 MHz).
        (void)_bus->SetBaudRatePrescaler(kSdDataPrescaler,
                                         kSpiDmaTimeoutMs);
    } else {
        (void)_bus->SetBaudRatePrescaler(SPI_BAUDRATEPRESCALER_2,
                                         kSpiDmaTimeoutMs);
    }
    return ok;
}
