/**
 * @file app.c
 * @brief Board-specific application; app_init() before scheduler, app_run() from default task.
 */

#include "app.h"
#include "cmsis_os.h"
#include "main.h"

void app_init(void) {
}

void app_run(void) {
    /* PC13 (USER_LED): onboard LED on typical Blue Pill boards (often active-low; toggle still blinks). */
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    osDelay(2500);
}
