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
#define INPUT_A_Pin GPIO_PIN_0
#define INPUT_A_GPIO_Port GPIOC
#define INPUT_A_EXTI_IRQn EXTI0_IRQn
#define INPUT_B_Pin GPIO_PIN_1
#define INPUT_B_GPIO_Port GPIOC
#define INPUT_B_EXTI_IRQn EXTI1_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define RADIO_SCK_Pin GPIO_PIN_10
#define RADIO_SCK_GPIO_Port GPIOB
#define RADIO_NSS_Pin GPIO_PIN_12
#define RADIO_NSS_GPIO_Port GPIOB
#define INPUT_LEFT_Pin GPIO_PIN_13
#define INPUT_LEFT_GPIO_Port GPIOB
#define INPUT_LEFT_EXTI_IRQn EXTI15_10_IRQn
#define RADIO_MISO_Pin GPIO_PIN_14
#define RADIO_MISO_GPIO_Port GPIOB
#define RADIO_MOSI_Pin GPIO_PIN_15
#define RADIO_MOSI_GPIO_Port GPIOB
#define INPUT_DOWN_Pin GPIO_PIN_9
#define INPUT_DOWN_GPIO_Port GPIOA
#define INPUT_DOWN_EXTI_IRQn EXTI9_5_IRQn
#define INPUT_UP_Pin GPIO_PIN_10
#define INPUT_UP_GPIO_Port GPIOA
#define INPUT_UP_EXTI_IRQn EXTI15_10_IRQn
#define RADIO_DIO0_Pin GPIO_PIN_11
#define RADIO_DIO0_GPIO_Port GPIOA
#define RADIO_RST_Pin GPIO_PIN_12
#define RADIO_RST_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define INPUT_RIGHT_Pin GPIO_PIN_5
#define INPUT_RIGHT_GPIO_Port GPIOB
#define INPUT_RIGHT_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
