#ifndef AT25SF128A_HPP
#define AT25SF128A_HPP

#include "stm32f4xx_hal.h"

#include <cstdint>

// SPI DMA NOR flash driver. Transfers block the calling task on a semaphore
// until the DMA completion callback runs.
class At25sf128a {
  public:
    // AT25SF128A (128 Mbit) — see Renesas datasheet.
    static constexpr uint32_t kCapacityBytes = 16U * 1024U * 1024U;
    static constexpr uint32_t kSectorSizeBytes = 4096U;
    static constexpr uint32_t kPageProgramBytes = 256U;

    At25sf128a() = default;

    bool Init(SPI_HandleTypeDef *spi,
              GPIO_TypeDef *cs_port,
              uint16_t cs_pin);

    bool Read(uint32_t address, uint8_t *data, uint32_t size);
    bool Write(uint32_t address, const uint8_t *data, uint32_t size);
    bool Erase(uint32_t address);
    bool EnsureIdle(void);
    bool Reset(void);
    bool JedecID(uint8_t *id);
    bool ReadStatus(uint8_t *status);

  private:
    bool SpiTransmitDma(const uint8_t *data, uint16_t size);
    bool SpiReceiveDma(uint8_t *data, uint16_t size);
    bool SpiTransmitReceiveDma(const uint8_t *tx,
                               uint8_t *rx,
                               uint16_t size);

    SPI_HandleTypeDef *_spi{nullptr};
    GPIO_TypeDef *_cs_port{nullptr};
    uint16_t _cs_pin{0};
};

#endif /* AT25SF128A_HPP */
