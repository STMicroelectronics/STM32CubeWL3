/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_sigfox.c
  * @author  GPM WBL Application Team
  * @brief   Application of the Sigfox Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024-2025 STMicroelectronics.
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
#include "oob_adc.h"
#include <stdio.h>
#include <string.h>
#include "stm32wl3x_ll_usart.h"
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
uint32_t cust_counter = 0;
uint8_t customer_data[12] = { 0 };
uint8_t customer_resp[8];
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
  uint32_t wakeupPin = HAL_PWR_GetClearWakeupSource(LL_PWR_WAKEUP_PORTA);
  
  if ((wakeupPin & B2_PIN)){
    BSP_LED_On(LD2);      
    
    printf("Sending Sigfox message...\n\r");    
    
    cust_counter++;
    
    for (uint8_t i = 0; i < 4; i++)
      customer_data[i] = (uint8_t)(cust_counter >> ((3 - i) * 8));
    
    SIGFOX_API_send_frame(customer_data, 4, customer_resp, 2, 0);
    
    BSP_LED_Off(LD2);
    
    printf("Done.\n\r");    
  }  
  
  /* USER CODE END MX_Sigfox_Process_1 */

  /* USER CODE BEGIN MX_Sigfox_Process_2 */
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
  COM_InitTypeDef COM_Init = {0};
  
  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);  
  
  /* Low Power Manager Init */
  UTIL_LPM_Init();      
  
  BSP_LED_Init(LD2);
  
  BSP_PB_Init(B2, BUTTON_MODE_GPIO);
  
  OOB_ADC_Init();
  
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
  
  /* Configures the Button 2 as interrupt wakeup source */    
  HAL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PORTA, B2_PIN, PWR_WUP_FALLEDG);
  
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

  /* User Application */  
  printf("Sigfox Push Button demo\r\n");

  printf("ID: %.8lX - PAC: ", (unsigned long)sfxCredentials.id);
  for (uint16_t i = 0; i < sizeof(sfxCredentials.pac); i++) {
    printf("%.2X", sfxCredentials.pac[i]);
  }
  printf("\r\n");
  /* USER CODE END Sigfox_Init_3 */
}
/* USER CODE BEGIN PrFD */
                      
/* USER CODE END PrFD */
