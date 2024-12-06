/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file    app_entry.c
* @author  GPM WBL Application Team
* @brief   Entry point of the application
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
#include "stm32wl3x_ll_usart.h"
#include "stm32wl3x_ll_lpawur.h"


/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
#define PAYLOAD_LEN 7
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a,b)                        (((a) < (b))? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t LPAWUR_Payload[8];
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
  BSP_LED_Init(LD2);
  
  printf("LPAWUR - Receiver example.\n\r");  
  
  LL_LPAWUR_SetWakeUpLevel(WAKEUP_FRAME_VALID);
  LL_LPAWUR_SetState(ENABLE);
  
  /* USER CODE END APPE_Init_2 */
  return 0;
}

/* USER CODE BEGIN FD */

/**
* @brief  Check if UART is BUSY or not.
* @retval TRUE if UART is BUSY, FALSE otherwise.
*/

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
  /* USER CODE END App_PowerSaveLevel_Check_1 */
  
  return output_level;
}
#endif

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

  /* Wakeup source configuration */    
  HAL_PWREx_EnableInternalWakeUpLine(PWR_WAKEUP_LPAWUR, PWR_WUP_RISIEDG);    
  
  uint32_t wakeupSource = HAL_PWREx_GetClearInternalWakeUpLine();  
     
  /* Wakeup on LPAWUR Frame Valid */  
  
  if (wakeupSource & PWR_WAKEUP_LPAWUR)
  {
    BSP_LED_On(LD2);
    
    HAL_LPAWUR_GetPayload(LPAWUR_Payload);
    
    printf("LPAWUR data received: [ ");
    for(uint8_t i=0;i<PAYLOAD_LEN;i++)
    {
      printf("%x",LPAWUR_Payload[i]);
    }
    printf(" ]\n\r");
    
    HAL_LPAWUR_ClearStatus();
    LL_LPAWUR_SetState(ENABLE);
    
    BSP_LED_Off(LD2);       
  }    
  

  
  /* USER CODE END MX_APPE_Process_2 */
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck()
{
  return POWER_SAVE_LEVEL_DEEPSTOP_TIMER;
}

void MX_APPE_Idle(void)
{
#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_powerSave_level, vtimer_powerSave_level, final_level;
  
  volatile uint32_t dummy[15];
  uint8_t i;
  
  for (i=0; i<10; i++)
  {
    dummy[i] = 0;
    __asm("NOP");
  }
	
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
}

