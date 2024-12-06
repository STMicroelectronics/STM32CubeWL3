/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    hw_mrsubg.h
  * @author  GPM WBL Application Team
  * @brief   This file contains all the functions prototypes for the MRSUBG MANAGER
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
#ifndef HW_MRSUBG_H
#define HW_MRSUBG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
  
/** @addtogroup HW_MRSUBG_Peripheral  MRSUBG MANAGER
 * @{
 */

/** @defgroup HW_MRSUBG_Exported_Types Exported Types
 * @{
 */
	
/* Enumerated values used to report the MRSUBG result status after a process */
typedef enum
{
  HW_MRSUBG_SUCCESS     =  0,
  HW_MRSUBG_ERROR
} HW_MRSUBG_ResultStatus;

/**
 * @}
 */

/** @defgroup HW_MRSUBG_Exported_Constants  Exported Constants
 * @{
 */
/**
 * @}
 */

/** @defgroup HW_MRSUBG_Exported_Macros           Exported Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup HW_MRSUBG_Exported_Functions        Exported Functions
 * @{
 */
HW_MRSUBG_ResultStatus HW_MRSUBG_Init(void);
HW_MRSUBG_ResultStatus HW_MRSUBG_Deinit(void);
/**
  * @}
  */


/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* HW_MRSUBG_H */
