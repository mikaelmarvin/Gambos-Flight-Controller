#include "nrf24l01p.hpp"
#include "nrf24l01p_hw.h"

#include <cstring>

#include "stm32f4xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

namespace {

constexpr uint8_t kAddressLength = 5U;
constexpr TickType_t kSpiDmaTimeoutMs = 100U;

StaticSemaphore_t spi_dma_semaphore_buffer;
SemaphoreHandle_t spi_dma_semaphore = nullptr;

/** HAL_SPI_TxRxCpltCallback is one global for the firmware; hspi
 * tells which bus finished, not which driver started the transfer.
 * Set while NrfSpiExchange waits so SPI1 (flash/SD) completions do
 * not wake nRF. */
SPI_HandleTypeDef *spi_dma_owner = nullptr;

bool EnsureSpiDmaSemaphore(void) {
    if (spi_dma_semaphore == nullptr) {
        spi_dma_semaphore =
            xSemaphoreCreateBinaryStatic(&spi_dma_semaphore_buffer);
    }
    return spi_dma_semaphore != nullptr;
}

void SignalSpiDmaCompleteFromIsr(void) {
    if (spi_dma_semaphore == nullptr) {
        return;
    }
    BaseType_t hpw = pdFALSE;
    (void)xSemaphoreGiveFromISR(spi_dma_semaphore, &hpw);
    portYIELD_FROM_ISR(hpw);
}

// Default ShockBurst address (same for TX and RX)
constexpr uint8_t kAddressBytes[5U] = {
    0xE7U,
    0xE7U,
    0xE7U,
    0xE7U,
    0xE7U,
};

} // namespace

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (spi_dma_owner != nullptr && hspi == spi_dma_owner) {
        SignalSpiDmaCompleteFromIsr();
    }
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    if (spi_dma_owner != nullptr && hspi == spi_dma_owner) {
        SignalSpiDmaCompleteFromIsr();
    }
}

bool Nrf24l01p::RegisterCePin(GPIO_TypeDef *port, uint16_t pin) {
    _ce_pin_.port = port;
    _ce_pin_.pin = pin;
    return (port != nullptr);
}

bool Nrf24l01p::RegisterCsnPin(GPIO_TypeDef *port, uint16_t pin) {
    _csn_pin_.port = port;
    _csn_pin_.pin = pin;
    return (port != nullptr);
}

bool Nrf24l01p::Init(SPI_HandleTypeDef *spi,
                     const PrimaryRole primary_role) {
    _spi = spi;
    if (_spi == nullptr) {
        return false;
    }
    if (!EnsureSpiDmaSemaphore()) {
        return false;
    }

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

    // --- Program the radio configuration ---
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
    if (!WriteReg(NRF24_REG_RX_PW_P0, _payload_length) ||
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
        config &= ~NRF24_CONFIG_PRIM_RX;
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

    // If the device is RX, keep CE high permanently
    if (primary_role == PrimaryRole::Prx) {
        HAL_GPIO_WritePin(_ce_pin_.port, _ce_pin_.pin, GPIO_PIN_SET);
    }

    // This must block the task for 2 ms
    vTaskDelay(pdMS_TO_TICKS(2));
    return true;
}

bool Nrf24l01p::Send(const uint8_t *data) {
    HAL_GPIO_WritePin(_ce_pin_.port, _ce_pin_.pin, GPIO_PIN_SET);
    const bool ok =
        WriteRegArray(NRF24_CMD_W_TX_PAYLOAD, data, _payload_length);
    HAL_GPIO_WritePin(_ce_pin_.port, _ce_pin_.pin, GPIO_PIN_RESET);
    return ok;
}

bool Nrf24l01p::Receive(uint8_t *data) {
    return WriteReadRegArray(
        NRF24_CMD_R_RX_PAYLOAD, data, _payload_length);
}

bool Nrf24l01p::NrfSpiExchange(SPI_HandleTypeDef *spi,
                               const uint8_t *tx,
                               uint8_t *rx,
                               uint8_t len) {
    if (spi == nullptr || tx == nullptr || rx == nullptr ||
        len == 0U) {
        return false;
    }
    if (spi_dma_semaphore == nullptr || _csn_pin_.port == nullptr) {
        return false;
    }

    while (xSemaphoreTake(spi_dma_semaphore, 0) == pdTRUE) {}

    HAL_GPIO_WritePin(_csn_pin_.port, _csn_pin_.pin, GPIO_PIN_RESET);

    spi_dma_owner = spi;
    const bool started =
        HAL_SPI_TransmitReceive_DMA(spi, tx, rx, len) == HAL_OK;
    if (!started) {
        spi_dma_owner = nullptr;
        HAL_GPIO_WritePin(
            _csn_pin_.port, _csn_pin_.pin, GPIO_PIN_SET);
        return false;
    }

    const bool completed =
        xSemaphoreTake(spi_dma_semaphore,
                       pdMS_TO_TICKS(kSpiDmaTimeoutMs)) == pdTRUE;
    spi_dma_owner = nullptr;
    HAL_GPIO_WritePin(_csn_pin_.port, _csn_pin_.pin, GPIO_PIN_SET);

    return completed;
}

bool Nrf24l01p::WriteReadRegArray(const uint8_t cmd,
                                  uint8_t *data,
                                  const uint8_t len) {
    if (len > kMaxFifoBytes) {
        return false;
    }

    uint8_t tx[33];
    uint8_t rx[33];
    tx[0] = cmd;
    memset(&tx[1], 0, len);
    if (!NrfSpiExchange(_spi, tx, rx, len + 1U)) {
        return false;
    }
    memcpy(data, &rx[1], len);
    return true;
}

bool Nrf24l01p::WriteReg(const uint8_t reg, const uint8_t value) {
    uint8_t tx[2] = {NRF24_CMD_W_REG(reg), value};
    uint8_t rx[2];
    return NrfSpiExchange(_spi, tx, rx, 2U);
}

bool Nrf24l01p::WriteRegArray(const uint8_t reg,
                              const uint8_t *value,
                              const uint8_t length) {
    if (length > kMaxFifoBytes) {
        return false;
    }
    uint8_t tx[33];
    uint8_t rx[33];
    tx[0] = NRF24_CMD_W_REG(reg);
    memcpy(&tx[1], value, length);
    return NrfSpiExchange(_spi, tx, rx, length + 1U);
}

bool Nrf24l01p::Command(const uint8_t value) {
    uint8_t tx[1] = {value};
    uint8_t rx[1];
    return NrfSpiExchange(_spi, tx, rx, 1U);
}

uint8_t Nrf24l01p::ReadStatus() {
    uint8_t tx[1] = {NRF24_CMD_NOP};
    uint8_t rx[1];
    (void)NrfSpiExchange(_spi, tx, rx, 1U);
    return rx[0];
}

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
