/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32wl3x_hal_msp.c
  * @author  MCD Application Team
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
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
static void lowPowerIOSetup(void);
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

  /* IOs configuration to have the lowest power consumption in DEEPSTOP */
  lowPowerIOSetup();

  /* USER CODE END MspInit 0 */

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief MRSubG MSP Initialization
  * This function configures the hardware resources used in this example
  * @retval None
  */
void HAL_MRSubG_MspInit(void)
{
  
  /* USER CODE BEGIN MRSubG_MspInit 0 */
  
  /* USER CODE END MRSubG_MspInit 0 */
  
  /* Peripheral clock enable */
  if (__HAL_RCC_MRSUBG_IS_CLK_DISABLED())
  {
    /* MRSUBG Peripheral reset */
    __HAL_RCC_MRSUBG_FORCE_RESET();
    __HAL_RCC_MRSUBG_RELEASE_RESET();
    
    /* Enable MRSUBG peripheral clock */
    __HAL_RCC_MRSUBG_CLK_ENABLE();
  }
  
  /* USER CODE BEGIN MRSubG_MspInit 1 */
  
  /* USER CODE END MRSubG_MspInit 1 */
}

/**
  * @brief MRSubG MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @retval None
  */
void HAL_MRSubG_MspDeInit(void)
{
  /* USER CODE BEGIN MRSubG_MspDeInit 0 */
  
  /* USER CODE END MRSubG_MspDeInit 0 */
  
  /* Peripheral clock disable */
  __HAL_RCC_MRSUBG_CLK_DISABLE();
  __HAL_RCC_MRSUBG_FORCE_RESET();
  __HAL_RCC_MRSUBG_RELEASE_RESET();
  
  /* USER CODE BEGIN MRSubG_MspDeInit 1 */
  
  /* USER CODE END MRSubG_MspDeInit 1 */

}

/* USER CODE BEGIN 1 */

/**
  * @brief  Configures the IO to have the optimized power consumption.
  * @param  None
  * @retval None
  */
static void lowPowerIOSetup(void)
{
  HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A,                              
                       PWR_GPIO_BIT_4|  
                       PWR_GPIO_BIT_5|  
                       PWR_GPIO_BIT_6|                                
                       PWR_GPIO_BIT_12|
                       PWR_GPIO_BIT_13|
                       PWR_GPIO_BIT_14|
                       PWR_GPIO_BIT_15);

  HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B,
                       PWR_GPIO_BIT_3|
                       PWR_GPIO_BIT_4|
                       PWR_GPIO_BIT_5|
                       PWR_GPIO_BIT_8|
                       PWR_GPIO_BIT_9|
                       PWR_GPIO_BIT_10|
                       PWR_GPIO_BIT_11);
}

/* USER CODE END 1 */

