/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    timer_utils.c
  * @author  GPM WBL Application Team
  * @brief   This file provides functions to process systick
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

#include "timer_utils.h"

volatile uint32_t lSystickCounter = 0;

__weak void SysTickUserAction(uint32_t counter) { }

/**
* @brief  This function handles SysTick Handler.
* @param  None
* @retval None
*/
void SysTick_Handler(void)
{
  lSystickCounter++;

  SysTickUserAction(lSystickCounter);
}

uint32_t TIMER_UTILS_GetTimerValue(void)
{
  SysTick->CTRL;
  uint32_t reload, ticks;

  do {
    reload = lSystickCounter;
    ticks = SysTick->VAL;
  } while (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk);

  return (reload*1000+(SysTick->LOAD-ticks)/CLOCK_FREQUENCY_MHZ);
}

void TIMER_UTILS_TimestampReset(void)
{
  lSystickCounter = 0;
  SysTick->VAL = 0;
}
