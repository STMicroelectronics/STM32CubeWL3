/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file    app_entry.c
* @author  GPM WBL Application Team
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
#include "stm32_lpm.h"
#include "stm32_lpm_if.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
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

/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

uint32_t MX_APPE_Init(void *p_param)
{
  UNUSED(p_param);

  /* USER CODE BEGIN APPE_Init_1 */

  COM_InitTypeDef COM_Init = {0};

  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);

  /* USER CODE END APPE_Init_1 */

  /* Low Power Manager Init */
  UTIL_LPM_Init();

  /* USER CODE BEGIN APPE_Init_2 */

  /* Init SW2 User Button */
  BSP_PB_Init(B2, BUTTON_MODE_GPIO);

  printf("STM32WL3R Low Power Manager (LPM) demonstration\n\r");

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
static PowerSaveLevels App_PowerSaveLevel_Check(void)
{
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_ULTRADEEPSTOP;
  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */
  /* USER CODE END App_PowerSaveLevel_Check_1 */

  return output_level;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

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

  /* Configures the Button 2 as interrupt wakeup source */
  HAL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PORTA, PWR_WAKEUP_PIN7, PWR_WUP_RISIEDG);

  /*Check if wakeup occurred from UltradeepStop.*/
  if ((PWR->SDWN_WUF != 0))
  {
    printf("woke up from Ultra-Deepstop\n\r");
  }


  /* USER CODE END MX_APPE_Process_2 */
}

void MX_APPE_Idle(void)
{

  PowerSaveLevels app_powerSave_level;

  volatile uint32_t dummy[15];
  uint8_t i;

  for (i = 0; i < 10; i++)
  {
    dummy[i] = 0;
    __asm("NOP");
  }

  app_powerSave_level = App_PowerSaveLevel_Check();

  if (app_powerSave_level != POWER_SAVE_LEVEL_DISABLED)
  {
    switch (app_powerSave_level)
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
    case POWER_SAVE_LEVEL_ULTRADEEPSTOP:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_ULTRADEEPSTOP_MODE);
      break;
    }

    UTIL_LPM_Enter(0);
  }

}

