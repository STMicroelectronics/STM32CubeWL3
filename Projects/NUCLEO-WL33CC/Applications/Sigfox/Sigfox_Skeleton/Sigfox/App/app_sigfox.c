/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_sigfox.c
  * @author  GPM WBL Application Team
  * @brief   Application of the Sigfox Middleware
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32_lpm.h"
#include "app_sigfox.h"

/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIN(a,b)                        (((a) < (b))? (a) : (b))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
static void Fatal_Error(void);
static void SystemApp_Init(void);
static void Sigfox_Init(void);
PowerSaveLevels App_PowerSaveLevel_Check(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void MX_Sigfox_Init(void)
{
  /* USER CODE BEGIN MX_Sigfox_Init_1 */

  /* USER CODE END MX_Sigfox_Init_1 */
  SystemApp_Init();
  /* USER CODE BEGIN MX_Sigfox_Init_1_1 */

  /* USER CODE END MX_Sigfox_Init_1_1 */
  Sigfox_Init();
  /* USER CODE BEGIN MX_Sigfox_Init_2 */

  /* USER CODE END MX_Sigfox_Init_2 */
}

void MX_Sigfox_Process(void)
{
  /* USER CODE BEGIN MX_Sigfox_Process_1 */
#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_powerSave_level, vtimer_powerSave_level, final_level;

  app_powerSave_level = App_PowerSaveLevel_Check();

  if(app_powerSave_level != POWER_SAVE_LEVEL_DISABLED)
  {
    vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
    final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);

    switch(final_level)
    {
    case POWER_SAVE_LEVEL_DISABLED:
      /* Not Power Save device is busy */
      return;
      break;
    case POWER_SAVE_LEVEL_SLEEP:
      UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
      UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
      break;
    case POWER_SAVE_LEVEL_DEEPSTOP_TIMER:
      UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_ENABLE);
      UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
      break;
    case POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER:
      UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_ENABLE);
      UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_ENABLE);
      break;
    }

    UTIL_LPM_EnterLowPower();
  }
#endif /* CFG_LPM_SUPPORTED */
  
  /* USER CODE END MX_Sigfox_Process_1 */

  /* USER CODE BEGIN MX_Sigfox_Process_2 */

  /* USER CODE END MX_Sigfox_Process_2 */
}

/* USER CODE BEGIN EF */
#if (CFG_LPM_SUPPORTED == 1)
PowerSaveLevels App_PowerSaveLevel_Check(void)
{
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;

  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */
  /* USER CODE END App_PowerSaveLevel_Check_1 */

  return output_level;
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck()
{
  return POWER_SAVE_LEVEL_DEEPSTOP_TIMER;
}
#endif

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

/**
 * @brief  Application fatal error.
 * @param  None
 * @retval None
 */
static void Fatal_Error(void)
{
  /* USER CODE BEGIN Fatal_Error_1 */
  
  /* USER CODE END Fatal_Error_1 */

  while (1) {
  /* USER CODE BEGIN Fatal_Error_2 */
  
  /* USER CODE END Fatal_Error_2 */
  }
}

static void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */
  
  /* Low Power Manager Init */
  UTIL_LPM_Init();  
  
  /* USER CODE END SystemApp_Init_1 */

  if (__HAL_RCC_MRSUBG_IS_CLK_DISABLED())
  {
    /* Radio Peripheral reset */
    __HAL_RCC_MRSUBG_FORCE_RESET();
    __HAL_RCC_MRSUBG_RELEASE_RESET();

    /* Enable Radio peripheral clock */
    __HAL_RCC_MRSUBG_CLK_ENABLE();
  }

  /* USER CODE BEGIN SystemApp_Init_2 */
  
  /* USER CODE END SystemApp_Init_2 */
}

static void Sigfox_Init(void)
{
  /* USER CODE BEGIN Sigfox_Init_1 */

  /* USER CODE END Sigfox_Init_1 */
  const ST_SFX_FEMConfiguration fem_configuration = { .model = ST_SFX_FEM_NONE };

  /* STM32WL3-Specific Sigfox Low-Level Initialization */
  ST_SFX_BoardCredentials sfxCredentials;
  ST_SFX_StatusTypeDef stSfxRetErr = ST_SFX_Init(&sfxCredentials, &fem_configuration);

  if (stSfxRetErr != ST_SFX_OK) {
    /* If an error occurred reading Sigfox credentials (for example the board has never been registered)
     * automatically set the test mode. */
    if (stSfxRetErr == ST_SFX_ERR_CREDENTIALS) {
      /* USER CODE BEGIN Sigfox_Init_2 */
      sfxCredentials.id = 0;
      memset(sfxCredentials.pac, 0x00, 8);
      sfxCredentials.rc = 0;
      ST_SFX_SetPublicKey(TRUE);
      /* USER CODE END Sigfox_Init_2 */
    } else
      Fatal_Error();
  }

  /* USER CODE BEGIN Sigfox_Init_3 */
  uint8_t rc = sfxCredentials.rc == 0 ? 1 : sfxCredentials.rc;

  if (ST_SFX_Open(rc) != ST_SFX_OK)
    Fatal_Error();        
  /* USER CODE END Sigfox_Init_3 */
}
/* USER CODE BEGIN PrFD */
                      
/* USER CODE END PrFD */
