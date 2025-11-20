/* USER CODE BEGIN Header */
/**
  ******************************************************************************
* @file    Sigfox/Sigfox_CLI/Core/Src/main.c
* @author  GPM WBL Application Team
* @brief   This code implements a a CLI to perform Sigfox evaluation.
  ******************************************************************************
  * @attention
  *
* Copyright (c) 2024-2025 STMicroelectronics.
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
#include "app_sigfox.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FW_VERSION "wl3_sigfox_cli_demo_1.0.1r"
#define READ_DATA_SIZE 1024
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define READ_BUFFER_IS_EMPTY() (Read_ptr_in == Read_ptr_out)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile uint8_t interactive = 1;

static uint8_t  Read_Buffer[READ_DATA_SIZE];
static uint32_t Read_ptr_in = 0;
static uint32_t Read_ptr_out = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
static uint8_t processCmdInput (uint8_t interactive);
uint8_t Read_Buffer_Pop(uint8_t *byte);
void Read_Buffer_Push(uint8_t byte);
extern uint8_t responsePrintf(const char* formatString, ...);
extern uint8_t responseHeaderPrintf(const char* formatString, ...);

/* Commands callbacks */
static void helpAction(void);
static void interactiveAction(void);
static void fwVersionAction(void);
static void rebootAction(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CommandEntry CommandTable[] = {
  { "help", helpAction, "", "List commands" }, { "interactive", interactiveAction, "u", "Set interactive mode" },
  { "fw_version", fwVersionAction, "", "Get fw version" }, { "reboot", rebootAction, "", "reboots the uC" }, COMMAND_TABLE,
  { NULL, NULL, NULL, NULL } // NULL action makes this a terminator
};
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
  MX_Sigfox_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_Sigfox_Process();

    /* USER CODE BEGIN 3 */
    if (processCmdInput(interactive)) {
      if (interactive) {
        printf(">");
      }
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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

static uint8_t processCmdInput(uint8_t interactive)
{
  static uint8_t buff[COMMAND_BUFFER_LENGTH];
  static uint16_t len;
  static uint16_t currIndex = 0;
  if (interactive) {
    // continue calling emberSerialReadPartialLine() until it returns success, indicating a full line has been read
    if (!serialReadPartialLine((char*)buff, COMMAND_BUFFER_LENGTH, &currIndex)) {
      return 0;
    }

    len = 0;
    // search forward looking for first CR, LF, or NULL to determine length of the string
    while ((buff[len] != '\n') && (buff[len] != '\r') && (buff[len] != '\0')) {
      len++;
    }
    buff[len++] = '\r'; // set the final char to be CR
    buff[len++] = '\n'; // set the final char to be NL

    currIndex = 0;
    return processCommandString(buff, len);
  } else {
    return processCommandInput();
  }
}

void interactiveAction(void)
{
  interactive = unsignedCommandArgument(0);
  responsePrintf("{&N utility call... &t2x}\r\n", "interactive", "mode", interactive);
}

void helpAction(void)
{
  CommandEntry* cmd;

  for (cmd = CommandTable; cmd->name != NULL; cmd++) {
    if (!CMD_HIDDEN(cmd)) {
      printf("%s %s - %s\r\n\r\n", cmd->name, cmd->argumentTypes, cmd->description);
    }
  }
}

void fwVersionAction(void)
{
  responsePrintf("{&N API call...&ts}\r\n", "fw_version", "value", FW_VERSION);
}

void rebootAction(void)
{
  responsePrintf("{&N API call...&ts}\r\n", "reboot");
#if !(defined(BLUENRG2_DEVICE) || defined(BLUENRG1_DEVICE))
  HAL_NVIC_SystemReset();
#else
  NVIC_SystemReset();
#endif
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
  __disable_irq();
  while (1)
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
