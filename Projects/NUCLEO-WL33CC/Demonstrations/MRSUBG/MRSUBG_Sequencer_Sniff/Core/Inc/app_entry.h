/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.h
  * @author  MCD Application Team
  * @brief   Interface to the application
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
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ---------------------------------------------*/
PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck(void);
void MX_APPE_Config(void);
uint32_t MX_APPE_Init(void *p_param);
void MX_APPE_Process(void);
void MX_APPE_Idle(void);

/* USER CODE BEGIN EF */
void MR_SUBG_FAST_RX_TERM_IRQHandler(void);
void MR_SUBG_RX_TIMEOUT_IRQHandler(void);
void MR_SUBG_RX_OK_IRQHandler(void);
void MR_SUBG_RX_CRC_ERROR_IRQHandler(void);
void MR_SUBG_HW_ANA_FAILURE_IRQHandler(void);
void MR_SUBG_SEQ_IRQHandler(void);
/* USER CODE END EF */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_ENTRY_H */
