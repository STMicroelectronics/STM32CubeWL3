/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  GPAM Wireless Application Team
  * @brief   Entry point of the application
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#if (CFG_LPM_SUPPORTED == 1)
#include "stm32_lpm.h"
#include "stm32_lpm_if.h"
#endif /* CFG_LPM_SUPPORTED */

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "radio_ota.h"
#include <stdio.h>
#include <string.h>
#include "stm32wl3x_ll_usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a, b)                        (((a) < (b)) ? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/

/* USER CODE BEGIN PFP */
static void RxUART_Init(void);
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

uint32_t MX_APPE_Init(void *p_param)
{

  UNUSED(p_param);

  /* USER CODE BEGIN APPE_Init_1 */
  BSP_LED_Init(LD1);
  BSP_LED_Init(LD2);
  BSP_LED_Init(LD3);
  //BSP_LED_On(LD1);

  COM_InitTypeDef COM_Init = {0};

  COM_Init.BaudRate = 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);

  RxUART_Init();

  /* Initialize OTA path when not in YMODEM-only debug mode. */
  if (YMODEM_DEBUG_ONLY == 0U)
  {
    (void)OTA_Init();
  }

  if (OTA_DEBUG_ONLY == 0U)
  {
    Ymodem_Init();
  }
  else {
    preset_Ymodem_image();
  }
  /* USER CODE END APPE_Init_1 */

#if (CFG_LPM_SUPPORTED == 1)
  /* Low Power Manager Init */
  UTIL_LPM_Init();
#endif /* CFG_LPM_SUPPORTED */

  /* USER CODE BEGIN APPE_Init_2 */
  if (YMODEM_DEBUG_ONLY != 0U)
  {
    printf("STM32WL3 SFUOTA Ymodem Server Demo (YMODEM debug only).\r\n");
  }
  else
  {
    printf("STM32WL3 SFUOTA Ymodem Server Demo (YMODEM + OTA).\r\n");
  }
  /* USER CODE END APPE_Init_2 */

  return 0;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
#if (CFG_LPM_SUPPORTED == 1)
static PowerSaveLevels App_PowerSaveLevel_Check(void)
{
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;

  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */
  /* Keep SFUOTA server clocks/peripherals active to avoid missing UART YMODEM bytes. */
  output_level = POWER_SAVE_LEVEL_DISABLED;

  if (OTA_Get_App_Token_Status() == TOKEN_TAKEN_YMODEM)
  {
    output_level = POWER_SAVE_LEVEL_DISABLED;
  }

  if (OTA_GetStatus() == OTA_DATAREQ)
  {
    output_level = POWER_SAVE_LEVEL_DISABLED;
  }

  /* USER CODE END App_PowerSaveLevel_Check_1 */

  return output_level;
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck(void)
{
  return POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;
}
#endif

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

static void RxUART_Init(void)
{
  LL_USART_EnableIT_RXNE(USART1);
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void HAL_MRSubG_IRQ_Callback(void)
{
  OTA_Radio_IRQHandler();
}

/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void MX_APPE_Process(void)
{
  /* USER CODE BEGIN MX_APPE_Process_1 */

  /* USER CODE END MX_APPE_Process_1 */

  /* USER CODE BEGIN MX_APPE_Process_2 */
  if (OTA_DEBUG_ONLY == 0U)
  {
    (void)OTA_ymodem_tick();
  }
  if (YMODEM_DEBUG_ONLY == 0U)
  {
    (void)OTA_Tick();
  }
  /* USER CODE END MX_APPE_Process_2 */
}

void MX_APPE_Idle(void)
{
  /* USER CODE BEGIN MX_APPE_Idle_1 */

  /* USER CODE END MX_APPE_Idle_1 */

#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_powerSave_level, vtimer_powerSave_level, final_level;

  app_powerSave_level = App_PowerSaveLevel_Check();

  if (app_powerSave_level != POWER_SAVE_LEVEL_DISABLED)
  {
    vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
    final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);

    switch (final_level)
    {
    case POWER_SAVE_LEVEL_DISABLED:
      /* Not Power Save device is busy */
      return;
      break;
    case POWER_SAVE_LEVEL_SLEEP:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_SLEEP_MODE);
      break;
    case POWER_SAVE_LEVEL_DEEPSTOP_TIMER:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_DEEPSTOP_LS_MODE);
      break;
    case POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_DEEPSTOP_NOLS_MODE);
      break;
    }

    UTIL_LPM_Enter(0);
  }
#endif /* CFG_LPM_SUPPORTED */

  /* USER CODE BEGIN MX_APPE_Idle_2 */

  /* USER CODE END MX_APPE_Idle_2 */
}

