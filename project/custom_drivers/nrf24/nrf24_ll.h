#ifndef NRF24_LL_H
#define NRF24_LL_H

#include <stdint.h>
#include <stdbool.h>

uint8_t nrf24_ll_read_reg(uint8_t reg);
bool    nrf24_ll_write_reg(uint8_t reg, uint8_t value);
uint8_t nrf24_ll_get_status(void);

#endif /* NRF24_LL_H */
