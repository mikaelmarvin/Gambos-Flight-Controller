#ifndef AT25SF128A_HPP
#define AT25SF128A_HPP

#include "bus.hpp"

#include <cstdint>

// SPI DMA NOR flash driver. Transfers block the calling task on a
// semaphore until the DMA completion callback runs.
class At25sf128a {
  public:
    // AT25SF128A (128 Mbit) — see Renesas datasheet.
    static constexpr uint32_t kCapacityBytes = 16U * 1024U * 1024U;
    static constexpr uint32_t kSectorSizeBytes = 4096U;
    static constexpr uint32_t kPageProgramBytes = 256U;

    At25sf128a() = default;
    At25sf128a(SpiBus &bus, CsPin cs);

    bool Init(void);

    bool Read(uint32_t address, uint8_t *data, uint32_t size);
    bool Write(uint32_t address, const uint8_t *data, uint32_t size);
    bool Erase(uint32_t address);
    bool EnsureIdle(void);
    bool Reset(void);
    bool JedecID(uint8_t *id);
    bool ReadStatus(uint8_t *status);

  private:
    SpiBus *_bus{nullptr};
    CsPin _cs{};
};

#endif /* AT25SF128A_HPP */
