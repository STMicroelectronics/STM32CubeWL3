/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples/LCSC/LCSC_BasicDemo/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to use a LCSC peripheral.
  *          This example is based on the STM32WL3x LCSC HAL API.
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
static volatile uint32_t lcscFlagCounter_ClockWiseDetect = 0;
static volatile uint32_t lcscFlagCounter_AntiClockWiseDetect = 0;
static volatile uint32_t lcscFlagCounter_TamperDetect = 0;
static volatile uint32_t lcscFlagCounter_OutOfBoundDetect = 0;
static volatile uint32_t lcscFlagCounter_FallingEdgeDetect = 0;
static volatile uint32_t lcscFlagCounter_RisingEdgeDetect = 0;

static volatile int8_t show_update = 0;
static volatile int8_t calibration_value = 0;
static volatile uint32_t slow_clock_period = 0;
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

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_COMP1_Init(void);
static void MX_DAC1_Init(void);
static void MX_LCSC_Init(void);
/* USER CODE BEGIN PFP */

void LCSC_InitCal(void);

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

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

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
  MX_COMP1_Init();
  MX_DAC1_Init();
  MX_LCSC_Init();
  /* USER CODE BEGIN 2 */

  /* Initial calibration of the LCSC */
  LCSC_InitCal();

  /* Enable LCSC */
  LL_LCSC_Enable(LCSC);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(show_update == 1)
    {
      uint32_t cw = lcscFlagCounter_ClockWiseDetect;
      uint32_t aw = lcscFlagCounter_AntiClockWiseDetect;
      printf("Clockwise revolutions = %08d, Counterclockwise revolutions = %08d\n\r", cw, aw);
      show_update = 0;
    }

    if(calibration_value)
    {
      LL_LCSC_SetDampingThreshold_LCAB(LCSC, LL_LCSC_GetDampingThreshold_LCAB(LCSC) + calibration_value);
      LL_LCSC_SetMax_CounterOutOfBound(LCSC, LL_LCSC_GetMax_CounterOutOfBound(LCSC) + calibration_value);
      LL_LCSC_SetMin_CounterOutOfBound(LCSC, LL_LCSC_GetMin_CounterOutOfBound(LCSC) + calibration_value);
      LL_LCSC_SetDampingThreshold_LCT(LCSC, LL_LCSC_GetDampingThreshold_LCT(LCSC) + calibration_value);
      calibration_value = 0;
    }
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
  LL_RCC_KRM_SetRateMultiplier(12);
}

/**
  * @brief COMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_COMP1_Init(void)
{

  /* USER CODE BEGIN COMP1_Init 0 */

  /* USER CODE END COMP1_Init 0 */

  LL_COMP_InitTypeDef COMP_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB0_GRP1_EnableClock(LL_APB0_GRP1_PERIPH_COMP);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**COMP1 GPIO Configuration
  PA14   ------> COMP1_INP
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN COMP1_Init 1 */

  /* USER CODE END COMP1_Init 1 */
  COMP_InitStruct.PowerMode = LL_COMP_POWERMODE_HIGHSPEED;
  COMP_InitStruct.InputPlus = LL_COMP_INPUT_PLUS_IO1;
  COMP_InitStruct.InputMinus = LL_COMP_INPUT_MINUS_DAC1_CH1;
  COMP_InitStruct.InputHysteresis = LL_COMP_HYSTERESIS_MEDIUM;
  COMP_InitStruct.OutputPolarity = LL_COMP_OUTPUTPOL_NONINVERTED;
  COMP_InitStruct.OutputBlankingSource = LL_COMP_BLANKINGSRC_NONE;
  LL_COMP_Init(COMP1, &COMP_InitStruct);

  /* Wait loop initialization and execution */
  /* Note: Variable divided by 2 to compensate partially CPU processing cycles */
  __IO uint32_t wait_loop_index = 0;
  wait_loop_index = (LL_COMP_DELAY_VOLTAGE_SCALER_STAB_US * (SystemCoreClock / (1000000 * 2)));
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  /* USER CODE BEGIN COMP1_Init 2 */

  /* USER CODE END COMP1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  LL_DAC_InitTypeDef DAC_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB0_GRP1_EnableClock(LL_APB0_GRP1_PERIPH_DAC);

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC channel OUT1 config
  */
  DAC_InitStruct.TriggerSource = LL_DAC_TRIG_SOFTWARE;
  DAC_InitStruct.WaveAutoGeneration = LL_DAC_WAVE_AUTO_GENERATION_NONE;
  DAC_InitStruct.OutputBuffer = LL_DAC_OUTPUT_BUFFER_DISABLE;
  LL_DAC_Init(DAC1, LL_DAC_CHANNEL_1, &DAC_InitStruct);
  LL_DAC_DisableTrigger(DAC1, LL_DAC_CHANNEL_1);
  /* USER CODE BEGIN DAC1_Init 2 */

  /* Enable VCMBUFF output */
  LL_DAC_EnableVCMBufferOutput(DAC1);

  /* Configure DAC DHR register to choose DAC analog output level of DACOUT_COMPINMINUS using DAC channel data holding register */
  LL_DAC_ConvertData6RightAligned(DAC1,LL_DAC_CHANNEL_1, 0x20);

  /* DAC channel output to COMP INMINUS enable */
  LL_DAC_SetOutputConnection(DAC1,LL_DAC_CHANNEL_1,DAC_CR_CMPEN);

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief LCSC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LCSC_Init(void)
{

  /* USER CODE BEGIN LCSC_Init 0 */

  /* USER CODE END LCSC_Init 0 */

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_LSCO_SetSource(LL_RCC_LSCO_CLKSOURCE_HSI64M_DIV2048);

  /* Peripheral clock enable */
  LL_APB0_GRP1_EnableClock(LL_APB0_GRP1_PERIPH_LCSC);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**LCSC GPIO Configuration
  PA14   ------> LCSC_LCB
  PB1   ------> LCSC_LCA
  PB2   ------> LCSC_LCT
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_1|LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* LCSC interrupt Init */
  NVIC_SetPriority(LCSC_IRQn, 2);
  NVIC_EnableIRQ(LCSC_IRQn);
  NVIC_SetPriority(LCSC_LC_ACTIVITY_IRQn, 2);
  NVIC_EnableIRQ(LCSC_LC_ACTIVITY_IRQn);

  /* USER CODE BEGIN LCSC_Init 1 */
  
  /* Configure PB4 as output pin */
  /* This allows to discharge residual voltage on external capacitor Cext */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* To add a delay to discharge Cext */

  /* Configure PB4 as analog pin */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Since APC bit is set, PUB1, PUB2, PUA14, PUB4 and PDB1, PDB2,
    PDA14, PDB4 shall be disabled */
  LL_PWR_SetNoPullB(LL_GPIO_PIN_1);
  LL_PWR_SetNoPullB(LL_GPIO_PIN_2);
  LL_PWR_SetNoPullA(LL_GPIO_PIN_14);
  LL_PWR_SetNoPullB(LL_GPIO_PIN_4);
  LL_PWR_SetNoPullB(LL_PWR_GPIO_BIT_12|LL_PWR_GPIO_BIT_13);

  /* USER CODE END LCSC_Init 1 */
  LL_LCSC_SetMeasurementTime(LCSC, 0x05C);
  LL_LCSC_SetCaptureTime(LCSC, 0x9);
  LL_LCSC_SetInterCaptureTime(LCSC, 0x0);
  LL_LCSC_Set_VCMBuff_RecoveryTime(LCSC, 0xa);
  LL_LCSC_Set_VCMBuff_StartingTime(LCSC, 0x20);
  LL_LCSC_EnableIT_LCT(LCSC);
  LL_LCSC_EnableIT_ClockWise(LCSC);
  LL_LCSC_SetClockWiseTarget(LCSC, 0x1);
  LL_LCSC_EnableIT_AntiClockWise(LCSC);
  LL_LCSC_SetAntiClockWiseTarget(LCSC, 0x1);
  LL_LCSC_EnableIT_CounterOutOfBoundWakeup(LCSC);
  LL_LCSC_SetMin_CounterOutOfBound(LCSC, 0x0);
  LL_LCSC_SetMax_CounterOutOfBound(LCSC, 0x0);
  LL_LCSC_SetDampingThreshold_LCAB(LCSC, 0x48);
  LL_LCSC_SetLowPulseWidth_LCAB(LCSC, 0xf);
  LL_LCSC_SetMeasurementInterval_LCT(LCSC, 0x1);
  LL_LCSC_SetDampingThreshold_LCT(LCSC, 0x1);
  LL_LCSC_SetLowPulseWidth_LCT(LCSC, 0xf);
  /* USER CODE BEGIN LCSC_Init 2 */

  /* USER CODE END LCSC_Init 2 */

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
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
* @brief  LCSC Initial calibration
*         Variables that depend on the system: Ls value, Cs value, cnt_target
*         Goals: to set DAC_DHR value, TCAP value, TMEAS value, LCAB_DAMP_THRS value, LCT_DAMP_THRS value
* @retval None
*/
void LCSC_InitCal(void)
{
  static uint8_t tcap = 0;
  static uint32_t t_meas = 0;
  static uint8_t vcmp_tmp = 0;
  static uint8_t vcmp_start = 0x21;  /* 1 LSB over the Vbat/2 */
  static uint8_t vcmp_stop = 0x3F;   /* Maximum value for DAC_DHR */
  static uint8_t vcmp_step = 1;
  static double M_PI_VALUE = 3.14159f;   /* PI value */
  static double Ls = 470e-6;
  static double Cs = 560e-12;
  static uint32_t freqOsc;
  static uint8_t last_count = 0;
  static uint8_t cnt_target = 30;  /* Target of the oscillations count in non-metal case */
  static uint32_t f_slow_clock = 32768;
  static uint8_t lcab_damp_thrs = 0;
  static uint8_t max, min, new_damp_thres = 0;

  /* Set Tmeas to 0 in order to perform a one-shot measurement */
  LL_LCSC_SetMeasurementTime(LCSC, 0x0);

  freqOsc = (uint32_t) (1 / (2 * M_PI_VALUE * sqrt(Ls * Cs)));

  /* Capture window that contains around 100 LC oscillations */
  tcap = ((100 * f_slow_clock) / freqOsc) + 1;

  /* Set capture time starting value (in this way the cnt max is 100) */
  LL_LCSC_SetCaptureTime(LCSC, tcap);

  vcmp_tmp = vcmp_start;

  printf("Initial calibration start: expose LCA to plastic.\n\r");
  printf("The COMP threshold will be set in order to reach %d counts in plastic case.\n\r", cnt_target);

  /* This do-while cycle will set the DAC_DHR value based on the cnt_target */
  do
  {
      LL_DAC_ConvertData6RightAligned(DAC1, LL_DAC_CHANNEL_1, vcmp_tmp);
      LL_LCSC_Enable(LCSC);
      LL_mDelay(5);
      last_count = LL_LCSC_GetComparatorLastDampingCount_LCA(LCSC);
      printf("Last count: %d\n\r", last_count);
      LL_LCSC_Disable(LCSC);
      LL_mDelay(1);
      vcmp_tmp = vcmp_tmp + vcmp_step;
  }
  while ((last_count > cnt_target) && (vcmp_tmp <= vcmp_stop));

  if (vcmp_tmp == vcmp_stop)
  {
    /* Error during setting DAC threshold */
    while(1);
  }

  /* Calculate Tcap based on slow_clock_freq and cnt_target */
  /* 1 is added since a round is needed to be sure the Tcap covers the expected oscillations */
  tcap = (cnt_target * f_slow_clock / freqOsc) + 1;
  LL_LCSC_SetCaptureTime(LCSC, tcap);

  /* at this stage DAC_DHR is set, it is time to rotate clockwise the wheel for 4 rotations,
  and set LCAB_DAMP_THRES in the middle of the range */

  /* Calculate LCAB_DAMP_THRS based on a percentage of the cnt_target */
  lcab_damp_thrs = (uint8_t)(LCSC_CALIB_DAMP_THRES_FACTOR * last_count);
  LL_LCSC_SetDampingThreshold_LCAB(LCSC, lcab_damp_thrs);


  /* Restore the T_MEAS */
  t_meas = LCSC_T_SCAN_MAX /1000 * f_slow_clock /1000  - LCSC_TREC_VCM;
  LL_LCSC_SetMeasurementTime(LCSC, t_meas);
  LL_LCSC_Enable(LCSC);
  LL_LCSC_ClearFlag_CNT_OFB(LCSC);

  printf("COMP threshold set, rotate the wheel clockwise.\n\r");

  while(LL_LCSC_GetClockWise(LCSC) < 4)
  {}

  min = LL_LCSC_GetMin_Counter(LCSC);
  max = LL_LCSC_GetMax_Counter(LCSC);
  new_damp_thres = (max + min) / 2;

  LL_LCSC_SetDampingThreshold_LCAB(LCSC, new_damp_thres);

  /* Set the min and max out of bound values */
  LL_LCSC_SetMax_CounterOutOfBound(LCSC, (uint32_t)LCSC_CALIB_MAX_LCAB_CNT_BOUND_FACTOR * max);
  LL_LCSC_SetMin_CounterOutOfBound(LCSC, (uint32_t)LCSC_CALIB_MIN_LCAB_CNT_BOUND_FACTOR * min);

  /* Set LCT damp threshold as well */
  LL_LCSC_SetDampingThreshold_LCT(LCSC, new_damp_thres);

  /* Reset max and min count and clockwise count since their values are affected by the calibration procedure*/
  LL_LCSC_ClearFlag_CNT_OFB(LCSC);
  LL_LCSC_ClearFlag_CLKWISE(LCSC);

  printf("Damping threhsold set at the middle of max and min counts.\n\r");
  printf("Calibration done!\n\r");
}

void LCSC_CallbackClockWiseDetect(void)
{
  lcscFlagCounter_ClockWiseDetect += LL_LCSC_GetClockWise(LCSC);
  show_update = 1;
}

void LCSC_CallbackAntiClockWiseDetect(void)
{
  lcscFlagCounter_AntiClockWiseDetect += LL_LCSC_GetAntiClockWise(LCSC);
  show_update = 1;
}

void LCSC_CallbackTamperDetect(void)
{
  printf("Tampering detected!\n\r");
}

void LCSC_CallbackOutOfBoundDetect(void)
{
  uint8_t max = LL_LCSC_GetMax_Counter(LCSC);
  uint8_t min = LL_LCSC_GetMin_Counter(LCSC);

  if (max > LL_LCSC_GetMax_CounterOutOfBound(LCSC))
    calibration_value = max - LL_LCSC_GetMax_CounterOutOfBound(LCSC);
  else
    calibration_value = min - LL_LCSC_GetMin_CounterOutOfBound(LCSC);
}

void LCSC_CallbackFallingEdgeLCDetect(void)
{
  lcscFlagCounter_FallingEdgeDetect++;
  show_update = 1;
}

void LCSC_CallbackRisingEdgeLCDetect(void)
{
  lcscFlagCounter_RisingEdgeDetect++;
  show_update = 1;
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

  /* Turn on LED and remain in infinite loop */
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
     ex: printf("Wrong parameters value: file %s on line %d", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
