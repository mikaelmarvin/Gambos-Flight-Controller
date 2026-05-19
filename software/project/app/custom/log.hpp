/**
 * @file log.hpp
 * @brief Timestamped logging (HAL_GetTick() ms since boot).
 *
 * Use LOG("fmt", ...) for messages with a leading [  ms] stamp.
 * Plain printf() does not add a timestamp.
 * Safe to include from C or C++ (macro-only).
 */

#ifndef GAMBOS_LOG_HPP
#define GAMBOS_LOG_HPP

#include <stdio.h>

#include "stm32f4xx_hal.h"

#define LOG(fmt, ...)                                                \
    do {                                                             \
        printf("[%10lu ms] ", (unsigned long)HAL_GetTick());         \
        printf(fmt, ##__VA_ARGS__);                                  \
    } while (0)

#endif /* GAMBOS_LOG_HPP */
