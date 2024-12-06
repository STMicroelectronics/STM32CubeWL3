/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    hw_mrsubg.h
  * @author  GPM WBL Application Team
  * @brief   This file contains all the functions prototypes for the LPAWUR MANAGER
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
#ifndef HW_LPAWUR_H
#define HW_LPAWUR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
  
/** @addtogroup HW_LPAWUR_Peripheral  LPAWUR MANAGER
 * @{
 */

/** @defgroup HW_LPAWUR_Exported_Types Exported Types
 * @{
 */
	
/* Enumerated values used to report the LPAWUR result status after a process */
typedef enum
{
  HW_LPAWUR_SUCCESS     =  0,
  HW_LPAWUR_ERROR
} HW_LPAWUR_ResultStatus;

/**
 * @}
 */

/** @defgroup HW_LPAWUR_Exported_Constants  Exported Constants
 * @{
 */
/**
 * @}
 */

/** @defgroup HW_LPAWUR_Exported_Macros           Exported Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup HW_LPAWUR_Exported_Functions        Exported Functions
 * @{
 */
HW_LPAWUR_ResultStatus HW_LPAWUR_Init(void);
HW_LPAWUR_ResultStatus HW_LPAWUR_Deinit(void);
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

#endif /* HW_LPAWUR_H */
