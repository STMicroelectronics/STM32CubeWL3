/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    hw_mrsubg.c
  * @author  GPM WBL Application Team
  * @brief   This file provides functions implementation for MRSUBG Manager
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
#include "stm32wl3x_hal.h"
#include "hw_mrsubg.h"

/** @defgroup MRSUBG_Manager_STM32WL3  MRSUBG Manager
* @{
*/

/** @defgroup MRSUBG_Manager_STM32WL3_TypesDefinitions Private Type Definitions
* @{
*/
/**
* @}
*/

/** @defgroup MRSUBG_Manager_STM32WL3_Private_Defines Private Defines
* @{
*/
#define ATOMIC_SECTION_BEGIN() uint32_t uwPRIMASK_Bit = __get_PRIMASK(); \
__disable_irq(); \
  /* Must be called in the same or in a lower scope of ATOMIC_SECTION_BEGIN */
#define ATOMIC_SECTION_END() __set_PRIMASK(uwPRIMASK_Bit)

/**
* @}
*/
  
/** @defgroup MRSUBG_Manager_STM32WL3_Private_Variables Private Variables
* @{
*/
/**
* @}
*/

/** @defgroup MRSUBG_Manager_STM32WL3_External_Variables External Variables
* @{
*/
/**
* @}
*/


/** @defgroup MRSUBG_Manager_STM32WL3_Public_Functions Public Functions
* @{
*/

HW_MRSUBG_ResultStatus HW_MRSUBG_Init(void)
{
  if (__HAL_RCC_MRSUBG_IS_CLK_DISABLED())
  {
    /* Radio Peripheral reset */
    __HAL_RCC_MRSUBG_FORCE_RESET();
    __HAL_RCC_MRSUBG_RELEASE_RESET();
    
    /* Enable Radio peripheral clock */
    __HAL_RCC_MRSUBG_CLK_ENABLE();
  }
  return HW_MRSUBG_SUCCESS;
}

HW_MRSUBG_ResultStatus HW_MRSUBG_Deinit(void)
{
  return HW_MRSUBG_SUCCESS;
}
/**
* @}
*/


/** @defgroup MRSUBG_Manager_STM32WL3_Private_Functions Private Functions
* @{
*/

/**
* @}
*/

/**
* @}
*/
