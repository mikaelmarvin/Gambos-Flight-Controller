/**
 * @file nrf24_commands.h
 * @brief nRF24L01+ SPI command opcodes per Nordic nRF24L01P Product Specification v1.0, Table 20 (§8.3.1).
 *
 * R/W register commands embed a 5-bit register address in the low bits.
 * Use NRF24_CMD_R_REG / NRF24_CMD_W_REG for those.
 *
 * Note: That table does not define a separate ACTIVATE (0x50) SPI command; FEATURE-related
 * commands use footnote (a) in Table 20 — configure FEATURE register as in the PS.
 */

#ifndef NRF24_COMMANDS_H
#define NRF24_COMMANDS_H

#include <stdint.h>

/* Base for register read/write: 000A AAAA / 001A AAAA (5-bit address AAAAA) */
#define NRF24_CMD_R_REG(reg) ((uint8_t)(0x00U | ((uint8_t)(reg)&0x1FU)))
#define NRF24_CMD_W_REG(reg) ((uint8_t)(0x20U | ((uint8_t)(reg)&0x1FU)))

/* FIFO and payload commands (fixed opcodes) */
#define NRF24_CMD_R_RX_PAYLOAD 0x61U
#define NRF24_CMD_W_TX_PAYLOAD 0xA0U
#define NRF24_CMD_FLUSH_TX 0xE1U
#define NRF24_CMD_FLUSH_RX 0xE2U
#define NRF24_CMD_REUSE_TX_PL 0xE3U
#define NRF24_CMD_R_RX_PL_WID 0x60U
#define NRF24_CMD_W_TX_PAYLOAD_NO_ACK 0xB0U
#define NRF24_CMD_NOP 0xFFU

/**
 * ACK payload: opcode 1010 1PPP (PPP = pipe 0–5, Table 20).
 * @param pipe 0–5
 */
#define NRF24_CMD_W_ACK_PAYLOAD(pipe) ((uint8_t)(0xA8U | ((uint8_t)(pipe)&0x07U)))

#endif /* NRF24_COMMANDS_H */
