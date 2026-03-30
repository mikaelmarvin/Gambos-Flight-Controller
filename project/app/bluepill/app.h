/**
 * @file app.h
 * @brief Application layer – Blue Pill (STM32F103 + FreeRTOS).
 */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void);
void app_run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
