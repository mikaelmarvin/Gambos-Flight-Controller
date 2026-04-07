/**
 * @file retarget_print.c
 * @brief Blocking printf/puts: newlib _write() -> syscalls __io_putchar() -> USART2.
 */

#include "main.h"
#include "usart.h"

int __io_putchar(int ch) {
    uint8_t c = (uint8_t)ch;
    (void)HAL_UART_Transmit(&huart2, &c, 1U, HAL_MAX_DELAY);
    return ch;
}
