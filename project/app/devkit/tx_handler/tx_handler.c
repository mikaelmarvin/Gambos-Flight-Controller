/**
 * @file tx_handler.c
 * @brief nRF24 / TX path — FreeRTOS task (native API).
 */

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "tx_handler/tx_handler.h"

#define TX_HANDLER_TASK_STACK_WORDS 256U
#define TX_HANDLER_TASK_PRIORITY (tskIDLE_PRIORITY + 1U)

static StaticSemaphore_t data_to_send_semaphore_buf;
static SemaphoreHandle_t data_to_send_semaphore;

static void tx_handler_task(void *pv_parameters) {
    (void)pv_parameters;

    for (;;) {

        // Wait for data to send be available
        if (xSemaphoreTake(data_to_send_semaphore, portMAX_DELAY) == pdTRUE) {
            // Send data to nRF24
        }
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

void tx_handler_start(void) {
    data_to_send_semaphore = xSemaphoreCreateBinaryStatic(&data_to_send_semaphore_buf);
    if (data_to_send_semaphore == NULL) {
        return;
    }

    (void)xTaskCreate(tx_handler_task,
                      "tx_handler",
                      TX_HANDLER_TASK_STACK_WORDS,
                      NULL,
                      TX_HANDLER_TASK_PRIORITY,
                      NULL);
}
