#ifndef NRF24L01P_HPP
#define NRF24L01P_HPP

#include "bus.hpp"
#include "stm32f4xx_hal.h"

#include <cstdint>

/** Nordic nRF24L01+ — 2.4 GHz radio on SPI (CE/CSN GPIO). */
class Nrf24l01p {
  public:
    enum class PrimaryRole : uint8_t {
        Ptx = 0U,
        Prx = 1U,
    };

    struct Pin {
        GPIO_TypeDef *port{nullptr};
        uint16_t pin{0U};
    };

    Nrf24l01p() = default;

    bool Init(SPI_HandleTypeDef *spi,
              PrimaryRole primary_role,
              Bus *bus);
    bool RegisterCePin(GPIO_TypeDef *port, uint16_t pin);
    bool RegisterCsnPin(GPIO_TypeDef *port, uint16_t pin);
    bool Send(const uint8_t *data);
    bool Receive(uint8_t *data);

  private:
    /** nRF24 max payload / longest burst we support with fixed [33] SPI buffers. */
    static constexpr uint8_t kMaxFifoBytes = 32U;

    bool NrfSpiExchange(SPI_HandleTypeDef *spi,
                        const uint8_t *tx,
                        uint8_t *rx,
                        uint8_t len);
    bool WriteReg(uint8_t reg, uint8_t value);
    bool WriteRegArray(uint8_t reg, const uint8_t *value, uint8_t length);
    bool WriteReadRegArray(uint8_t cmd, uint8_t *data, uint8_t len);
    bool Command(uint8_t value);
    uint8_t ReadStatus();
    bool ToSetupAddressWidth(uint8_t length, uint8_t &setup_aw_value);

    SPI_HandleTypeDef *_spi{nullptr};
    Bus *_bus{nullptr};
    Pin _ce_pin_{};
    Pin _csn_pin_{};
    uint8_t _payload_length{1U};
};

#endif /* NRF24L01P_HPP */
