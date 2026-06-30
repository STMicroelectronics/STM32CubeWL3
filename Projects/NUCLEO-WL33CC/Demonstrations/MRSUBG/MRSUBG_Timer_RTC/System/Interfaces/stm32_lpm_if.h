/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32_lpm_if.h
  * @author  GPM WBL Application Team
  * @brief   Header for stm32_lpm_f.c module (device specific LP management)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef STM32_TINY_LPM_IF_H
#define STM32_TINY_LPM_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @defgroup TINY_LPM_IF TINY LPM IF
  * @{
  */

/* Exported Definitions ------------------------------------------------------------------*/

/** @defgroup TINY_LPM_IF_Exported_definitions TINY LPM IF Exported definitions
 * @{
 */

/* Exported typedef ---------------------------------------------------------*/
/** @defgroup TINY_LPM_Exported_typedef TINY LPM exported typedef
  * @{
  */
  
/**
 * @brief Enumeration for low power modes.
 *
 * This enumeration defines the various low power modes that the system can enter.
 *
 * @note It must be consistent with UTIL_LPM_Driver array definition
 */
typedef enum
{
  UTIL_LPM_SLEEP_MODE,           /**< Sleep mode */
  UTIL_LPM_DEEPSTOP_LS_MODE,     /**< DeepStop mode with low-speed clock active */
  UTIL_LPM_DEEPSTOP_NOLS_MODE,   /**< DeepStop mode with low-speed clock disabled */
  UTIL_LPM_NUM_MODES             /**< Number of supported modes */
} UTIL_LPM_Mode_t;


  /* Exported Functions ------------------------------------------------------------------*/

/** @defgroup TINY_LPM_IF_Exported_functions TINY LPM IF Exported functions
 * @{
 */

/**
 * @brief Manage the device SLEEP mode.
 *
 * This function configures the system to enter and exit Sleep mode. 
 * Any specific behavior can be controlled by the `param` parameter.
 *
 * @param param Configuration parameter for the SLEEP mode.        
 *
 * @return None
 */
void LPM_SLEEP_Mode(uint32_t param);

/**
 * @brief Manage the device DEEPSTOP_LS mode.
 *
 * This function configures the system to enter and exit the DeepStop mode, 
 * where the low-speed clock is kept active.
 * Any specific behavior can be controlled by the `param` parameter.
 *
 * @param param Configuration parameter for the DEEPSTOP_LS mode.        
 *
 * @return None
 */
void LPM_DEEPSTOP_LS_Mode(uint32_t param);

/**
 * @brief Manage the device DEEPSTOP_NOLS mode.
 *
 * This function configures the system to enter and exit the DeepStop mode, 
 * where the low-speed clock is disabled.
 * Any specific behavior can be controlled by the `param` parameter.
 *
 * @param param Configuration parameter for the DEEPSTOP_NOLS mode.        
 *
 * @return None
 */
void LPM_DEEPSTOP_NOLS_Mode(uint32_t param);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* STM32_TINY_LPM_IF_H */
