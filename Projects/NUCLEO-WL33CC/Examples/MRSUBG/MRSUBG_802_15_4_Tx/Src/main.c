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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SMRSubGConfig MRSUBG_RadioInitStruct;
MRSubG_802_15_4_PcktFields MRSUBG_PacketSettingsStruct;

/* USER CODE BEGIN PV */
uint8_t vectcTxBuff[MSG_SIZE]={0x11,0x22,0x33,0x44,0x55,0x0f,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0x10,0x11};

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
  BSP_LED_Init(LD2);

  COM_InitTypeDef COM_Init;
  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);

  /* Payload length config */  
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(MSG_SIZE);

    /* Set the pointer to the data buffer */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&vectcTxBuff);
  
  /* Set TX Mode to Normal Mode*/
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    BSP_LED_On(LD2);
    
    printf("TX - Data to transmit: [ ");
    
    for(uint8_t i=0 ; i<MSG_SIZE ; i++)
      printf("%02x ", vectcTxBuff[i]);
    printf("]\n\r");
    
    /* Send the TX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);    
    
    /* Wait for TX done */
    while((__HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS() & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F) == 0) {};
    
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);

    BSP_LED_Off(LD2);

    /* Pause between transmissions */
    HAL_Delay(500);
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
  MRSUBG_RadioInitStruct.lFrequencyBase = 865100000;
  MRSUBG_RadioInitStruct.xModulationSelect = MOD_2GFSK05;
  MRSUBG_RadioInitStruct.lDatarate = 100000;
  MRSUBG_RadioInitStruct.lFreqDev = 25000;
  MRSUBG_RadioInitStruct.lBandwidth = 150000;
  MRSUBG_RadioInitStruct.dsssExp = 0;
  MRSUBG_RadioInitStruct.outputPower = 14;
  MRSUBG_RadioInitStruct.PADrvMode = PA_DRV_TX_HP;
  HAL_MRSubG_Init(&MRSUBG_RadioInitStruct);

  /** Configures the packet parameters
  */
  MRSUBG_PacketSettingsStruct.Modulation = MOD_2FSK;
  MRSUBG_PacketSettingsStruct.PreambleLength = 0x08;
  MRSUBG_PacketSettingsStruct.FCSType = FCS_32BIT;
  MRSUBG_PacketSettingsStruct.Whitening = DISABLE;
  MRSUBG_PacketSettingsStruct.FecType = FEC_15_4_G_NONE;
  MRSUBG_PacketSettingsStruct.FrameLength = MSG_SIZE + FCS_SIZE;
  HAL_MRSubG_802_15_4_PacketInit(&MRSUBG_PacketSettingsStruct);
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
