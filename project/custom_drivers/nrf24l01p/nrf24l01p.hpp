#ifndef NRF24L01P_H
#define NRF24L01P_H

#include "driver_configurator/driver_configurator.hpp"

#include "stm32f4xx.h"

#include <stdint.h>

class Nrf24l01p {
  public:
    /** Matches CONFIG.PRIM_RX: PTX = 0, PRX = 1. */
    enum class PrimaryRole : uint8_t {
        Ptx = 0U,
        Prx = 1U,
    };

    struct Pin {
        /** HAL_GPIO_* expects non-const GPIO_TypeDef* (peripheral base address). */
        GPIO_TypeDef *port{nullptr};
        uint16_t pin{0U};
    };

    bool Init(PrimaryRole primary_role);
    bool RegisterCePin(GPIO_TypeDef *port, uint16_t pin) {
        ce_pin_.port = port;
        ce_pin_.pin = pin;
        return true;
    }
    bool RegisterCsnPin(GPIO_TypeDef *port, uint16_t pin) {
        csn_pin_.port = port;
        csn_pin_.pin = pin;
        return true;
    }
    bool Send(const uint8_t *const data, const uint8_t length);
    bool Receive(uint8_t *const data, const uint8_t length);

  private:
    bool WriteReg(const uint8_t reg, const uint8_t value);
    bool WriteRegArray(const uint8_t reg,
                       const uint8_t *const value,
                       const uint8_t length);
    bool Command(const uint8_t value);
    uint8_t ReadStatus();
    bool ToSetupAddressWidth(const uint8_t length,
                             uint8_t &setup_aw_value);

    Pin ce_pin_;
    Pin csn_pin_;
};

class RxNrf24l01p : public Nrf24l01p {
  public:
    bool Init() { return Nrf24l01p::Init(PrimaryRole::Prx); }

    bool Receive(uint8_t *const data, const uint8_t length) {
        return Nrf24l01p::Receive(data, length);
    }
};

class TxNrf24l01p : public Nrf24l01p {
  public:
    bool Init() { return Nrf24l01p::Init(PrimaryRole::Ptx); }

    bool Send(const uint8_t *const data, const uint8_t length) {
        return Nrf24l01p::Send(data, length);
    }
};

#endif /* NRF24L01P_H */
