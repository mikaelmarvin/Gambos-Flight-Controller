#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

#include "app/app.hpp"

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void);

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
    /* Create the thread(s) */
    /* creation of defaultTask */
    defaultTaskHandle =
        osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
}

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
void StartDefaultTask(void *argument) {
    (void)argument;
    app_run();
}
