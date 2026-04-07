/**
 * @file log.c
 * @brief Timestamped logging implementation.
 */

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

#include "main.h"

void log_printf(const char *fmt, ...) {
    (void)printf("[%10lu ms] ", (unsigned long)HAL_GetTick());
    va_list ap;
    va_start(ap, fmt);
    (void)vprintf(fmt, ap);
    va_end(ap);
}
