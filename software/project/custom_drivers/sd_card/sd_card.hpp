#ifndef SD_CARD_HPP
#define SD_CARD_HPP

#include "bus.hpp"
#include <cstdint>

class SdCard {
  public:
    SdCard() = default;
    SdCard(SpiBus &bus, CsPin cs);

    bool Initialize();
    bool ReadBlock(uint32_t sector, uint8_t *buff);
    bool WriteBlock(uint32_t sector, const uint8_t *block);
    bool IsHighCapacity(void) const { return _high_capacity; }

  private:
    uint32_t BlockAddress(uint32_t sector) const;

    SpiBus *_bus{nullptr};
    CsPin _cs{};
    bool _high_capacity{false};
};

// Binds FatFs disk_* glue to a concrete SdCard instance.
void RegisterSdCardForFatFs(SdCard &sd_card);

#endif // SD_CARD_HPP
