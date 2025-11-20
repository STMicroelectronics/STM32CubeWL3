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
#define MSG_SIZE 20
#define READ_DATA_SIZE 1024
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define READ_BUFFER_IS_EMPTY() (Read_ptr_in == Read_ptr_out)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SMRSubGConfig_t MRSUBG_RadioInitStruct;
MRSubG_PcktBasicFields_t MRSUBG_PacketSettingsStruct;

/* USER CODE BEGIN PV */
volatile FlagStatus xTxDoneFlag = RESET;
volatile FlagStatus xRxDoneFlag = RESET;
__attribute__((aligned(4))) uint8_t tx_data[MSG_SIZE];
__attribute__((aligned(4))) uint8_t rx_data[MSG_SIZE];
uint8_t tx_data_size=0;

static uint8_t  Read_Buffer[READ_DATA_SIZE];
static uint32_t Read_ptr_in = 0;
static uint32_t Read_ptr_out = 0;
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
  
  COM_InitTypeDef COM_Init;

  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);
  
  LL_USART_EnableIT_RXNE(USART1);  
  
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif  
  
  /* Payload length config */
  HAL_MRSubG_PktBasicSetPayloadLength(MSG_SIZE);
  
  /* Init & Enable IRQ */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E);
  
  /* Set the databuffer pointer and put the device in RX mode */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&rx_data);	
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);  
  
  printf("STM32WL3 Chat Application.\n\r");
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* While waiting for incoming messages, acquire text from the user */
    tx_data_size = 0;
    
    while(__io_getcharNonBlocking(&tx_data[tx_data_size]));
    
    /* reset the data to be sent */
    tx_data_size=0;
    
    printf("tx>");   
    
    do
    {
      while(!__io_getcharNonBlocking(&tx_data[tx_data_size]));
      
      if((char)tx_data[tx_data_size] != '\r' && (char)tx_data[tx_data_size] != '\n')
      {
        putchar(tx_data[tx_data_size]);
      }
      else
      {
        putchar('\n');
        putchar('\r');
        break;
      }
      tx_data_size++;
      /* if '\r' has been received */
    }while(tx_data_size != MSG_SIZE);
    
    /* payload length config */
    HAL_MRSubG_PktBasicSetPayloadLength(tx_data_size);
    
    __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
    
    /* wait for SABORT... */
    while ((__HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS() & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F) == 0) {};
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
    
    /* Set the pointer to the data buffer */
    __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&tx_data);
    
    /* Send the TX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);
    
    /* wait for TX done */
    while(!xTxDoneFlag);
    xTxDoneFlag = RESET;
    
    /* Revert the device to RX */
    HAL_MRSubG_PktBasicSetPayloadLength(MSG_SIZE);
    
    __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&rx_data);
    __HAL_MRSUBG_STROBE_CMD(CMD_RX); 
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

uint8_t __io_getcharNonBlocking(uint8_t *data)
{
  if (Read_Buffer_Pop(data))
    return 1;
  else
    return 0;
}

uint8_t Read_Buffer_Pop(uint8_t *byte)
{
  if(READ_BUFFER_IS_EMPTY())
    return 0;

  *byte = Read_Buffer[Read_ptr_out];
  Read_ptr_out++;

  if(Read_ptr_out == READ_DATA_SIZE)
    Read_ptr_out = 0;

  return 1;
}

void Read_Buffer_Push(uint8_t byte)
{
  Read_Buffer[Read_ptr_in] = byte;

  Read_ptr_in++;

  /* To avoid buffer overflow */
  if(Read_ptr_in == READ_DATA_SIZE)
  {
    Read_ptr_in = 0;
  }
}

void Process_InputData(uint8_t* data_buffer, uint16_t Nb_bytes)
{
  for(uint16_t i = 0; i < Nb_bytes; i++){
    Read_Buffer_Push(data_buffer[i]);
  }
}

void HAL_MRSubG_IRQ_Callback(void){
  uint32_t irq;
  
  irq = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();
  
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F) {
    
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F);
    
    uint16_t rxBytes = __HAL_MRSUBG_GET_DATABUFFER_COUNT();
    
    printf("\n\rrx>");
    
    /* Display the received message */
    for(uint8_t i=0 ; i<rxBytes; i++)
      printf("%c", rx_data[i]);
    
    printf("\n\rtx>");
    
    /* Restart RX */
    __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&rx_data);    
    __HAL_MRSUBG_STROBE_CMD(CMD_RX);    
  }
  
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F ) {
    
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);    
    
    /* set the tx_done_flag to manage the event in the main() */
    xTxDoneFlag = SET;
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
