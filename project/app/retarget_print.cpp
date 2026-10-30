/**
 * @file retarget_print.cpp
 * @brief Blocking printf/puts: newlib _write() -> syscalls __io_putchar() -> UART5.
 */

#include "main.h"
#include "usart.h"

extern "C" int __io_putchar(int ch)
{
	uint8_t c = (uint8_t)ch;
	(void)HAL_UART_Transmit(&huart5, &c, 1U, HAL_MAX_DELAY);
	return ch;
}
