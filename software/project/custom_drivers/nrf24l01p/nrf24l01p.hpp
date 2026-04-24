#ifndef NRF24L01P_H
#define NRF24L01P_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_spi.h"

#include <stdint.h>

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

    Nrf24l01p(SPI_HandleTypeDef *spi_handle) : _spi_handle(spi_handle) {}
    ~Nrf24l01p() = default;

    uint8_t Init(PrimaryRole primary_role);
    uint8_t RegisterCePin(GPIO_TypeDef *port, uint16_t pin) {
        _ce_pin_.port = port;
        _ce_pin_.pin = pin;
        return 1U;
    }
    uint8_t RegisterCsnPin(GPIO_TypeDef *port, uint16_t pin) {
        _csn_pin_.port = port;
        _csn_pin_.pin = pin;
        return 1U;
    }
    uint8_t Send(const uint8_t *const data);
    uint8_t Receive(uint8_t *const data);

  private:
    /** nRF24 max payload / longest burst we support with fixed [33] SPI buffers. */
    static constexpr uint8_t kMaxFifoBytes = 32U;

    uint8_t NrfSpiExchange(SPI_HandleTypeDef *h, const uint8_t *tx, uint8_t *rx, uint8_t len);
    uint8_t WriteReg(const uint8_t reg, const uint8_t value);
    uint8_t WriteRegArray(const uint8_t reg, const uint8_t *const value, const uint8_t length);
    uint8_t WriteReadRegArray(uint8_t cmd, uint8_t *data, uint8_t len);
    uint8_t Command(const uint8_t value);
    uint8_t ReadStatus();
    uint8_t ToSetupAddressWidth(const uint8_t length, uint8_t &setup_aw_value);

    SPI_HandleTypeDef *_spi_handle;
    Pin _ce_pin_;
    Pin _csn_pin_;
    const uint8_t _payload_length = 1U;
};

class RxNrf24l01p : public Nrf24l01p {
  public:
    uint8_t Init() { return Nrf24l01p::Init(PrimaryRole::Prx); }

    uint8_t Receive(uint8_t *const data) { return Nrf24l01p::Receive(data); }
};

class TxNrf24l01p : public Nrf24l01p {
  public:
    uint8_t Init() { return Nrf24l01p::Init(PrimaryRole::Ptx); }

    uint8_t Send(const uint8_t *const data) { return Nrf24l01p::Send(data); }
};

#endif /* NRF24L01P_H */
