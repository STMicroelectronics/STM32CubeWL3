/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples/ADC/ADC_MultiChannelSingleConversion/Src/stm32wl3x_hal_msp.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.
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
extern DMA_HandleTypeDef hdma_adc1_ds;

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
  * @brief ADC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
    /* USER CODE BEGIN ADC1_MspInit 0 */
   __HAL_RCC_ADCDIG_CLK_ENABLE();      
   __HAL_RCC_ADCANA_CLK_ENABLE();
    /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADCDIG_CLK_ENABLE();
    __HAL_RCC_ADCANA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB3     ------> ADC1_VINP0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_PWREx_DisableGPIOPullUp(PWR_GPIO_B, PWR_GPIO_BIT_3);

    HAL_PWREx_DisableGPIOPullDown(PWR_GPIO_B, PWR_GPIO_BIT_3);

    /* ADC1 DMA Init */
    /* ADC1_DS Init */
    hdma_adc1_ds.Instance = DMA1_Channel1;
    hdma_adc1_ds.Init.Request = DMA_REQUEST_ADC1_DS;
    hdma_adc1_ds.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1_ds.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1_ds.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1_ds.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1_ds.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1_ds.Init.Mode = DMA_CIRCULAR;
    hdma_adc1_ds.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1_ds) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1_ds);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
    /* USER CODE BEGIN ADC1_MspInit 1 */
    /* USER CODE END ADC1_MspInit 1 */

  }

}

/**
  * @brief ADC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */
 __HAL_RCC_ADCDIG_CLK_DISABLE();         
 __HAL_RCC_ADCANA_CLK_DISABLE();
    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADCDIG_CLK_DISABLE();
    __HAL_RCC_ADCANA_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB3     ------> ADC1_VINP0
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);

    /* ADC1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(ADC_IRQn);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */
    /* USER CODE END ADC1_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */
/* USER CODE END 1 */
