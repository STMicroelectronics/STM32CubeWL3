/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32wl3x_hal_msp.c
  * @author  GPM WBL Application Team
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
 
/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief LPAWUR MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hlpawur: LPAWUR handle pointer
  * @retval None
  */
void HAL_LPAWUR_MspInit(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    /* USER CODE BEGIN LPAWUR_MspInit 0 */
  
    /* USER CODE END LPAWUR_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC_WDG_SUBG_LPAWUR_LCD_LCSC;
    PeriphClkInitStruct.RTCWDGSUBGLPAWURLCDLCSCClockSelection = RCC_RTC_WDG_SUBG_LPAWUR_LCD_LCSC_CLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    if (__HAL_RCC_LPAWUR_IS_CLK_DISABLED())
    {
      /* LPAWUR Peripheral reset */
      __HAL_RCC_LPAWUR_FORCE_RESET();
      __HAL_RCC_LPAWUR_RELEASE_RESET();

      /* Enable LPAWUR peripheral clock */
      __HAL_RCC_LPAWUR_CLK_ENABLE();
    }

    /* USER CODE BEGIN LPAWUR_MspInit 1 */
  
    /* USER CODE END LPAWUR_MspInit 1 */

}

/**
  * @brief LPAWUR MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hlpawur: LPAWUR handle pointer
  * @retval None
  */
void HAL_LPAWUR_MspDeInit(void)
{
    /* USER CODE BEGIN LPAWUR_MspDeInit 0 */
  
    /* USER CODE END LPAWUR_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPAWUR_CLK_DISABLE();
    __HAL_RCC_LPAWUR_FORCE_RESET();
    __HAL_RCC_LPAWUR_RELEASE_RESET();

    /* USER CODE BEGIN LPAWUR_MspDeInit 1 */
  
    /* USER CODE END LPAWUR_MspDeInit 1 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
