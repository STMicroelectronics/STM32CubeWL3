/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @author  GPM WBL Application Team
  * @brief   This code implements a bidirectional point to point communication.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DATABUFFERS_SIZE 8

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SMRSubGConfig_t MRSUBG_RadioInitStruct;
MRSubG_PcktBasicFields_t MRSUBG_PacketSettingsStruct;

/* USER CODE BEGIN PV */
__attribute__((aligned(4))) volatile uint8_t databuf0[DATABUFFERS_SIZE];
__attribute__((aligned(4))) volatile uint8_t databuf1[DATABUFFERS_SIZE];

static volatile uint8_t ack_failed = 0, ack_received = 0, sequencer_timeout = 0;
static uint32_t msg_count = 0;

static MRSubG_Sequencer_GlobalConfiguration_t globalcfg;
static MRSubG_Sequencer_ActionConfiguration_t actioncfgSendMsg;
static MRSubG_Sequencer_ActionConfiguration_t actioncfgWaitACK;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_MRSUBG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_MRSUBG_Init();
  /* USER CODE BEGIN 2 */
  COM_InitTypeDef COM_Init = {0};
  
  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);
  
  /* Assign allocated buffers to databuffer pointer registers */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&databuf0[0]);
  __HAL_MRSUBG_SET_DATABUFFER1_POINTER((uint32_t)&databuf1[0]);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(DATABUFFERS_SIZE);

  HAL_MRSubG_SetPALeveldBm(7, 14, PA_DRV_TX_HP);

  /* Set a 500ms RX Timeout */
  __HAL_MRSUBG_SET_RX_TIMEOUT(HAL_MRSubG_Sequencer_Milliseconds(500));
  HAL_MRSubG_SetRSSIThreshold(-139);

  /* Set the SOC_WAKEUP_OFFSET */
  __HAL_MRSUBG_SET_WAKEUP_OFFSET(6);

  memcpy((uint8_t*)databuf0, &msg_count, sizeof(msg_count));
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  __HAL_MRSUBG_SET_PKT_LEN(DATABUFFERS_SIZE);

  actioncfgSendMsg.NextAction1Ptr = &actioncfgWaitACK;
  actioncfgSendMsg.NextAction1Ctrl = 0;
  actioncfgSendMsg.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F;
  actioncfgSendMsg.NextAction1Interval = 0;

  actioncfgSendMsg.NextAction2Ptr = NULL;
  actioncfgSendMsg.NextAction2Ctrl = 0;
  actioncfgSendMsg.NextAction2Mask = SEQ_MASK_NEVER_MATCH;
  actioncfgSendMsg.NextAction2Interval = 0;

  actioncfgSendMsg.ActionTimeout = 0;
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&actioncfgSendMsg, CMD_TX);

  /*******************************/

  __HAL_MRSUBG_SET_RX_MODE(RX_NORMAL);
  __HAL_MRSUBG_SET_PKT_LEN(1);
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_TIMEOUT_E);

  actioncfgWaitACK.NextAction1Ptr = NULL;
  actioncfgWaitACK.NextAction1Ctrl = 0;
  actioncfgWaitACK.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F;
  actioncfgWaitACK.NextAction1Interval = 0;

  actioncfgWaitACK.NextAction2Ptr = NULL;
  actioncfgWaitACK.NextAction2Ctrl = 0;
  actioncfgWaitACK.NextAction2Mask = (MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F | MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F);
  actioncfgWaitACK.NextAction2Interval = 0;

  actioncfgWaitACK.ActionTimeout = HAL_MRSubG_Sequencer_Milliseconds(1000);
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&actioncfgWaitACK, CMD_RX);

  /*******************************/

  globalcfg.Flags = SEQ_FLAG_FORCERELOAD | SEQ_FLAG_CLEAREVENTS;
  globalcfg.ActionConfigPtr = &actioncfgSendMsg;
  HAL_MRSubG_Sequencer_ApplyStaticConfig(&globalcfg);
  
  __HAL_MRSUBG_SEQ_SET_GLOBAL_CONFIG(&globalcfg);
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(0xffffffff);
  __HAL_MRSUBG_SEQ_TRIGGER();

  /*******************************/

  /*
   * Now the MRSUBG sequencer will take care of handling the whole 
   * transmission acknowledgement process.
   */
  printf("Triggered sequencer, transmitting message and auto-waiting for ACK.\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint8_t restart_sequence = 0;

    if (ack_received) {
      printf("ACK OK, scheduling next message!\r\n");
      ack_received = 0;
      restart_sequence = 1;
    }
    if (ack_failed) {
      printf("ACK missed, scheduling next message!\r\n");
      ack_failed = 0;
      restart_sequence = 1;
    }
    if (sequencer_timeout) {
      sequencer_timeout = 0;
      printf("Unexpected sequencer timeout!\r\n");
      restart_sequence = 1;
    }
    
    if (restart_sequence) {
      HAL_Delay(50); /* Allow other party to print the message */
      msg_count++;
      memcpy((uint8_t*)databuf0, &msg_count, sizeof(msg_count));
      globalcfg.ActionConfigPtr = &actioncfgSendMsg;
      __HAL_MRSUBG_SEQ_TRIGGER();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource and SYSCLKDivider
  */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_RC64MPLL;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_RC64MPLL_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WAIT_STATES_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLK_DIV4;
  PeriphClkInitStruct.KRMRateMultiplier = 4;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief MRSUBG Initialization Function
  * @param None
  * @retval None
  */
static void MX_MRSUBG_Init(void)
{

  /* USER CODE BEGIN MRSUBG_Init 0 */

  /* USER CODE END MRSUBG_Init 0 */

  /* USER CODE BEGIN MRSUBG_Init 1 */

  /* USER CODE END MRSUBG_Init 1 */

  /** Configures the radio parameters
  */
  MRSUBG_RadioInitStruct.lFrequencyBase = 868000000;
  MRSUBG_RadioInitStruct.xModulationSelect = MOD_2FSK;
  MRSUBG_RadioInitStruct.lDatarate = 38400;
  MRSUBG_RadioInitStruct.lFreqDev = 20000;
  MRSUBG_RadioInitStruct.lBandwidth = 100000;
  MRSUBG_RadioInitStruct.dsssExp = 0;
  MRSUBG_RadioInitStruct.outputPower = 14;
  MRSUBG_RadioInitStruct.PADrvMode = PA_DRV_TX_HP;
  HAL_MRSubG_Init(&MRSUBG_RadioInitStruct);

  /** Configures the packet parameters
  */
  MRSUBG_PacketSettingsStruct.PreambleLength = 16;
  MRSUBG_PacketSettingsStruct.PostambleLength = 0;
  MRSUBG_PacketSettingsStruct.SyncLength = 31;
  MRSUBG_PacketSettingsStruct.SyncWord = 0x88888888;
  MRSUBG_PacketSettingsStruct.FixVarLength = VARIABLE;
  MRSUBG_PacketSettingsStruct.PreambleSequence = PRE_SEQ_0101;
  MRSUBG_PacketSettingsStruct.PostambleSequence = POST_SEQ_0101;
  MRSUBG_PacketSettingsStruct.CrcMode = PKT_CRC_MODE_8BITS;
  MRSUBG_PacketSettingsStruct.Coding = CODING_NONE;
  MRSUBG_PacketSettingsStruct.DataWhitening = ENABLE;
  MRSUBG_PacketSettingsStruct.LengthWidth = BYTE_LEN_1;
  MRSUBG_PacketSettingsStruct.SyncPresent = ENABLE;
  HAL_MRSubG_PacketBasicInit(&MRSUBG_PacketSettingsStruct);
  /* USER CODE BEGIN MRSUBG_Init 2 */

  /* USER CODE END MRSUBG_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_MRSubG_IRQ_Callback(void) {
  uint32_t status = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();
  
  if (status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F) {
    ack_received = 1;
  }
  if (status & (MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F |
                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F)) {
    ack_failed = 1;
  }
  if (status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SEQ_F) {
    uint32_t status_details = __HAL_MRSUBG_GET_RFSEQ_STATUS_DETAIL();
    if (status_details & MR_SUBG_GLOB_STATUS_RFSEQ_STATUS_DETAIL_SEQ_ACTIONTIMEOUT_F) {
      sequencer_timeout = 1;
    }
    __HAL_MRSUBG_SET_RFSEQ_STATUS_DETAIL(status_details);
  }
  
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(status);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
