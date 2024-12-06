/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32wl3x_ll_lpuart.h"
#include "stm32wl3x_ll_rcc.h"
#include "stm32wl3x_ll_pwr.h"
#include "stm32wl3x_ll_bus.h"
#include "stm32wl3x_ll_system.h"
#include "stm32wl3x_ll_cortex.h"
#include "stm32wl3x_ll_utils.h"
#include "stm32wl3x_ll_dma.h"
#include "stm32wl3x_ll_dmamux.h"
#include "stm32wl3x_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32_lpm_if.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define LPUART_TX_PORT                                          GPIOB
#define LPUART_TX_PIN                                           LL_GPIO_PIN_3
#define LPUART_TX_AF                                            LL_GPIO_AF_1
#define LL_GPIO_SetAFPin_LPUART_TX()                            LL_GPIO_SetAFPin_0_7(LPUART_TX_PORT, LPUART_TX_PIN, LPUART_TX_AF)
#define LPUART_RX_PORT                                          GPIOA
#define LPUART_RX_PIN                                           LL_GPIO_PIN_5
#define LPUART_RX_AF                                            LL_GPIO_AF_1
#define LL_GPIO_SetAFPin_LPUART_RX()                            LL_GPIO_SetAFPin_0_7(LPUART_RX_PORT, LPUART_RX_PIN, LPUART_RX_AF)
#define LL_EnableClock_LPUART()                                 LL_APB1_EnableClock(LL_APB1_PERIPH_LPUART)
#define LL_EnableClock_LPUART_TX_PORT()                         LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOB)
#define LL_EnableClock_LPUART_RX_PORT()                         LL_AHB_EnableClock(LL_AHB_PERIPH_GPIOA)

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void LPUART_CharReception_Callback(void);
void Error_Callback(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LD1_Pin LL_GPIO_PIN_14
#define LD1_GPIO_Port GPIOA
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
