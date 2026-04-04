#include "nrf24.h"

#include "nrf24_ll.h"
#include "nrf24_reg.h"

bool nrf24_init(void)
{
  uint8_t cfg = nrf24_ll_read_reg(NRF24_REG_CONFIG);
  (void)nrf24_ll_get_status();

  /* POR default CONFIG is often 0x08 — if MISO stuck / no chip, reads may be 0xFF */
  if (cfg == 0xFFU) {
    return false;
  }

  /* Example: power up, stay in PTX; refine in device layer */
  cfg |= (uint8_t)(NRF24_CONFIG_PWR_UP | NRF24_CONFIG_EN_CRC);
  cfg &= (uint8_t)~NRF24_CONFIG_PRIM_RX;
  if (!nrf24_ll_write_reg(NRF24_REG_CONFIG, cfg)) {
    return false;
  }

  return nrf24_ll_read_reg(NRF24_REG_CONFIG) == cfg;
}
