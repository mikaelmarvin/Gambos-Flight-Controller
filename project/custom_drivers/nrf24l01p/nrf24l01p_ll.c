#include "nrf24l01p_ll.h"
#include "nrf24l01p_hw.h"
#include <string.h>

uint8_t nrf24_read_reg(const uint8_t reg) {
    // Clocking the first byte gets the STATUS register on MISO, while
    // the next byte gets the gets the register value
    uint8_t tx[2] = {NRF24_CMD_R_REG(reg), NRF24_CMD_NOP};
    uint8_t rx[2];

    nrf24_spi_exchange(tx, rx, 2U);
    // Return only the value of the register
    return rx[1];
}

bool nrf24_write_reg(const uint8_t reg, const uint8_t value) {
    uint8_t tx[2] = {NRF24_CMD_W_REG(reg), value};
    uint8_t rx[2];
    return nrf24_spi_exchange(tx, rx, 2U);
}

bool nrf24_write_reg_array(const uint8_t reg,
                           const uint8_t *value,
                           const uint8_t length) {
    uint8_t tx[length + 1];
    tx[0] = NRF24_CMD_W_REG(reg);
    memcpy(&tx[1], value, length);
    uint8_t rx[length + 1];

    // Must make sure the order is correct in the array the value
    // pointer is pointing to
    return nrf24_spi_exchange(tx, rx, length + 1U);
}

bool nrf24_command(const uint8_t command) {
    uint8_t tx[1] = {command};
    uint8_t rx[1];
    return nrf24_spi_exchange(tx, rx, 1U);
}

bool nrf24_command_array(const uint8_t command,
                         const uint8_t *data,
                         const uint8_t length) {
    uint8_t tx[length + 1];
    tx[0] = command;
    memcpy(&tx[1], data, length);
    uint8_t rx[length + 1];
    return nrf24_spi_exchange(tx, rx, length + 1U);
}

uint8_t nrf24_read_status(void) {
    uint8_t tx[1] = {NRF24_CMD_NOP};
    uint8_t rx[1];
    nrf24_spi_exchange(tx, rx, 1U);
    return rx[0];
}