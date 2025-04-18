/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    DAC/DAC_SignalsGeneration/Src/stm32wl3x_hal_msp.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
extern DMA_HandleTypeDef hdma_dac_out1;

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
  * @brief DAC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hdac: DAC handle pointer
  * @retval None
  */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdac->Instance==DAC1)
  {
    /* USER CODE BEGIN DAC1_MspInit 0 */
  
    /* USER CODE END DAC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DAC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC1 GPIO Configuration
    PA13     ------> DAC1_OUT1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_PWREx_DisableGPIOPullUp(PWR_GPIO_A, PWR_GPIO_BIT_13);

    HAL_PWREx_DisableGPIOPullDown(PWR_GPIO_A, PWR_GPIO_BIT_13);

    /* DAC1 DMA Init */
    /* DAC_OUT1 Init */
    hdma_dac_out1.Instance = DMA1_Channel1;
    hdma_dac_out1.Init.Request = DMAMUX_REQ_DAC;
    hdma_dac_out1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac_out1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac_out1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac_out1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_dac_out1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_dac_out1.Init.Mode = DMA_CIRCULAR;
    hdma_dac_out1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_dac_out1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hdac,DMA_Handle1,hdma_dac_out1);

    /* USER CODE BEGIN DAC1_MspInit 1 */
  
    /* USER CODE END DAC1_MspInit 1 */

  }

}

/**
  * @brief DAC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hdac: DAC handle pointer
  * @retval None
  */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC1)
  {
    /* USER CODE BEGIN DAC1_MspDeInit 0 */

    /* USER CODE END DAC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC_CLK_DISABLE();

    /**DAC1 GPIO Configuration
    PA13     ------> DAC1_OUT1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13);

    /* DAC1 DMA DeInit */
    HAL_DMA_DeInit(hdac->DMA_Handle1);
    /* USER CODE BEGIN DAC1_MspDeInit 1 */

    /* USER CODE END DAC1_MspDeInit 1 */
  }

}

/**
  * @brief TIM_Base MSP Initialization
  * This function configures the hardware resources used in this example
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM16)
  {
    /* USER CODE BEGIN TIM16_MspInit 0 */

    /* USER CODE END TIM16_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM16_CLK_ENABLE();
    /* USER CODE BEGIN TIM16_MspInit 1 */

    /* USER CODE END TIM16_MspInit 1 */

  }

}

/**
  * @brief TIM_Base MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM16)
  {
    /* USER CODE BEGIN TIM16_MspDeInit 0 */

    /* USER CODE END TIM16_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM16_CLK_DISABLE();
    /* USER CODE BEGIN TIM16_MspDeInit 1 */

    /* USER CODE END TIM16_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
