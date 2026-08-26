#ifndef SD_CARD_HPP
#define SD_CARD_HPP

#include "bus.hpp"

#include <cstdint>

// SPI DMA SD card block driver. Transfers block the calling task on a
// semaphore until the DMA completion callback runs. No filesystem —
// only init and 512-byte sector R/W (FatFs lives in sd_fs).
class SdCard {
  public:
    static constexpr uint32_t kBlockSizeBytes = 512U;

    SdCard() = default;
    SdCard(SpiBus &bus, CsPin cs);

    bool Init(void);

    bool ReadBlock(uint32_t sector, uint8_t *buff);
    bool WriteBlock(uint32_t sector, const uint8_t *block);
    bool IsHighCapacity(void) const { return _high_capacity; }

  private:
    uint32_t BlockAddress(uint32_t sector) const;

    void CsLow(void);
    void CsHigh(void);

    bool WaitForR1(uint8_t &r1, TickType_t timeout);
    bool SendStartupDummyClocks(TickType_t timeout);
    bool ReleaseBus(TickType_t timeout);
    bool EnsureSdDataClock(TickType_t timeout);
    bool WaitReady(TickType_t timeout);
    bool SendCommandAndReadR1(const uint8_t (&command)[6],
                              uint8_t &r1,
                              TickType_t timeout);
    bool WaitForDataResponse(uint8_t &response, TickType_t timeout);
    bool WaitWhileBusy(TickType_t timeout);
    bool WaitForDataToken(TickType_t timeout);
    bool DiscardCrc16(TickType_t timeout);
    bool SendCmd13AndValidateR2(TickType_t timeout);
    bool SendCmd8AndValidateR7(TickType_t timeout);
    bool SendCmd58AndValidateOcr(TickType_t timeout);

    SpiBus *_bus{nullptr};
    CsPin _cs{};
    bool _high_capacity{false};
};

#endif // SD_CARD_HPP
