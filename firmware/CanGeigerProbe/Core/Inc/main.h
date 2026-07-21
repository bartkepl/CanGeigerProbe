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
#include "stm32l4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define COMP_GM_IN_Pin GPIO_PIN_1
#define COMP_GM_IN_GPIO_Port GPIOA
#define FBACK_CTRL_Pin GPIO_PIN_2
#define FBACK_CTRL_GPIO_Port GPIOA
#define HV_meas_Pin GPIO_PIN_5
#define HV_meas_GPIO_Port GPIOA
#define VIN_SENSE_Pin GPIO_PIN_6
#define VIN_SENSE_GPIO_Port GPIOA
#define I_SENS_Pin GPIO_PIN_7
#define I_SENS_GPIO_Port GPIOA
#define EN_I_SENS_Pin GPIO_PIN_0
#define EN_I_SENS_GPIO_Port GPIOB
#define CAN_STBY_Pin GPIO_PIN_9
#define CAN_STBY_GPIO_Port GPIOA
#define CAN_R_CTRL_Pin GPIO_PIN_10
#define CAN_R_CTRL_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define CAN_RX_IT_Pin GPIO_PIN_15
#define CAN_RX_IT_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
