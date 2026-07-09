/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the
 *LICENSE file in the root directory of this software component. If no
 *LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes
 * ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
void MX_GPIO_Init(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA,
                      NRF24_CS_Pin | NRF24_CE_Pin | FLASH_CS_Pin |
                          LED3_Pin | LED2_Pin | LED1_Pin,
                      GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : USR_BTN_Pin (EXTI for button_handler) */
    GPIO_InitStruct.Pin = USR_BTN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USR_BTN_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : NRF24_CS_Pin NRF24_CE_Pin FLASH_CS_Pin
       LED3_Pin LED2_Pin LED1_Pin */
    GPIO_InitStruct.Pin = NRF24_CS_Pin | NRF24_CE_Pin | FLASH_CS_Pin |
                          LED3_Pin | LED2_Pin | LED1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : IRQ_Pin */
    GPIO_InitStruct.Pin = IRQ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IRQ_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : SD_CS_Pin */
    GPIO_InitStruct.Pin = SD_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
