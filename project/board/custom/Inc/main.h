/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_BTN_Pin GPIO_PIN_0
#define USER_BTN_GPIO_Port GPIOC
#define RF_IRQ_Pin GPIO_PIN_0
#define RF_IRQ_GPIO_Port GPIOA
#define RF_CS_Pin GPIO_PIN_1
#define RF_CS_GPIO_Port GPIOA
#define RF_CE_Pin GPIO_PIN_2
#define RF_CE_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_3
#define FLASH_CS_GPIO_Port GPIOA
#define ESC_Pin GPIO_PIN_6
#define ESC_GPIO_Port GPIOC
#define SERVO5_Pin GPIO_PIN_7
#define SERVO5_GPIO_Port GPIOC
#define SERVO4_Pin GPIO_PIN_8
#define SERVO4_GPIO_Port GPIOC
#define SERVO3_Pin GPIO_PIN_9
#define SERVO3_GPIO_Port GPIOC
#define SERVO2_Pin GPIO_PIN_8
#define SERVO2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_9
#define LED3_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_10
#define LED2_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_11
#define LED1_GPIO_Port GPIOA
#define BUFF_EN_Pin GPIO_PIN_12
#define BUFF_EN_GPIO_Port GPIOA
#define SERVO1_Pin GPIO_PIN_15
#define SERVO1_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
