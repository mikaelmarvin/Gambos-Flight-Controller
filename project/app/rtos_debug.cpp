/**
 * @file rtos_debug.cpp
 * @brief Symbols for RTOS-aware debugging (J-Link FreeRTOS plugin via
 * Cortex-Debug "rtos": "FreeRTOS").
 *
 * This FreeRTOS version no longer defines uxTopUsedPriority, which debug
 * plugins read to know how many ready lists to walk. Nothing references
 * it, so the top-level CMakeLists.txt keeps it alive with
 * -Wl,--undefined=uxTopUsedPriority (--gc-sections would drop it).
 */

#include "FreeRTOS.h"

extern "C" {
extern const volatile UBaseType_t uxTopUsedPriority;
const volatile UBaseType_t uxTopUsedPriority = configMAX_PRIORITIES - 1;
}
