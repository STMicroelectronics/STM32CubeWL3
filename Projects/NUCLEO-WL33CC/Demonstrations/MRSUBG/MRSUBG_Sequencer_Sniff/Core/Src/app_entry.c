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
#include "stm32_lpm_if.h"
#include "SequencerFlowgraph.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */

#define BASE_FREQUENCY              868000000
#define MODULATION_SELECT           MOD_2FSK
#define DATARATE                    38400
#define FREQ_DEVIATION              20000
#define BANDWIDTH                   100000

#define PACKET_LEN                  15
#define SNIFF_INTERVAL_TIME         12
#define RX_TIMEOUT                  30
#define FAST_RX_TIMEOUT             430

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a,b)                        (((a) < (b))? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t Data_received = 0;
volatile uint8_t flagDEEPSTOP;

/* data buffers */
extern volatile uint8_t databuf0[];
extern volatile uint8_t databuf1[];

uint8_t fast_rx_time_val;
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/

/* USER CODE BEGIN PFP */
void SequencerConfig(void);

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
  
  printf("STM32WL3 MRSUBG Sequencer Sniff\r\n");
    
  /* Calculate FAST TIM val */
  uint8_t temp_val;
  temp_val = LL_MRSubG_GetChFlt_E();
  fast_rx_time_val = (FAST_RX_TIMEOUT * (16000000/(8*(1<<(temp_val&0x0F)))))/1000000;
  
  /* Load Sequencer Configuration */
  SequencerConfig();
  
  HAL_SuspendTick();
  
  /* USER CODE END APPE_Init_1 */
  
  /* Low Power Manager Init */
  UTIL_LPM_Init();
  
  /* USER CODE BEGIN APPE_Init_2 */  
  BSP_LED_Init(LD2);
  
  /* Power Save semaphore */
  flagDEEPSTOP = TRUE;
  
  /* Payload length config */
  HAL_MRSubG_PktBasicSetPayloadLength(PACKET_LEN);   
  
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&databuf0);
  __HAL_MRSUBG_SET_DATABUFFER1_POINTER((uint32_t)&databuf1);
  
  __HAL_MRSUBG_SET_WAKEUP_OFFSET(50);
  
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
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DISABLED;
  
  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */

  output_level = flagDEEPSTOP ? POWER_SAVE_LEVEL_DEEPSTOP_TIMER: POWER_SAVE_LEVEL_DISABLED;
  
  /* USER CODE END App_PowerSaveLevel_Check_1 */
  
  return output_level;
}
#endif

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

static void print_RX_data(void)
{
  BSP_LED_On(LD2);
              
  /* Print the received data */
  printf("Received: ");

  for(uint8_t i=0 ; i<PACKET_LEN; i++)
    printf("%x ", databuf0[i]);

  printf("\r\n");
  
  
  BSP_LED_Off(LD2);
}

void MRSUBG_RX_TIMEOUT_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MRSUBG_RX_OK_IRQHandler(void) {
  Data_received = 1;
//  flagDEEPSTOP = TRUE;
}

void MRSUBG_RX_CRC_ERROR_IRQHandler(void) {
  Data_received = 1;
  flagDEEPSTOP = TRUE;
}

void MRSUBG_SEQ_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MRSUBG_FAST_RX_TERM_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MRSUBG_HW_ANA_FAILURE_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
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
  
  if(Data_received == 1)
  {
    print_RX_data();
    Data_received = 0;
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

    if (final_level >= POWER_SAVE_LEVEL_DEEPSTOP_TIMER)
    {
      flagDEEPSTOP = FALSE;
    }
  }
#endif /* CFG_LPM_SUPPORTED */
}
