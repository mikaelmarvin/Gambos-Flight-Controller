#include "nrf24l01p.hpp"
#include "nrf24l01p_hw.h"
#include "nrf24l01p_ll.h"

#include "stm32f4xx_hal_gpio.h"

namespace {

constexpr uint8_t kAddressLength = 5U;
// Default ShockBurst address (same for TX and RX pipe 0 for ACK
// return path).
constexpr uint8_t kAddressBytes[5U] = {
    0xE7U, 0xE7U, 0xE7U, 0xE7U, 0xE7U};

} // namespace

bool Nrf24l01p::Init(const PrimaryRole primary_role) {
    // setup address width is used later to set the address width
    // register
    uint8_t setup_aw = 0U;
    if (!ToSetupAddressWidth(kAddressLength, setup_aw)) {
        return false;
    }

    // --- Initialization actions ---
    // Action 1:
    // - Power down first, go to Power Down mode.
    uint8_t config = 0x00U;
    if (!WriteReg(NRF24_REG_CONFIG, config)) {
        return false;
    }

    // Action 2:
    // - Flush TX and RX FIFOs.
    if (!Command(NRF24_CMD_FLUSH_TX) ||
        !Command(NRF24_CMD_FLUSH_RX)) {
        return false;
    }

    // Action 3:
    // - Clear interrupt flags in STATUS register.
    uint8_t status = ReadStatus();
    status |=
        NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT;
    if (!WriteReg(NRF24_REG_STATUS, status)) {
        return false;
    }

    // ---Program the radio configuration---
    // Configuration 1:
    // - Setup address width.
    if (!WriteReg(NRF24_REG_SETUP_AW, setup_aw)) {
        return false;
    }

    // Configuration 2:
    // - RF channel: 0th channel = 2.400 GHz + 0 MHz.
    if (!WriteReg(NRF24_REG_RF_CH, 0x00U)) {
        return false;
    }

    // Configuration 3:
    // - RF setup: 250 kbps mode, 0 dBm output power.
    config = NRF24_RF_SETUP_RF_DR_LOW;
    config |= NRF24_RF_SETUP_RF_PWR_0dBm;
    if (!WriteReg(NRF24_REG_RF_SETUP, config)) {
        return false;
    }

    // Configuration 4:
    // - Disable dynamic payload length, payload with ACK and noACK
    // command.
    if (!WriteReg(NRF24_REG_FEATURE, 0x00U)) {
        return false;
    }

    // Configuration 5:
    // - Enable auto-ack only for pipe 0.
    if (!WriteReg(NRF24_REG_EN_AA, NRF24_ENAA_P0)) {
        return false;
    }

    // Configuration 6:
    // - Enable RX data pipe 0.
    if (!WriteReg(NRF24_REG_EN_RXADDR, NRF24_ERX_P0)) {
        return false;
    }

    // Configuration 7:
    // - Automatic re-transmission: ARD = 1 ms, ARC = 5 retries.
    config = NRF24_SETUP_RETR_ARD_1000us;
    config |= NRF24_SETUP_RETR_ARC_5;
    if (!WriteReg(NRF24_REG_SETUP_RETR, config)) {
        return false;
    }

    // Configuration 8:
    // - Set TX address to send to.
    if (!WriteRegArray(
            NRF24_REG_TX_ADDR, kAddressBytes, kAddressLength)) {
        return false;
    }

    // Configuration 9:
    // - Set RX pipe 0 address to match TX target so ACKs can return
    // on pipe 0.
    if (!WriteRegArray(
            NRF24_REG_RX_ADDR_P0, kAddressBytes, kAddressLength)) {
        return false;
    }

    // Configuration 10:
    // - Configure only pipe 0 payload width; leave other pipes
    // disabled.
    if (!WriteReg(NRF24_REG_RX_PW_P0, 0x01U) ||
        !WriteReg(NRF24_REG_RX_PW_P1, 0x00U) ||
        !WriteReg(NRF24_REG_RX_PW_P2, 0x00U) ||
        !WriteReg(NRF24_REG_RX_PW_P3, 0x00U) ||
        !WriteReg(NRF24_REG_RX_PW_P4, 0x00U) ||
        !WriteReg(NRF24_REG_RX_PW_P5, 0x00U)) {
        return false;
    }

    // Configuration 11:
    // - Build CONFIG register: CRC enabled, 1-byte CRC, PTX/PRX role,
    // interrupts unmasked.
    config = 0x00U;
    config |= NRF24_CONFIG_EN_CRC;
    if (primary_role == PrimaryRole::Prx) {
        config |= NRF24_CONFIG_PRIM_RX;
    } else {
        config &= static_cast<uint8_t>(~NRF24_CONFIG_PRIM_RX);
    }
    config &= ~NRF24_CONFIG_CRCO;
    if (!WriteReg(NRF24_REG_CONFIG, config)) {
        return false;
    }

    // Action 4:
    // - Power up, go to Standby-I mode.
    config |= NRF24_CONFIG_PWR_UP;
    if (!WriteReg(NRF24_REG_CONFIG, config)) {
        return false;
    }

    // This must block the task
    HAL_Delay(1500);
    return true;
}

bool Nrf24l01p::Send(const uint8_t *const data,
                     const uint8_t length) {

    // Enter Standby-II mode
    HAL_GPIO_WritePin(ce_pin_.port, ce_pin_.pin, GPIO_PIN_SET);

    // Write the data to the FIFO
    if (!nrf24_command_array(NRF24_CMD_W_TX_PAYLOAD, data, length)) {
        return false;
    }

    return true;
}

bool Nrf24l01p::Receive(uint8_t *const data, const uint8_t length) {
    return true;
}

// Private methods

bool Nrf24l01p::WriteReg(const uint8_t reg, const uint8_t value) {
    return nrf24_write_reg(reg, value);
}

bool Nrf24l01p::WriteRegArray(const uint8_t reg,
                              const uint8_t *const value,
                              const uint8_t length) {
    return nrf24_write_reg_array(reg, value, length);
}

bool Nrf24l01p::Command(const uint8_t value) {
    return nrf24_command(value);
}

uint8_t Nrf24l01p::ReadStatus() { return nrf24_read_status(); }

bool Nrf24l01p::ToSetupAddressWidth(const uint8_t length,
                                    uint8_t &setup_aw_value) {
    switch (length) {
    case 3U:
        setup_aw_value = NRF24_SETUP_AW_3;
        return true;
    case 4U:
        setup_aw_value = NRF24_SETUP_AW_4;
        return true;
    case 5U:
        setup_aw_value = NRF24_SETUP_AW_5;
        return true;
    default:
        return false;
    }
}
