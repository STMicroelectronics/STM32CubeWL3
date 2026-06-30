/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @author  GPM WBL Application Team
  * @brief   Basic project structure template.
  ******************************************************************************
  * @attention
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
/** @} */
/** \cond DOXYGEN_SHOULD_SKIP_THIS
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MSG_SIZE 20

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config_HSI(void);
void SystemClock_Config_HSE(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_MRSUBG_Init(void);
static void MX_MRSUBG_TIMER_Init(void);
static void MX_RTC_Init(void);

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
  
  /* Configure the system clock using HSE */
  SystemClock_Config_HSE();
  
  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_MRSUBG_Init();

  MX_MRSUBG_TIMER_Init();
  MX_RTC_Init();
  
  /* USER CODE BEGIN 2 */
  COM_InitTypeDef COM_Init = {0};
  
  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);
  
  /* USER CODE END 2 */
  MX_APPE_Init(NULL);
  
  /* re-configure the system clock using HSI */
  SystemClock_Config_HSI();
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    
    MX_APPE_Process();
    
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration using HSE
  * @retval None
  */
void SystemClock_Config_HSE(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource clocks dividers
  */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_RC64MPLL;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_RC64MPLL_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WAIT_STATES_1) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief System Clock Configuration using HSI
  * @retval None
  */
void SystemClock_Config_HSI(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

/** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
 
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  } 
  
  /** Configure the SYSCLKSource and SYSCLKDivider
  */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
   
  /** Configure the SMPS frequency divider */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInit.SmpsDivSelection = RCC_SMPSCLK_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Radio Initialization Function
  * @param  None
  * @retval None
  */
static void MX_MRSUBG_Init(void)
{
  /* USER CODE BEGIN Radio_Init 1 */
  
  /* USER CODE END Radio_Init 1 */
  
  /* Configures the radio parameters */
  SMRSubGConfig_t MRSUBG_RadioInitStruct = {0};
  
  MRSUBG_RadioInitStruct.lFrequencyBase = 868000000;
  MRSUBG_RadioInitStruct.xModulationSelect = MOD_2FSK;
  MRSUBG_RadioInitStruct.lDatarate = 38400;
  MRSUBG_RadioInitStruct.lFreqDev = 20000;
  MRSUBG_RadioInitStruct.lBandwidth = 100000;
  MRSUBG_RadioInitStruct.dsssExp = 0;
  MRSUBG_RadioInitStruct.outputPower = 0;
  MRSUBG_RadioInitStruct.PADrvMode = PA_DRV_TX_HP;
  HAL_MRSubG_Init(&MRSUBG_RadioInitStruct);
  
  /* Configures the packet parameters */
  MRSubG_PcktBasicFields_t MRSUBG_PacketSettingsStruct = {0};
  
  MRSUBG_PacketSettingsStruct.PreambleLength = PREAMBLE_BYTE(4);
  MRSUBG_PacketSettingsStruct.PostambleLength = PREAMBLE_BYTE(0);
  MRSUBG_PacketSettingsStruct.SyncLength = SYNC_BYTE(4);
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
  
  /* USER CODE BEGIN Radio_Init 2 */
  
  /* USER CODE END Radio_Init 2 */
}

/**
  * @brief MRSUBG_TIMER Initialization Function
  * @param None
  * @retval None
  */
static void MX_MRSUBG_TIMER_Init(void)
{

  /* USER CODE BEGIN MRSUBG_TIMER_Init 0 */

  /* USER CODE END MRSUBG_TIMER_Init 0 */

  MRSUBG_TIMER_InitTypeDef MRSUBG_TIMER_InitStruct = {0};

  /* USER CODE BEGIN MRSUBG_TIMER_Init 1 */

  /* USER CODE END MRSUBG_TIMER_Init 1 */
  while(LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC) < 0x10);
  MRSUBG_TIMER_InitStruct.HSE_XTAL_freq = 48000000;
  MRSUBG_TIMER_InitStruct.XTAL_StartupTime = 1260;
  MRSUBG_TIMER_InitStruct.enableInitialCalibration = FALSE;
  MRSUBG_TIMER_InitStruct.periodicCalibrationInterval = 0;
  HAL_MRSUBG_TIMER_Init(&MRSUBG_TIMER_InitStruct);
  timerHandle.callback = TimeoutCallback;
  timerHandle2.callback = TimeoutCallback2;
  timerHandle3.callback = TimeoutCallback3;
  
  /* USER CODE BEGIN MRSUBG_TIMER_Init 2 */

  /* USER CODE END MRSUBG_TIMER_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{
  __HAL_RCC_RTC_CLK_ENABLE();
  __HAL_RCC_CLEAR_IT(RCC_IT_RTCRSTRELRDY);
  __HAL_RCC_RTC_FORCE_RESET();
  __HAL_RCC_RTC_RELEASE_RESET();
  while(__HAL_RCC_GET_IT(RCC_IT_RTCRSTRELRDY) == 0){}
  __HAL_RCC_CLEAR_IT(RCC_IT_RTCRSTRELRDY);
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK) { Error_Handler(); }
  HAL_PWREx_EnableInternalWakeUpLine(PWR_WAKEUP_RTC, PWR_WUP_RISIEDG);
  
  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(RTC_IRQn);

  configRTCWakeupTimer(WAKEUP_RTC_TIMEOUT);
}

/**
  * @brief  Configures the RTC wakeup timer, to wakeup the device from DEEPSTOP
  *         at specified timeout
  * @param  timeout wakeup timeout expressed in ms
  */
void configRTCWakeupTimer(uint32_t timeout)
{
  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, ((timeout/1000)*2048), RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
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


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN assert_failed */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\n", file, line) */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
  }
  /* USER CODE END assert_failed */
}

#endif


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
/** \endcond
 */

