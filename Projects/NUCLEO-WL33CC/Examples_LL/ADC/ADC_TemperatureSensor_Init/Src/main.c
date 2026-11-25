/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples_LL/ADC/ADC_TemperatureSensor_Init/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to use a ADC peripheral to perform
  *          a single ADC conversion of the internal temperature sensor
  *          and to calculate the temperature in degree Celsius;
  *          This example is based on the STM32WLxx ADC LL API;
  *          Peripheral initialization done using LL unitary services functions.
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
LL_ADC_InitTypeDef ADC_InitStruct1;

/* USER CODE BEGIN PV */

/* Variables for ADC conversion data */
__IO uint32_t uhADCxConvertedData = VAR_CONVERTED_DATA_INIT_VALUE; /* ADC group regular conversion data */
__IO uint32_t uwADCxConvertedDataRaw = 0UL;                        /* ADC raw down-sampler data register */
__IO uint32_t uwADCxTempCalC30 = 0UL;                              /* Temperature sensor factory raw data at calibration */
__IO uint32_t uwADCxTempCalTck = 0UL;                              /* Temperature sensor factory calibration temperature */

/* Variables for ADC conversion data computation to physical values */
__IO  int16_t hADCxConvertedData_Temperature_DegreeCelsius = 0UL;  /* Value of temperature calculated from ADC conversion data (unit: degree Celsius) */
__IO  int16_t hADCxConvertedData_TemperatureTypical_DegreeCelsius = 0UL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

void Activate_ADC(void);
void ConversionStartPoll_ADC_GrpRegular(void);
void LED_On(void);
void LED_Off(void);
static void UART_SendString(const char *text);
static void UART_SendInt(int32_t value);
static void UART_SendUInt(uint32_t value);
static void UART_SendTemperatureMeasurement(void);
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
  LL_APB0_GRP1_EnableClock(LL_APB0_GRP1_PERIPH_SYSCFG);

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
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  /* Activate ADC */
  /* Perform ADC activation procedure to make it ready to convert. */
  Activate_ADC();

  UART_SendString("ADC temperature sensor example\r\n");
  UART_SendString("UART: USART1 PA1/PA15 115200 8N1\r\n");

  ConversionStartPoll_ADC_GrpRegular();
  (void)LL_ADC_DSGetOutputData(ADCx);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* Turn LED off before performing a new ADC conversion start */
    LED_Off();
    LL_mDelay(200);

    /* Init variable containing ADC conversion data */
    uhADCxConvertedData = VAR_CONVERTED_DATA_INIT_VALUE;

    /* Perform ADC group regular conversion start, poll for conversion        */
    ConversionStartPoll_ADC_GrpRegular();

    /* Retrieve ADC conversion data */
    /* (data scale corresponds to configured down-sampler output width) */
    uwADCxConvertedDataRaw = LL_ADC_DSGetOutputData(ADCx);
    uhADCxConvertedData = uwADCxConvertedDataRaw & __LL_ADC_DIGITAL_SCALE(ADC_CONVERTED_DATA_WIDTH);

    uwADCxTempCalC30 = *TEMPSENSOR_C30_ADDR;
    uwADCxTempCalTck = *TEMPSENSOR_TCK_ADDR;

    /* Computation of ADC conversions raw data to physical values             */
    hADCxConvertedData_Temperature_DegreeCelsius = __LL_ADC_CALC_TEMPERATURE(uhADCxConvertedData, ADC_CONVERTED_DATA_WIDTH);
    
    hADCxConvertedData_TemperatureTypical_DegreeCelsius = __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(uhADCxConvertedData, ADC_CONVERTED_DATA_WIDTH);

    UART_SendTemperatureMeasurement();

    /* Turn LED on after ADC conversion completion */
    LED_On();
    LL_mDelay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

   /* Set HSE Capacitor Tuning */
  LL_RCC_HSE_SetCapacitorTuning(CFG_HW_RCC_HSE_CAPACITOR_TUNE);

   /* Set HSE startup Current */
 LL_RCC_HSE_SetStartupCurrent(0);

   /* Set HSE Amplitude Threshold */
 LL_RCC_HSE_SetAmplitudeThreshold(0);

   /* Set HSE Current Control */
  LL_RCC_HSE_SetCurrentControl(40);

   /* Enable the HSE */
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
  }

  LL_FLASH_SetLatency(LL_FLASH_WAIT_STATES_1);

   /* Enable the RC64MPLL*/
  LL_RCC_RC64MPLL_Enable();

   /* Wait till RC64MPLL is ready */
  while(LL_RCC_RC64MPLL_IsReady() != 1)
  {
  }

  LL_RCC_SetRC64MPLLPrescaler(LL_RCC_RC64MPLL_DIV_1);

   /* Update the SystemCoreClock global variable */
  LL_SetSystemCoreClock(64000000);

   /* Configure the source of time base considering new system clocks settings*/
  LL_Init1msTick(64000000);
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  LL_RCC_SetSMPSPrescaler(LL_RCC_SMPS_DIV_2);

   /* Enable the KRM*/
  LL_RCC_KRM_Enable();
  LL_RCC_KRM_SetRateMultiplier(2);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_ADCDIG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_ADCANA);

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC
  */
  LL_ADC_SetDSDataOutputRatio(ADC1, LL_ADC_DS_RATIO_1);
  LL_ADC_SetDSDataOutputWidth(ADC1, LL_ADC_DS_DATA_WIDTH_12_BIT);
  ADC_InitStruct1.SequenceLength = 1;
  ADC_InitStruct1.ContinuousConvMode = DISABLE;
  ADC_InitStruct1.SamplingMode = LL_ADC_SAMPLING_AT_END;
  ADC_InitStruct1.SampleRate = LL_ADC_SAMPLE_RATE_140;
  ADC_InitStruct1.Overrun = LL_ADC_NEW_DATA_IS_LOST;
  LL_ADC_Init(ADC1, &ADC_InitStruct1);

  /** Configure Regular Channel
  */
  LL_ADC_SetSequencerRanks(ADC1, LL_ADC_RANK_1, LL_ADC_CHANNEL_TEMPSENSOR);
  LL_ADC_ConfigureCalibPoint(ADC1, LL_ADC_CALIB_POINT_1, 0xFFFUL, 0UL);
  /* USER CODE BEGIN ADC1_Init 2 */
  
  LL_ADC_ConfigureDSDataOutput(ADC1, ADC_CONVERTED_DATA_WIDTH, ADC_OVERSAMPLING_RATIO);
  LL_ADC_InvertOutputDiffModeDisable(ADC1);
  LL_ADC_InvertOutputSingleNegModeDisable(ADC1);

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA1   ------> USART1_TX
  PA15   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART1);
  LL_USART_ConfigAsyncMode(USART1);

  /* USER CODE BEGIN WKUPType USART1 */

  /* USER CODE END WKUPType USART1 */

  LL_USART_Enable(USART1);

  /* Polling USART1 initialisation */
  while((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1))))
  {
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  /**/
  LL_GPIO_ResetOutputPin(LED2_GPIO_Port, LED2_Pin);

  /**/
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Perform ADC activation procedure to make it ready to convert
  *         (ADC instance: ADC).
  * @param  None
  * @retval None
  */
void Activate_ADC(void)
{
  #if (USE_TIMEOUT == 1)
  uint32_t Timeout = 0U; /* Variable used for timeout management */
  #endif /* USE_TIMEOUT */
  
  if (LL_ADC_IsEnabled(ADCx) == 0)
  {
    /* Enable ADC */
    LL_ADC_Enable(ADCx);
    
    /* Poll for ADC ready to convert */
    #if (USE_TIMEOUT == 1)
    Timeout = ADC_ENABLE_TIMEOUT_MS;
    #endif /* USE_TIMEOUT */
    
    while (LL_ADC_IsEnabled(ADCx) == 0)
    {
    #if (USE_TIMEOUT == 1)
      /* Check Systick counter flag to decrement the time-out value */
      if (LL_SYSTICK_IsActiveCounterFlag())
      {
        if(Timeout-- == 0)
        {
          /* Error: Time-out */
          Error_Handler();
        }
      }
    #endif /* USE_TIMEOUT */
    }
    
  } 
}

/**
  * @brief  Perform ADC group regular conversion start, poll for conversion
  *         completion.
  *         (ADC instance: ADC).
  * @note   In case of continuous mode or conversion trigger set to 
  *         external trigger, ADC group regular conversion stop must be added.
  * @param  None
  * @retval None
  */
void ConversionStartPoll_ADC_GrpRegular(void)
{
  #if (USE_TIMEOUT == 1)
  uint32_t Timeout = 0U; /* Variable used for timeout management */
  #endif /* USE_TIMEOUT */
  
  if ((LL_ADC_IsEnabled(ADCx) == 1)                  &&
      (LL_ADC_IsStopConversionOngoing(ADCx) == 0)      )
  {
    LL_ADC_ClearActiveFlags(ADCx, LL_ADC_IRQ_FLAGS_MASK);
    LL_ADC_StartConversion(ADCx);
  }
  else
  {
    /* Error: ADC conversion start could not be performed */
    Error_Handler();
  }
  
  #if (USE_TIMEOUT == 1)
  Timeout = ADC_UNITARY_CONVERSION_TIMEOUT_MS;
  #endif /* USE_TIMEOUT */
  
  while (LL_ADC_IsActiveFlag_EODS(ADCx) == 0)
  {
  #if (USE_TIMEOUT == 1)
    /* Check Systick counter flag to decrement the time-out value */
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
      if(Timeout-- == 0)
      {
        /* Error: Time-out */
        Error_Handler();
      }
    }
  #endif /* USE_TIMEOUT */
  }
  
  /* Clear flag ADC group regular end of unitary conversion */
  LL_ADC_ClearFlag_EODS(ADCx);
  
}

/**
  * @brief  Turn-on LED2.
  * @param  None
  * @retval None
  */
void LED_On(void)
{
  /* Turn LED on */
  LL_GPIO_SetOutputPin(LED2_GPIO_Port, LED2_Pin);
}

/**
  * @brief  Turn-off LED2.
  * @param  None
  * @retval None
  */
void LED_Off(void)
{
  /* Turn LED off */
  LL_GPIO_ResetOutputPin(LED2_GPIO_Port, LED2_Pin);
}

/**
  * @brief  Send String.
  * @param  None
  * @retval None
  */
static void UART_SendString(const char *text)
{
  while (*text != '\0')
  {
    while (LL_USART_IsActiveFlag_TXE_TXFNF(USARTx) == 0)
    {
    }
    LL_USART_TransmitData8(USARTx, (uint8_t)*text);
    text++;
  }

  while (LL_USART_IsActiveFlag_TC(USARTx) == 0)
  {
  }
}

/**
  * @brief  Send Int Value.
  * @param  None
  * @retval None
  */
static void UART_SendInt(int32_t value)
{
  if (value < 0)
  {
    UART_SendString("-");
    value = -value;
  }

  UART_SendUInt((uint32_t)value);
}

/**
  * @brief  Send UInt Value.
  * @param  None
  * @retval None
  */
static void UART_SendUInt(uint32_t value)
{
  char buffer[10];
  uint32_t index = 0UL;

  if (value == 0UL)
  {
    UART_SendString("0");
    return;
  }

  while (value != 0UL)
  {
    buffer[index] = (char)('0' + (value % 10UL));
    value /= 10UL;
    index++;
  }

  while (index != 0UL)
  {
    index--;
    while (LL_USART_IsActiveFlag_TXE_TXFNF(USARTx) == 0)
    {
    }
    LL_USART_TransmitData8(USARTx, (uint8_t)buffer[index]);
  }
}

/**
  * @brief  Send Temperature Measurement.
  * @param  None
  * @retval None
  */
static void UART_SendTemperatureMeasurement(void)
{
  UART_SendString("raw=");
  UART_SendUInt(uhADCxConvertedData);
  UART_SendString(", temp_cal=");
  UART_SendInt(hADCxConvertedData_Temperature_DegreeCelsius);
  UART_SendString(" C, temp_typ=");
  UART_SendInt(hADCxConvertedData_TemperatureTypical_DegreeCelsius);
  UART_SendString(" C\r\n");
}

/******************************************************************************/
/*   USER IRQ HANDLER TREATMENT                                               */
/******************************************************************************/


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* Toggle IO in an infinite loop */
  while (1)
  {
    LL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    LL_mDelay(LED_BLINK_ERROR);
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
     ex: printf("Wrong parameters value: file %s on line %d", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
