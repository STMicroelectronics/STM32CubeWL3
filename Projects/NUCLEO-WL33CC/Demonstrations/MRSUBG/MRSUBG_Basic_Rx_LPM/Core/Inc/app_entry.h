/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.h
  * @author  GPM WBL Application Team
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

#ifndef APP_ENTRY_H
#define APP_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck(void);
uint32_t MX_APPE_Init(void *p_param);
void MX_APPE_Process(void);
void MX_APPE_Idle(void);
void RX_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_ENTRY_H */
