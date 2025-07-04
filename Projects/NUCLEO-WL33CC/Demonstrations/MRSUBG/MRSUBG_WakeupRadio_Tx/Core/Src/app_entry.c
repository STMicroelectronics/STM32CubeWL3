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
#include "stm32wl3x_ll_mrsubg.h"
/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
#define PACKET_LEN 15
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a,b)                        (((a) < (b))? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
__attribute__((aligned(4))) static uint8_t vectcTxBuff[PACKET_LEN];
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/

/* USER CODE BEGIN PFP */
static void CreateLPAWURFrame(void);
void EvaluateCrc(uint8_t * LPAWUR_payload);

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
  
  /* Init SW2 User Button */
  BSP_PB_Init(B2, BUTTON_MODE_GPIO);
  
  /* Payload length config */
  HAL_MRSubG_PktBasicSetPayloadLength(PACKET_LEN);
  
  /* Set Manchester Coding Type */
  LL_MRSubG_PacketHandlerManchesterType(MANCHESTER_TYPE0);   
  
  /* Set TX Mode to Normal Mode*/
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&vectcTxBuff);
  
  /* Create the data to transmit */
  CreateLPAWURFrame();
  
  printf("STM32WL3 LPAWUR - Transmitter example.\n\r");
  printf("Use PUSH Button 2 to send a frame.\n\r");
  
  /* USER CODE END APPE_Init_2 */
  return 0;
}

/* USER CODE BEGIN FD */
static void CreateLPAWURFrame(void){
  
  /* bit sync */
  for(int i = 0; i<5; i++)
    vectcTxBuff[i] = 0x00;
  
  /* Frame sync */  
  vectcTxBuff[5] = 0x99;
  
  /* Payload */
  vectcTxBuff[6] = 0x07;
  vectcTxBuff[7] = 0x06;
  vectcTxBuff[8] = 0x05;
  vectcTxBuff[9] = 0x04;
  vectcTxBuff[10] = 0x03;
  vectcTxBuff[11] = 0x02;
  vectcTxBuff[12] = 0x01;  
  
  /* CRC */
  EvaluateCrc(&vectcTxBuff[6]);
}

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
  HAL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PORTA, PWR_WAKEUP_PIN11, PWR_WUP_FALLEDG); 
  
  uint32_t wakeupPin = HAL_PWR_GetClearWakeupSource(LL_PWR_WAKEUP_PORTA);  
  
  if (wakeupPin & B2_PIN)
  {
    BSP_LED_On(LD2);
    
    printf("Transmitting: [ ");
    
    for(uint8_t i=0 ; i<PACKET_LEN ; i++)
      printf("%d ", vectcTxBuff[i]);
    printf("]\n\r");
    
    /* Send the TX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);    
    
    /* Wait for TX done */
    while((__HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS() & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F) == 0) {};
    
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);      
    
    BSP_LED_Off(LD2);        
  }  
  

  /* USER CODE END MX_APPE_Process_2 */
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck()
{
  return POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;
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

