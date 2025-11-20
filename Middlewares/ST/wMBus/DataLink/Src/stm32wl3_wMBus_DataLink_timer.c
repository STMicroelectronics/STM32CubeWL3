/**
  ******************************************************************************
  * @file    stm32wl3_wMBus_DataLink_timer.c
  * @author  MCD Application Team
  * @brief   wM-Bus DataLink module driver.
  *          This file provides firmware functions to manage the wM-Bus DataLink driver
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

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal.h"
#include "stm32wl3_wMBus_DataLink_timer.h"

/** @addtogroup STM32WL3_wM-Bus
  * @{
  */

/** @addtogroup DataLink
  * @{
  */

/** @addtogroup Timer
  * @{
  */

/** @defgroup Timer_Private_Constants Timer Private Constants
  * @{
  */

/**
  * @}
  */

/** @defgroup Timer_Private_Macros Timer Private Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup Timer_Private_Variables Timer Private Variables
  * @{
  */
VTIMER_HandleType_t timerHandle;
uint8_t VTimer_Callback_flag = 0;
VTIMER_HandleType_t timerHandle2;
uint8_t VTimer_Callback2_flag = 0;
VTIMER_HandleType_t timerHandle3;
uint8_t VTimer_Callback3_flag = 0;
VTIMER_HandleType_t timerHandle4;
uint8_t VTimer_Callback4_flag = 0;
/**
  * @}
  */

/** @addtogroup Timer_Private_Functions
  * @{
  */
static void wMBus_DataLink_MRSUBG_Timer_MspInit(void);
/**
  * @}
  */

/** @defgroup Timer_Exported_Functions Timer Exported Functions
  * @{
  */
void wMBus_DataLink_Timer_Init()
{
  MRSUBG_TIMER_InitTypeDef MRSUBG_TIMER_InitStruct = {0};

  while (LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC) < 0x10);
  MRSUBG_TIMER_InitStruct.HSE_XTAL_freq = 48000000;
  MRSUBG_TIMER_InitStruct.XTAL_StartupTime = 1500;
  MRSUBG_TIMER_InitStruct.enableInitialCalibration = FALSE;
  MRSUBG_TIMER_InitStruct.periodicCalibrationInterval = 0;
  wMBus_DataLink_MRSUBG_Timer_MspInit();
  HAL_MRSUBG_TIMER_Init(&MRSUBG_TIMER_InitStruct);
  timerHandle.callback = TimeoutCallback;
  timerHandle2.callback = TimeoutCallback2;
  timerHandle3.callback = TimeoutCallback3;
  timerHandle4.callback = TimeoutCallback4;
}

void TimeoutCallback(void *timerHandle)
{
  /* set Radio Timer application flag */
  VTimer_Callback_flag = 1;
}

void TimeoutCallback2(void *timerHandle)
{
  /* set Radio Timer application flag */
  VTimer_Callback2_flag = 1;
}

void TimeoutCallback3(void *timerHandle)
{
  /* set Radio Timer application flag */
  VTimer_Callback3_flag = 1;
}

void TimeoutCallback4(void *timerHandle)
{
  /* set Radio Timer application flag */
  VTimer_Callback4_flag = 1;
}

void reset_VTimer_Flags(void)
{
  VTimer_Callback_flag = 0;
  VTimer_Callback2_flag = 0;
  VTimer_Callback3_flag = 0;
  VTimer_Callback4_flag = 0;
}

void wMBus_DataLink_WKUP_IRQHandler(void)
{
  HAL_MRSUBG_TIMER_TimeoutCallback();
}
/**
  * @}
  */

/** @defgroup Timer_Private_Functions Timer Private Functions
  * @{
  */
void wMBus_DataLink_MRSUBG_Timer_MspInit(void)
{
  /* MRSUBG TIMER interrupt Init */
  HAL_NVIC_SetPriority(MRSUBG_TIMER_CPU_WKUP_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MRSUBG_TIMER_CPU_WKUP_IRQn);
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */