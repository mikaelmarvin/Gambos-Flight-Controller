/**
 * @file nrf24_reg.h
 * @brief nRF24L01+ register map and bit masks (Nordic datasheet §9).
 *
 * Only the main radio; optional features (DYNPD/FEATURE) included for + variant.
 */

#ifndef NRF24_REG_H
#define NRF24_REG_H

#include <stdint.h>

/* Register addresses (5-bit, used with R_REG/W_REG) */
#define NRF24_REG_CONFIG 0x00U
#define NRF24_REG_EN_AA 0x01U
#define NRF24_REG_EN_RXADDR 0x02U
#define NRF24_REG_SETUP_AW 0x03U
#define NRF24_REG_SETUP_RETR 0x04U
#define NRF24_REG_RF_CH 0x05U
#define NRF24_REG_RF_SETUP 0x06U
#define NRF24_REG_STATUS 0x07U
#define NRF24_REG_OBSERVE_TX 0x08U
#define NRF24_REG_RPD 0x09U /* Carrier Detect / Received Power Detector */
#define NRF24_REG_RX_ADDR_P0 0x0AU
#define NRF24_REG_RX_ADDR_P1 0x0BU
#define NRF24_REG_RX_ADDR_P2 0x0CU /* LSByte only */
#define NRF24_REG_RX_ADDR_P3 0x0DU
#define NRF24_REG_RX_ADDR_P4 0x0EU
#define NRF24_REG_RX_ADDR_P5 0x0FU
#define NRF24_REG_TX_ADDR 0x10U
#define NRF24_REG_RX_PW_P0 0x11U
#define NRF24_REG_RX_PW_P1 0x12U
#define NRF24_REG_RX_PW_P2 0x13U
#define NRF24_REG_RX_PW_P3 0x14U
#define NRF24_REG_RX_PW_P4 0x15U
#define NRF24_REG_RX_PW_P5 0x16U
#define NRF24_REG_FIFO_STATUS 0x17U
#define NRF24_REG_DYNPD 0x1CU
#define NRF24_REG_FEATURE 0x1DU

/* --- CONFIG (0x00) --- */
#define NRF24_CONFIG_MASK_RX_DR (1U << 6)
#define NRF24_CONFIG_MASK_TX_DS (1U << 5)
#define NRF24_CONFIG_MASK_MAX_RT (1U << 4)
#define NRF24_CONFIG_EN_CRC (1U << 3)
#define NRF24_CONFIG_CRCO (1U << 2) /* 0 = 1 byte, 1 = 2 bytes */
#define NRF24_CONFIG_PWR_UP (1U << 1)
#define NRF24_CONFIG_PRIM_RX (1U << 0) /* 1 = PRX, 0 = PTX */

/* --- EN_AA (0x01) — auto-ack per pipe --- */
#define NRF24_ENAA_P5 (1U << 5)
#define NRF24_ENAA_P4 (1U << 4)
#define NRF24_ENAA_P3 (1U << 3)
#define NRF24_ENAA_P2 (1U << 2)
#define NRF24_ENAA_P1 (1U << 1)
#define NRF24_ENAA_P0 (1U << 0)

/* --- EN_RXADDR (0x02) — enable data pipes --- */
#define NRF24_ERX_P5 (1U << 5)
#define NRF24_ERX_P4 (1U << 4)
#define NRF24_ERX_P3 (1U << 3)
#define NRF24_ERX_P2 (1U << 2)
#define NRF24_ERX_P1 (1U << 1)
#define NRF24_ERX_P0 (1U << 0)

/* --- SETUP_AW (0x03) — address width (2 bits) --- */
#define NRF24_SETUP_AW_MASK 0x03U
#define NRF24_SETUP_AW_3 0x01U
#define NRF24_SETUP_AW_4 0x02U
#define NRF24_SETUP_AW_5 0x03U

/* --- SETUP_RETR (0x04) --- */
#define NRF24_SETUP_RETR_ARD_1000us 0x03U
#define NRF24_SETUP_RETR_ARC_5 0x05U

/* --- RF_CH (0x05) — channel 0–127 (usable band depends on region) --- */
#define NRF24_RF_CH_MASK 0x7FU

/* --- RF_SETUP (0x06) — see datasheet Table 28 --- */
#define NRF24_RF_SETUP_CONT_WAVE (1U << 7)
#define NRF24_RF_SETUP_RF_DR_LOW (1U << 5) /* with RF_DR: 250 kbps mode */
#define NRF24_RF_SETUP_RF_DR (1U << 3)     /* 0 = 1 Mbps, 1 = 2 Mbps (when RF_DR_LOW=0) */
#define NRF24_RF_SETUP_RF_PWR_SHIFT 1U
#define NRF24_RF_SETUP_RF_PWR_0dBm 0x06U
#define NRF24_RF_SETUP_RF_PWR_6dBm 0x04U
#define NRF24_RF_SETUP_RF_PWR_12dBm 0x02U
#define NRF24_RF_SETUP_RF_PWR_MASK 0x00U
#define NRF24_RF_SETUP_LNA_HCURR (1U << 0)

/* --- STATUS (0x07) — also returned on any SPI write; NOP reads it --- */
#define NRF24_STATUS_RX_DR (1U << 6)
#define NRF24_STATUS_TX_DS (1U << 5)
#define NRF24_STATUS_MAX_RT (1U << 4)
#define NRF24_STATUS_RX_P_NO_MASK 0x0EU
#define NRF24_STATUS_TX_FULL (1U << 0)

/* --- FIFO_STATUS (0x17) --- */
#define NRF24_FIFO_TX_REUSE (1U << 6)
#define NRF24_FIFO_TX_FULL (1U << 5)
#define NRF24_FIFO_TX_EMPTY (1U << 4)
#define NRF24_FIFO_RX_FULL (1U << 1)
#define NRF24_FIFO_RX_EMPTY (1U << 0)

/* --- DYNPD (0x1C) --- */
#define NRF24_DYNPD_DPL_P5 (1U << 5)
#define NRF24_DYNPD_DPL_P4 (1U << 4)
#define NRF24_DYNPD_DPL_P3 (1U << 3)
#define NRF24_DYNPD_DPL_P2 (1U << 2)
#define NRF24_DYNPD_DPL_P1 (1U << 1)
#define NRF24_DYNPD_DPL_P0 (1U << 0)

/* --- FEATURE (0x1D) --- */
#define NRF24_FEATURE_EN_DPL (1U << 2) /* dynamic payload length */
#define NRF24_FEATURE_EN_ACK_PAY (1U << 1)
#define NRF24_FEATURE_EN_DYN_ACK (1U << 0) /* W_TX_PAYLOAD_NO_ACK */

#endif /* NRF24_REG_H */
