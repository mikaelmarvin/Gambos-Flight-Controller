#ifndef NRF24_LL_H
#define NRF24_LL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Board-specific SPI: assert CSN, full-duplex exchange of @a len
 * bytes, release CSN. Must be provided by the application (or BSP);
 * there is no default — the link will fail with an undefined
 * reference until this is defined.
 *
 * @return true on success, false on error.
 */
bool nrf24_spi_exchange(const uint8_t *tx,
                        uint8_t *rx,
                        const uint8_t len);

uint8_t nrf24_read_reg(const uint8_t reg);
bool nrf24_write_reg(const uint8_t reg, const uint8_t value);
bool nrf24_write_reg_array(const uint8_t reg,
                           const uint8_t *value,
                           const uint8_t length);
bool nrf24_command(const uint8_t command);
bool nrf24_command_array(const uint8_t command,
                         const uint8_t *data,
                         const uint8_t length);
uint8_t nrf24_read_status(void);

#ifdef __cplusplus
}
#endif

#endif /* NRF24_LL_H */
