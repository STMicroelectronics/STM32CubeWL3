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
#define TX_BUFFER               350
#define PAYLOAD_LENGTH_FIX      256

#define CRC_POLYNOM 0x3D65


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define CEILING(X) (X-(uint16_t)(X) > 0 ? (uint16_t)(X+1) : (uint16_t)(X))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SMRSubGConfig MRSUBG_RadioInitStruct;
MRSubG_WMBUS_PcktFields MRSUBG_PacketSettingsStruct;

/* USER CODE BEGIN PV */
volatile FlagStatus xTxDoneFlag = RESET;
uint16_t cLField=20;
uint8_t vectcTxBuff[TX_BUFFER];
uint16_t nPcktLength;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_MRSUBG_Init(void);
/* USER CODE BEGIN PFP */
static uint16_t crcCalc(uint16_t crcReg, uint8_t crcData);
static uint16_t computeCrc(uint8_t *pStart, uint8_t cNBytes);
static void fitTxBuffer(void);
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

  
  /* IRQ Config */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E);
  HAL_NVIC_EnableIRQ(MR_SUBG_IRQn);
  
  /* Set the pointer to the data buffer */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&vectcTxBuff);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(cLField<20)
	cLField=20;

    nPcktLength = 1+cLField+2+2*(CEILING(((float)cLField-9)/16));

    /* fit the packet payload with data */
    fitTxBuffer();

    /* payload length config */
    HAL_MRSubG_PktBasicSetPayloadLength(nPcktLength);    

    printf("\r\nA data to transmit: [");
    for(uint16_t i=0;i<nPcktLength ;i++)
    {
	printf("%d ", vectcTxBuff[i]);
    }
    printf("]\r\n");
    printf("packet length= %d\r\n", nPcktLength);

    /* send the TX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);

    /* wait for TX done */
    while(!xTxDoneFlag);
    xTxDoneFlag = RESET;

    /* pause between two transmissions */
    HAL_Delay(1000);

    /* increment the payload length (overflowing it at PAYLOAD_LENGTH_FIX) */
    cLField++;
    if(cLField<20 || cLField>PAYLOAD_LENGTH_FIX)
	cLField=20;
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
  MRSUBG_PacketSettingsStruct.xWMbusSubmode = WMBUS_SUBMODE_S1_S2_LONG_HEADER;
  MRSUBG_PacketSettingsStruct.PreambleLength = PREAMBLE_LENGTH;
  MRSUBG_PacketSettingsStruct.PostambleLength = POSTAMBLE_LENGTH;
  HAL_MRSubG_WMBus_PacketInit(&MRSUBG_PacketSettingsStruct);
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
static uint16_t crcCalc(uint16_t crcReg, uint8_t crcData)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    /* If upper most bit is 1 */
    if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
	crcReg = (crcReg << 1)  ^ CRC_POLYNOM;
    else
	crcReg = (crcReg << 1);

    crcData <<= 1;
  }
  return crcReg;
}

static uint16_t computeCrc(uint8_t *pStart, uint8_t cNBytes)
{
  uint16_t  seed= 0x0000;

  for(uint8_t i=0; i<cNBytes ; i++)
  {
    seed = crcCalc(seed, pStart[i]);
  }
  seed =~seed;
  return seed;
}

static void fitTxBuffer(void)
{
  uint16_t nCrc, j=0, maxIndex, nFittedData;

  vectcTxBuff[j++] = cLField;   // L  field

  for(uint8_t i=0 ; i<9 ; i++)
  {
    vectcTxBuff[i+j] = i+1;
  }

  nCrc = computeCrc(&vectcTxBuff[j-1],10);
  j+=9;

  vectcTxBuff[j++] = (uint8_t)(nCrc>>8);
  vectcTxBuff[j++] = (uint8_t) nCrc;
  nFittedData=9;

  while(nFittedData<cLField)
  {
    if((cLField-nFittedData)/16 != 0)
      maxIndex = 16;
    else
      maxIndex = (cLField-nFittedData)%16;

    for(uint8_t i=0 ; i<maxIndex ; i++)
    {
      vectcTxBuff[j++] = i+1;
      nFittedData++;
    }

    nCrc = computeCrc(&vectcTxBuff[j-maxIndex],maxIndex);
    vectcTxBuff[j++] = (uint8_t)(nCrc>>8);
    vectcTxBuff[j++] = (uint8_t) nCrc;
  }
}

void HAL_MRSubG_IRQ_Callback(void){
  uint32_t irq;
  
  irq = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();
  
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F ) {
    
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
    
    /* set the tx_done_flag to manage the event in the main() */
    xTxDoneFlag = SET;
    
    BSP_LED_Toggle(LD2);
  }  
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