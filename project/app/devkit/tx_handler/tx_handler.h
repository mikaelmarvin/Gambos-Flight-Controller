/**
 * @file tx_handler.h
 * @brief TX handler task — create with tx_handler_start() before osKernelStart().
 */

#ifndef TX_HANDLER_H
#define TX_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

void tx_handler_start(void);

#ifdef __cplusplus
}
#endif

#endif /* TX_HANDLER_H */
