#ifndef NRF24_H
#define NRF24_H

#include <stdbool.h>

#include "nrf24_commands.h"
#include "nrf24_reg.h"

typedef enum { PTX = 0, PRX = 1 } nrf24_primary_role_t;

typedef struct {
    uint8_t address[5];
    uint8_t length;
} nrf24_address_t;

bool nrf24_init(const nrf24_primary_role_t primary_role, const nrf24_address_t address);

#endif /* NRF24_H */
