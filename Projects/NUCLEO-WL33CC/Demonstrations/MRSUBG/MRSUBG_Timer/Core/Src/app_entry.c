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

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "stm32wl3x_ll_usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */

#define WAKEUP_TIMEOUT 10000 // 10s
#ifndef SINGLE_TIMER_OPT
#define WAKEUP_TIMEOUT2 10000 
#define WAKEUP_TIMEOUT3 10000 
#endif


#define MSG_SIZE 20

#ifdef SINGLE_TIMER_OPT
void HAL_MRSUBG_TIMER_StartConfigureTimer(uint32_t time);

#define INTERPOLATED_FREQ (16*32768)
#define TIMEOUT_500us 500
#define TIMEOUT_10s 10
#define TIMEOUT_WITH_DEEPSTOP (uint32_t)(TIMEOUT_10s * INTERPOLATED_FREQ)
#define TIMEOUT_NO_DEEPSTOP (TIMEOUT_500us * INTERPOLATED_FREQ)/1000000
#define TIMER_MAX_VALUE 0xFFFFFFFF
#define LOW_POWER_THR_STU 60
#define LOW_POWER_THR (76 + 786) //786 is HS startup time (1500 us) expressed in interpolated time
#endif

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a,b)                        (((a) < (b))? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SMRSubGConfig_t MRSUBG_RadioInitStruct;
MRSubG_PcktBasicFields_t MRSUBG_PacketSettingsStruct;

/* USER CODE BEGIN PV */
__attribute__((aligned(4))) uint8_t vectcTxBuff[MSG_SIZE]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,0};
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */
VTIMER_HandleType_t timerHandle;
uint8_t VTimer_Callback_flag = 0;
#ifndef SINGLE_TIMER_OPT
VTIMER_HandleType_t timerHandle2;
VTIMER_HandleType_t timerHandle3;
uint8_t VTimer_Callback2_flag = 0;
uint8_t VTimer_Callback3_flag = 0;
#endif
/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
PowerSaveLevels App_PowerSaveLevel_Check(void);
static void SystemApp_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
uint32_t MX_APPE_Init(void *p_param)
{
  UNUSED(p_param);
  
  /* USER CODE BEGIN APPE_Init_1 */
  
  /* USER CODE END APPE_Init_1 */
  SystemApp_Init();
  /* Low Power Manager Init */
  UTIL_LPM_Init();

  /* USER CODE BEGIN APPE_Init_2 */
  /* Payload length config */
  HAL_MRSubG_PktBasicSetPayloadLength(MSG_SIZE);

  /* Set TX Mode to Normal Mode*/
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  
  /* Set the pointer to the data buffer */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&vectcTxBuff);

  /* Start the VTimer  */
  MX_APPE_RadioTimerConfig();
  printf("STM32WL3 MRSUBG Timer Demo.\n\r");
  printf("Wakeup every %d seconds or using PUSH Button 2 \n\r", WAKEUP_TIMEOUT/1000);
  /* USER CODE END APPE_Init_2 */
   return 0;
}

void MX_APPE_Process(void)
{
  /* USER CODE BEGIN MX_APPE_Process_1 */
#ifndef SINGLE_TIMER_OPT
  HAL_MRSUBG_TIMER_Tick();
#endif

  uint32_t wakeupPin = HAL_PWR_GetClearWakeupSource(LL_PWR_WAKEUP_PORTA);
  
  uint32_t wakeupInternal = HAL_PWREx_GetClearInternalWakeUpLine();
#ifndef SINGLE_TIMER_OPT
    /* check if Wake-up for transmission */
    if (VTimer_Callback_flag == 1)
		VTimer_Callback_flag = 0;
#endif
  if (wakeupPin & B2_PIN){
    BSP_LED_On(LD2);
    printf("Wakeup on Button B2!\r\n");
    BSP_LED_Off(LD2);        
  }

  if (wakeupInternal & PWR_WAKEUP_SUBGHOST){
    BSP_LED_On(LD2);
    printf("Wakeup on Radio Timer!\r\n");
    BSP_LED_Off(LD2);        
  }

  if ((wakeupPin & B2_PIN) || (wakeupInternal & PWR_WAKEUP_SUBGHOST)){
    BSP_LED_On(LD2);

    printf("TX - Data to transmit: [ ");

    for(uint8_t i=0 ; i<MSG_SIZE ; i++)
      printf("%d ", vectcTxBuff[i]);
    printf("]\n\r");

    /* Send the TX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);

    /* Wait for TX done */
    while((__HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS() & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F) == 0) {};

    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);

    /* Increment last byte */
    vectcTxBuff[MSG_SIZE-1]++;

    BSP_LED_Off(LD2);
  }
  /* USER CODE END MX_APPE_Process_1 */

  /* USER CODE BEGIN MX_APPE_Process_2 */
#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_powerSave_level, final_level;
#ifndef SINGLE_TIMER_OPT
  PowerSaveLevels vtimer_powerSave_level;
#endif
  
  app_powerSave_level = App_PowerSaveLevel_Check();
  
  if(app_powerSave_level != POWER_SAVE_LEVEL_DISABLED) 
  {
#ifdef SINGLE_TIMER_OPT
    final_level = app_powerSave_level;
#else
    vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
    final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);
#endif
    
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
  /* USER CODE END MX_APPE_Process_2 */

#endif /* CFG_LPM_SUPPORTED */
}

void MX_APPE_RadioTimerConfig(void)
{

#ifdef SINGLE_TIMER_OPT
#ifdef NO_DEEPSTOP
  HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_500us);
#else
  HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_10s);
  
#endif
#else
#ifdef NO_DEEPSTOP
  /* Start the VTimer 500us */
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, WAKEUP_NO_DEEPSTOP_TIMEOUT);
#else
  uint8_t ret_val=SUCCESS;
  /* Start the radio timer with WAKEUP_TIMEOUT */
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle, WAKEUP_TIMEOUT);
#ifndef SINGLE_TIMER_OPT
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle2, WAKEUP_TIMEOUT2);
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle3, WAKEUP_TIMEOUT3);
  if (ret_val != SUCCESS) {
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    while(1);
  }
#endif
#endif
#endif


}

/* USER CODE BEGIN EF */
void TimeoutCallback(void *timerHandle)
{
  uint8_t ret_val;
  
  /* Add app code to execute @ Stop timeout */

#ifdef NO_DEEPSTOP
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_NO_DEEPSTOP_TIMEOUT);
#else
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT);
#endif

  if (ret_val != SUCCESS) {
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    while(1);
  }
  /* set Radio Timer application flag */
  VTimer_Callback_flag = 1;
}

#ifndef SINGLE_TIMER_OPT
void TimeoutCallback2(void *timerHandle)
{
  uint8_t ret_val;
    
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT2);
  

  if (ret_val != SUCCESS) {
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    while(1);
  }

  /* set Radio Timer application flag */
  VTimer_Callback2_flag = 1;
}

void TimeoutCallback3(void *timerHandle)
{
  uint8_t ret_val;
    
  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT3);
  

  if (ret_val != SUCCESS) {
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    while(1);
  }

  /* set Radio Timer application flag */
  VTimer_Callback3_flag = 1;
}

#endif

#if (CFG_LPM_SUPPORTED == 1)
PowerSaveLevels App_PowerSaveLevel_Check(void)
{
#ifdef NO_DEEPSTOP
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DISABLED;
#else
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_TIMER;
#ifdef SINGLE_TIMER_OPT
  if(LL_MRSUBG_TIMER_GetCPUWakeupTime(MR_SUBG_GLOB_RETAINED) < (LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC)))
  {
    HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_10s);
  }
  else if(LL_MRSUBG_TIMER_GetCPUWakeupTime(MR_SUBG_GLOB_RETAINED) < (LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC) + LOW_POWER_THR))
  {
    output_level = POWER_SAVE_LEVEL_SLEEP;
  }
#endif
#endif
  
  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */ 
   
  /* USER CODE END App_PowerSaveLevel_Check_1 */
  
  return output_level;
}
#endif

#ifdef SINGLE_TIMER_OPT
void HAL_MRSUBG_TIMER_StartConfigureTimer(uint32_t time)
{
  uint32_t ABSOLUTE_TIME =  LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC);
  
  if (time == TIMEOUT_500us) {
    LL_MRSUBG_TIMER_SetCPUWakeupTime(MR_SUBG_GLOB_RETAINED, ((ABSOLUTE_TIME + TIMEOUT_NO_DEEPSTOP) & TIMER_MAX_VALUE));
  } else {
    LL_MRSUBG_TIMER_SetCPUWakeupTime(MR_SUBG_GLOB_RETAINED, ((ABSOLUTE_TIME + TIMEOUT_WITH_DEEPSTOP) & TIMER_MAX_VALUE));
  }
  
  LL_MRSUBG_TIMER_EnableCPUWakeupTimer(MR_SUBG_GLOB_RETAINED);
}
#endif

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck()
{
  return POWER_SAVE_LEVEL_DEEPSTOP_TIMER;
}

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

static void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */
  
  /* Low Power Manager Init */
  UTIL_LPM_Init();      
  
  BSP_LED_Init(LD2);
  
  BSP_PB_Init(B2, BUTTON_MODE_GPIO);
  
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
  HAL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PORTA, PWR_WAKEUP_PIN11, PWR_WUP_FALLEDG);
  /* USER CODE END SystemApp_Init_2 */
}
