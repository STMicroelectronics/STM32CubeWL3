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

/* Allocate databuffers */
__attribute__((aligned(4))) volatile uint8_t databuf0[PACKET_LEN];
__attribute__((aligned(4))) volatile uint8_t databuf1[PACKET_LEN];

static MRSubG_Sequencer_GlobalConfiguration_t GlobalSequencerConfiguration;
static MRSubG_Sequencer_ActionConfiguration_t ReceiveFrame;
static MRSubG_Sequencer_ActionConfiguration_t WaitForMessage;

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

void SequencerConfig(void)
{  
  /*****
  * Control flow settings for ReceiveFrame
  * If a signal was previously detected (CS flag raised in WaitForMessage), restart RX timer from the beginning to give receiver more time to capture message.
  * If message is successfully received or CRC error occurs, the flowgraph terminates (unconnected NextAction1 output).
  * If CS was erroneous and actually no message is on the air, restart with WaitForMessage after a 100ms interval.
  *****/
  
  /****** frame RX mode ******/
  ReceiveFrame.NextAction2Ptr = &WaitForMessage;  // re-start next RX from CS with 200 ms interval
  ReceiveFrame.NextAction2Ctrl = SEQ_FLAG_SLEEPENABLE | SEQ_FLAG_CLEAREVENTS | SEQ_FLAG_FORCERELOAD;
  ReceiveFrame.NextAction2Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F;  // found SYNC and packet received
  ReceiveFrame.NextAction2Interval = HAL_MRSubG_Sequencer_Milliseconds(SNIFF_INTERVAL_TIME);
  
  ReceiveFrame.NextAction1Ptr = &WaitForMessage;  // re-start next RX from CS with 500ms interval
  ReceiveFrame.NextAction1Ctrl = SEQ_FLAG_SLEEPENABLE | SEQ_FLAG_CLEAREVENTS | SEQ_FLAG_FORCERELOAD;
  ReceiveFrame.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F |
    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F | // no packet within 15 msec
      MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_FAST_RX_TERM_F |
        MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F;
  ReceiveFrame.NextAction1Interval = HAL_MRSubG_Sequencer_Milliseconds(SNIFF_INTERVAL_TIME);
  
  ReceiveFrame.ActionTimeout = 0; // no limit
  
  /*****
  * Control flow settings for WaitForMessage
  * If some signal is received (CS flag raised), transition to ReceiveFrame state.
  * Otherwise, if no signal is received within the RX timer interval (500ms), restart listening, but only after a 500ms delay.
  * This means that the overall receive duty cycle is 50%.
  *****/
  WaitForMessage.NextAction1Ptr = &ReceiveFrame;  /* move to RX frame mode */
  WaitForMessage.NextAction1Ctrl = SEQ_FLAG_CLEAREVENTS| SEQ_FLAG_FORCERELOAD;
  WaitForMessage.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_CS_F; // trigger by RSSI OK
  WaitForMessage.NextAction1Interval = HAL_MRSubG_Sequencer_Milliseconds(0);
  
  WaitForMessage.NextAction2Ptr = &WaitForMessage;  // stay same action
  WaitForMessage.NextAction2Ctrl = SEQ_FLAG_SLEEPENABLE | SEQ_FLAG_CLEAREVENTS;
  WaitForMessage.NextAction2Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_FAST_RX_TERM_F |//; //FAST_RX Timeout is occurred
    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F; 
  WaitForMessage.NextAction2Interval = HAL_MRSubG_Sequencer_Milliseconds(SNIFF_INTERVAL_TIME); // next RX start is after 15 msec
  
  WaitForMessage.ActionTimeout = 0; // no limit
  
  /************************************************************************
  Basic dynamic configuration for
  === ReceiveFrame === 
  *******/
  __HAL_MRSUBG_SET_PKT_LEN(PACKET_LEN);

  HAL_MRSubG_SetFrequencyBase(BASE_FREQUENCY);
  HAL_MRSubG_SetDatarate(DATARATE);
  HAL_MRSubG_SetFrequencyDev(FREQ_DEVIATION);
  HAL_MRSubG_SetChannelBW(BANDWIDTH);

  /* Configure modulation MOD_2FSK */
  LL_MRSubG_SetBTSel(BT_1);
  HAL_MRSubG_SetModulation(MOD_2FSK, 0);

  /* FIFO Configuration */
  LL_MRSubG_SetAlmostFullThresholdRx(100);

  /*** RX timer configuration ***/
  __HAL_MRSUBG_SET_RX_TIMEOUT(HAL_MRSubG_Sequencer_Milliseconds(RX_TIMEOUT)); // 15 msec
  
  /**** Fast Timeout config No need on full RX phase ****/
  LL_MRSubG_FastCsTermEn(DISABLE);
  __HAL_MRSUBG_SET_FAST_RX_TIMEOUT(0); // disable FAST RX TIMEOUT
  
  /******* Interrupt config ********/
  /* IRQ: - RX_OK, - RX_TIMEOUT, - CRC_ERROR, - HW_ANA_FAILURE, - SEQ_RND */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_TIMEOUT_E |
                                      MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E |
                                        MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_FAST_RX_TERM_E |
                                          MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E);
  
  
  /* Always perform calibration */
  LL_MRSubG_VCOCalibReq(SET);

  /* Store dynamic configuration to ReceiveFrame */
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&ReceiveFrame, CMD_RELOAD_RX_TIMER);
  
  
  /***********************************************************************************
  Basic dynamic configuration for
  === WaitForMessage == 
  ******/
  __HAL_MRSUBG_SET_PKT_LEN(PACKET_LEN);

  HAL_MRSubG_SetFrequencyBase(BASE_FREQUENCY);
  HAL_MRSubG_SetDatarate(DATARATE);
  HAL_MRSubG_SetFrequencyDev(FREQ_DEVIATION);
  HAL_MRSubG_SetChannelBW(BANDWIDTH);

  /* Configure modulation MOD_2FSK */
  LL_MRSubG_SetBTSel(BT_1);
  HAL_MRSubG_SetModulation(MOD_2FSK, 0);
  
  /* FIFO Configuration */
  LL_MRSubG_SetAlmostEmptyThresholdTx(10);
  LL_MRSubG_SetAlmostFullThresholdRx(100);
  
  /*** No use RX Timeout, use FAST RX TIMER 
  If CS det during FAST TIMEOUT, move to normal RX phase and set RX TIMER */
  
  __HAL_MRSUBG_SET_RX_TIMEOUT(0); // Don't use RX timer (use FAST_RX_TIME)
  
  /**** CLEAR == Don't Stop RX Timer => still effect RX timeout even through to detct it */
  /* When CS is raised, move state to RX (all packet) immediately. So, don't need to stop RX timer */
  LL_MRSubG_SetRxCsTimeout(DISABLE);
  LL_MRSubG_SetRxPqiTimeout(DISABLE);
  LL_MRSubG_SetRxSqiTimeout(DISABLE);
  
  LL_MRSubG_SetRxOrnAndSelect(SET); // set OR
   
  LL_MRSubG_FastCsTermEn(ENABLE); // Enable FAST RX TIMER
  __HAL_MRSUBG_SET_FAST_RX_TIMEOUT(fast_rx_time_val);
  
  
  /******* Interrupt config ********/
  /* IRQ: - RX_TIMEOUT, - HW_ANA_FAILURE, - SEQ_END, - CS_E */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_FAST_RX_TERM_E |                                          
                                      MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_CS_E);

  /* Always perform calibration */
  LL_MRSubG_VCOCalibReq(SET);

  /* Store dynamic configuration to WaitForMessage */
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&WaitForMessage, CMD_RX);
  
  /* =======Up to here, set WaitForMessage SeqAction ====================================================================================*/  

  /* Assign allocated buffers to databuffer pointer registers */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&databuf0[0]);
  __HAL_MRSUBG_SET_DATABUFFER1_POINTER((uint32_t)&databuf1[0]);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(sizeof(databuf0));

  /* Enable calibration */
  LL_MRSubG_VCOCalibReq(SET);

  __HAL_MRSUBG_SET_RX_MODE(RX_NORMAL);
  HAL_MRSubG_SetRSSIThreshold(-70);

  GlobalSequencerConfiguration.Flags = 0;
  GlobalSequencerConfiguration.ActionConfigPtr = &WaitForMessage;
  HAL_MRSubG_Sequencer_ApplyStaticConfig(&GlobalSequencerConfiguration);
  LL_MRSubG_Sequencer_SetGlobalConfiguration(&GlobalSequencerConfiguration);
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(0xffffffff);
}

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

void HAL_MRSubG_IRQ_Callback(void) {
  uint32_t irq_status = MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS;
  uint32_t status_detail = MR_SUBG_GLOB_STATUS->RFSEQ_STATUS_DETAIL;
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(0xffffffff);

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F)
    MR_SUBG_HW_ANA_FAILURE_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F) {
    MR_SUBG_RX_TIMEOUT_IRQHandler();
  }
  
  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_FAST_RX_TERM_F) {
    MR_SUBG_FAST_RX_TERM_IRQHandler();
  }
  
  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F) {
    MR_SUBG_RX_OK_IRQHandler();
  }

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F)
    MR_SUBG_RX_CRC_ERROR_IRQHandler();
}

void MR_SUBG_RX_TIMEOUT_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MR_SUBG_RX_OK_IRQHandler(void) {
  Data_received = 1;
  flagDEEPSTOP = TRUE;
}

void MR_SUBG_RX_CRC_ERROR_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MR_SUBG_SEQ_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MR_SUBG_FAST_RX_TERM_IRQHandler(void) {
  flagDEEPSTOP = TRUE;
}

void MR_SUBG_HW_ANA_FAILURE_IRQHandler(void) {
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

    if (final_level >= POWER_SAVE_LEVEL_DEEPSTOP_TIMER)
    {
      flagDEEPSTOP = FALSE;
    }
  }
#endif /* CFG_LPM_SUPPORTED */
}
