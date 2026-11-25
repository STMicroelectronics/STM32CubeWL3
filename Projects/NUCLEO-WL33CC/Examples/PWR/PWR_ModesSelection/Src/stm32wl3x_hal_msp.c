/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32wl3x_hal_msp.c
  * @author  MCD Application Team
  * @brief   Global MSP initialization.
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

/* USER CODE BEGIN 1 */

/**
  * @brief  Configures the IOs for current measurement after console shutdown.
  * @param  None
  * @retval None
  */
void lowPowerIOSetup(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  gpio.Pin = GPIO_PIN_ALL;
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &gpio);
  HAL_GPIO_Init(GPIOB, &gpio);

  HAL_PWREx_DisableGPIOPullUp(PWR_GPIO_A, GPIO_PIN_ALL);
  HAL_PWREx_DisableGPIOPullDown(PWR_GPIO_A, GPIO_PIN_ALL);
  HAL_PWREx_DisableGPIOPullUp(PWR_GPIO_B, GPIO_PIN_ALL);
  HAL_PWREx_DisableGPIOPullDown(PWR_GPIO_B, GPIO_PIN_ALL);
  HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_A, MEASUREMENT_PULLUP_A);
  HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, GPIO_PIN_ALL & ~MEASUREMENT_PULLUP_A);
  HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_B, MEASUREMENT_PULLUP_B);
  HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B, GPIO_PIN_ALL & ~MEASUREMENT_PULLUP_B);
  HAL_PWREx_EnablePullUpPullDownConfig();
}

/* USER CODE END 1 */
