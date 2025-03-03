/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_wmbus.h
 * @author  MCD Application Team
 * @brief   Header of application of the WmBus Phy Middleware
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
#ifndef __APP_WMBUS_H__
#define __APP_WMBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported Functions Prototypes ---------------------------------------------*/
/**
 * @brief  Init wmbphy Application
 */
void MX_wmbphy_Init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format);

void MX_wmbphy_Process(void);
void MX_wmbphy_Idle(void);

PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__APP_WMBUS_H__*/
