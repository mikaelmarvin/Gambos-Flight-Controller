/**
 * @file log.h
 * @brief Timestamped logging (HAL_GetTick() ms since boot).
 *
 * Use LOG("fmt", ...) for messages with a leading [  ms] stamp.
 * Plain printf() does not add a timestamp.
 */

#ifndef LOG_H
#define LOG_H

void log_printf(const char *fmt, ...);

/** Log one line with "[%10lu ms] " prefix (GCC variadic). */
#define LOG(fmt, ...) log_printf((fmt), ##__VA_ARGS__)

#endif /* LOG_H */
