#include "nrf24_ll.h"

#include "nrf24_commands.h"
#include "nrf24_handler.h"
#include "spi.h"
#include "stm32f1xx_hal.h"

#define NRF24_SPI_TIMEOUT_MS 100U

static bool nrf24_ll_spi_exchange(const uint8_t *tx, uint8_t *rx, size_t len)
{
  static bool handler_ready;

  if (len == 0U || len > 65535U) {
    return false;
  }
  if (!handler_ready) {
    nrf24_handler_spi_init();
    handler_ready = true;
  }

  if (HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)tx, rx, (uint16_t)len) != HAL_OK) {
    return false;
  }

  nrf24_handler_spi_wait_dma_done(NRF24_SPI_TIMEOUT_MS);

  if (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY) {
    (void)HAL_SPI_Abort(&hspi1);
    return false;
  }
  return true;
}

uint8_t nrf24_ll_get_status(void)
{
  uint8_t tx[1] = { NRF24_CMD_NOP };
  uint8_t rx[1];
  if (!nrf24_ll_spi_exchange(tx, rx, 1U)) {
    return 0xFFU;
  }
  return rx[0];
}

uint8_t nrf24_ll_read_reg(uint8_t reg)
{
  uint8_t tx[2] = { NRF24_CMD_R_REG(reg), NRF24_CMD_NOP };
  uint8_t rx[2];
  if (!nrf24_ll_spi_exchange(tx, rx, 2U)) {
    return 0xFFU;
  }
  return rx[1];
}

bool nrf24_ll_write_reg(uint8_t reg, uint8_t value)
{
  uint8_t tx[2] = { NRF24_CMD_W_REG(reg), value };
  uint8_t rx[2];
  return nrf24_ll_spi_exchange(tx, rx, 2U);
}
