#include "nrf24.h"

#include "nrf24_ll.h"
#include "nrf24_reg.h"

bool nrf24_init(const nrf24_primary_role_t primary_role, const nrf24_address_t send_to) {

    // Power down first, go to Power Down mode
    uint8_t config = 0x00;
    nrf24_write_reg(NRF24_REG_CONFIG, config);

    // Flush TX and RX FIFOs
    nrf24_command(NRF24_CMD_FLUSH_TX);
    nrf24_command(NRF24_CMD_FLUSH_RX);

    // --- Program the radio configuration: ---
    // Address width 3 bytes
    nrf24_write_reg(NRF24_REG_SETUP_AW, NRF24_SETUP_AW_3);

    // RF channel: 0th channel = 2.400 GHz + 0 MHz
    nrf24_write_reg(NRF24_REG_RF_CH, 0x00);

    // RF setup: 250 kbps mode, 0 dBm output power
    config = NRF24_RF_SETUP_RF_DR_LOW;
    config |= NRF24_RF_SETUP_RF_PWR_0dBm;
    nrf24_write_reg(NRF24_REG_RF_SETUP, config);

    // Enable dynamic payload length and disable payload with ACK and disable noACK command
    // (NRF24_CMD_W_TX_PAYLOAD_NO_ACK)
    nrf24_write_reg(NRF24_REG_FEATURE, NRF24_FEATURE_EN_DPL);

    // Enable dynamic payload length for pipe 0
    nrf24_write_reg(NRF24_REG_DYNPD, 0x01);

    // Enable auto-ack only for pipe 0
    nrf24_write_reg(NRF24_REG_EN_AA, NRF24_ENAA_P0);

    // Enable RX data pipe 0
    nrf24_write_reg(NRF24_REG_EN_RXADDR, NRF24_ERX_P0);

    // Automatic re-transmission: ARD = 1 ms, ARC = 5 retries
    config = NRF24_SETUP_RETR_ARD_1000us;
    config |= NRF24_SETUP_RETR_ARC_5;
    nrf24_write_reg(NRF24_REG_SETUP_RETR, config);

    // set TX address to send to
    nrf24_write_reg_array(NRF24_REG_TX_ADDR, send_to.address, send_to.length);

    // Set RX address for pipe 0, same as TX address, we expect to receive the ACK from the address
    // we are sending data to, which is the address TX address is pointing to
    nrf24_write_reg_array(NRF24_REG_RX_ADDR_P0, send_to.address, send_to.length);

    // Set Number of bytes in RX payload in data pipe 0 (1 to 32 bytes); 1 byte payload. Other pipes
    // are not used, so set to 0.
    nrf24_write_reg(NRF24_REG_RX_PW_P0, 0x01);
    nrf24_write_reg(NRF24_REG_RX_PW_P1, 0x00);
    nrf24_write_reg(NRF24_REG_RX_PW_P2, 0x00);
    nrf24_write_reg(NRF24_REG_RX_PW_P3, 0x00);
    nrf24_write_reg(NRF24_REG_RX_PW_P4, 0x00);
    nrf24_write_reg(NRF24_REG_RX_PW_P5, 0x00);

    // --- Clear interrupt flags in the STATUS register ---
    uint8_t status = nrf24_read_status();
    status |= NRF24_STATUS_RX_DR | NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT;
    nrf24_write_reg(NRF24_REG_STATUS, status);

    // --- Build the CONFIG register: ---
    // Enable CRC, 1 byte CRC encoding scheme, do not mask any interrupt,
    config = 0x00;
    config |= NRF24_CONFIG_EN_CRC;
    config &= ~NRF24_CONFIG_CRCO;
    nrf24_write_reg(NRF24_REG_CONFIG, config);

    // --- Set primary role ---
    config |= primary_role;
    nrf24_write_reg(NRF24_REG_CONFIG, config);

    // --- Power up, go to Standby-I mode ---
    config |= NRF24_CONFIG_PWR_UP;
    nrf24_write_reg(NRF24_REG_CONFIG, config);

    // TODO: Wait for power up at least 1.5 ms

    return true;
}
