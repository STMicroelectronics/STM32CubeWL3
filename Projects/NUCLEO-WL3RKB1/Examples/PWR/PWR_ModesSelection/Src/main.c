/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    PWR/PWR_ModesSelection/Src/main.c
  * @author  MCD Application Team
  * @brief   Select a power mode over UART for current consumption measurement.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

typedef enum
{
  MODE_RUN = '1',
  MODE_SLEEP = '2',
  MODE_DEEPSTOP_CLOCK_OFF = '3',
  MODE_DEEPSTOP_CLOCK_ON = '4',
  MODE_SHUTDOWN_BOR_OFF = '5',
  MODE_SHUTDOWN_BOR_ON = '6',
  MODE_ULTRA_DEEPSTOP_BOR_OFF = '7',
  MODE_ULTRA_DEEPSTOP_BOR_ON = '8',
} PowerMode_TypeDef;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
static void Console_Init(void);
static void Console_Print(const char *text);
static PowerMode_TypeDef SelectMode(void);
static void ConfigureMode(PowerMode_TypeDef mode);
static void PrepareMeasurement(void);
static void EnterMode(PowerMode_TypeDef mode);
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
  PowerMode_TypeDef mode;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  if (HAL_Init() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* USER CODE BEGIN 2 */
  Console_Init();
  mode = SelectMode();
  ConfigureMode(mode);

  Console_Print("\r\nSelection accepted. SYSCLK: 64 MHz; SMPS enabled; radio off.\r\n"
                "Measurement starts in 5 seconds. Disconnect debug/serial links.\r\n"
                "UART and LEDs will be disabled. RESET is the only exit.\r\n");
  HAL_Delay(5000);
  PrepareMeasurement();
  EnterMode(mode);
  Error_Handler();
  /* USER CODE END 2 */
}

/**
  * @brief  System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
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


/* USER CODE BEGIN 4 */

static void Console_Init(void)
{
  COM_InitTypeDef config = {0};

  config.BaudRate = 115200;
  config.WordLength = COM_WORDLENGTH_8B;
  config.StopBits = COM_STOPBITS_1;
  config.Parity = COM_PARITY_NONE;
  config.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &config) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
}

static void Console_Print(const char *text)
{
  uint16_t length = 0;

  while (text[length] != '\0')
  {
    length++;
  }
  if (HAL_UART_Transmit(&hcom_uart[COM1], (const uint8_t *)text, length, 1000) != HAL_OK)
  {
    Error_Handler();
  }
}

static PowerMode_TypeDef SelectMode(void)
{
  uint8_t selection;

  Console_Print("\r\nPWR_ModesSelection - ");
  Console_Print((const char *)BSP_GetBoardName());
  Console_Print("\r\n115200 baud, 8 data bits, no parity, 1 stop bit.\r\n"
                "1: Run (64 MHz, idle loop in Flash)\r\n"
                "2: Sleep (64 MHz, no periodic wakeup)\r\n"
                "3: DeepStop (slow clock OFF, SRAM retained)\r\n"
                "4: DeepStop (LSI ON, SRAM retained, no RTC)\r\n"
                "5: Shutdown (BOR OFF)\r\n"
                "6: Shutdown (BOR ON)\r\n");
  Console_Print("7: Ultra DeepStop (BOR OFF, WL3R only)\r\n"
                "8: Ultra DeepStop (BOR ON, WL3R only)\r\n");
  Console_Print("Select one digit; use RESET to select another mode: ");

  for (;;)
  {
    if (HAL_UART_Receive(&hcom_uart[COM1], &selection, 1, HAL_MAX_DELAY) != HAL_OK)
    {
      Error_Handler();
    }
    switch (selection)
    {
      case MODE_RUN:
      case MODE_SLEEP:
      case MODE_DEEPSTOP_CLOCK_OFF:
      case MODE_DEEPSTOP_CLOCK_ON:
      case MODE_SHUTDOWN_BOR_OFF:
      case MODE_SHUTDOWN_BOR_ON:
      case MODE_ULTRA_DEEPSTOP_BOR_OFF:
      case MODE_ULTRA_DEEPSTOP_BOR_ON:
        if (HAL_UART_Transmit(&hcom_uart[COM1], &selection, 1, 1000) != HAL_OK)
        {
          Error_Handler();
        }
        return (PowerMode_TypeDef)selection;

      case '\r':
      case '\n':
        break;

      default:
        Console_Print("\r\nInvalid selection. Select a digit from the menu: ");
        break;
    }
  }
}

static void ConfigureMode(PowerMode_TypeDef mode)
{
  RCC_OscInitTypeDef oscillator = {0};
  PWR_DEEPSTOPTypeDef deepstop = {0};
  PWR_SHUTDOWNTypeDef shutdown = {0};
  PWR_ULTRA_DEEPSTOPTypeDef ultraDeepstop = {0};

  /* The HAL LSE configuration disables LSI, so configure LSE first. */
  oscillator.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  oscillator.LSEState = RCC_LSE_OFF;
  if (HAL_RCC_OscConfig(&oscillator) != HAL_OK)
  {
    Error_Handler();
  }
  oscillator.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  oscillator.LSIState = (mode == MODE_DEEPSTOP_CLOCK_ON) ? RCC_LSI_ON : RCC_LSI_OFF;
  if (HAL_RCC_OscConfig(&oscillator) != HAL_OK)
  {
    Error_Handler();
  }

  switch (mode)
  {
    case MODE_RUN:
    case MODE_SLEEP:
      break;

    case MODE_DEEPSTOP_CLOCK_OFF:
    case MODE_DEEPSTOP_CLOCK_ON:
      deepstop.deepStopMode = (mode == MODE_DEEPSTOP_CLOCK_ON) ?
                             PWR_DEEPSTOP_WITH_SLOW_CLOCK_ON :
                             PWR_DEEPSTOP_WITH_SLOW_CLOCK_OFF;
      if (HAL_PWR_ConfigDEEPSTOP(&deepstop) != HAL_OK)
      {
        Error_Handler();
      }
      break;

    case MODE_SHUTDOWN_BOR_OFF:
    case MODE_SHUTDOWN_BOR_ON:
      shutdown.BORStatus = (mode == MODE_SHUTDOWN_BOR_ON) ? ENABLE : DISABLE;
      shutdown.WakeUpPinStatus = DISABLE;
      shutdown.WakeUpPol = PWR_WUP_FALLEDG;
      if (HAL_PWR_ConfigSHUTDOWN(&shutdown) != HAL_OK)
      {
        Error_Handler();
      }
      break;

    case MODE_ULTRA_DEEPSTOP_BOR_OFF:
    case MODE_ULTRA_DEEPSTOP_BOR_ON:
      ultraDeepstop.BORStatus = (mode == MODE_ULTRA_DEEPSTOP_BOR_ON) ? ENABLE : DISABLE;
      ultraDeepstop.WakeUpPinStatus = DISABLE;
      ultraDeepstop.WakeUpPol = PWR_WUP_FALLEDG;
      if (HAL_PWR_ConfigUltraDeepStop(&ultraDeepstop) != HAL_OK)
      {
        Error_Handler();
      }
      break;
    default:
      Error_Handler();
      break;
  }
}

static void PrepareMeasurement(void)
{
  /* Blocking UART transmission has completed before its clock is disabled. */
  if (BSP_COM_DeInit(COM1) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
  HAL_DBGMCU_DisableDBGSleepMode();
  HAL_DBGMCU_DisableDBGStopMode();
  HAL_PWREx_DisableDEEPSTOP2();
  HAL_PWREx_DisableDBGRetention();

  lowPowerIOSetup();

  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PORTA, GPIO_PIN_ALL);
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PORTB, GPIO_PIN_ALL);
  LL_PWR_DisableInternWU(LL_PWR_WAKEUP_ALL);
  LL_PWR_DisableIOWakeupSDN();
  LL_PWR_ClearInternalWakeupSource(LL_PWR_WAKEUP_ALL);
  LL_PWR_ClearWakeupSource(PWR_WAKEUP_PORTA, LL_PWR_WAKEUP_ALL);
  LL_PWR_ClearWakeupSource(PWR_WAKEUP_PORTB, LL_PWR_WAKEUP_ALL);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_DEEPSTOPF);

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_SYSCFG_CLK_DISABLE();

  /* Remove periodic activity and stale pending requests before WFI. */
  HAL_SuspendTick();
  __disable_irq();
  SysTick->CTRL = 0;
  SysTick->VAL = 0;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
  NVIC->ICER[0] = 0xFFFFFFFFUL;
  NVIC->ICPR[0] = 0xFFFFFFFFUL;
  __DSB();
  __ISB();
}

static void EnterMode(PowerMode_TypeDef mode)
{
  switch (mode)
  {
    case MODE_RUN:
      for (;;)
      {
        __NOP();
      }

    case MODE_SLEEP:
      for (;;)
      {
        HAL_PWR_EnterSLEEPMode();
      }

    case MODE_DEEPSTOP_CLOCK_OFF:
    case MODE_DEEPSTOP_CLOCK_ON:
      HAL_PWR_EnterDEEPSTOPMode();
      break;

    case MODE_SHUTDOWN_BOR_OFF:
    case MODE_SHUTDOWN_BOR_ON:
      HAL_PWREx_EnterSHUTDOWNMode();
      break;

    case MODE_ULTRA_DEEPSTOP_BOR_OFF:
    case MODE_ULTRA_DEEPSTOP_BOR_ON:
      HAL_PWR_EnterUltraDeepStopMode();
      break;
    default:
      break;
  }
  /* A reset-only deep power mode must not return to active execution. */
  Error_Handler();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  BSP_LED_Init(LD3);
  BSP_LED_On(LD3);
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
  (void)file;
  (void)line;
  Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
