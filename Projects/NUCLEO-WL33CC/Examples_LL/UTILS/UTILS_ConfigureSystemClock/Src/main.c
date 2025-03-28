/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples_LL/UTILS/UTILS_ConfigureSystemClock/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to configure system clock using PLL with
  *          HSI as source clock through the STM32WL3x UTILS LL API.
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void     LED_Init(void);
void     MCO_ConfigGPIO(void);
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
  /* System started with default clock used after reset */
  /* Configure the system clock */
  SystemClock_Config();
  
  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
  
  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  /* Configure the RC64MPLL multiplication factor */
  LL_RCC_SetRC64MPLLPrescaler(LL_RCC_RC64MPLL_DIV_2);
  
  /* Update the SystemCoreClock global variable according to the new frequency */
  LL_SetSystemCoreClock(32000000);

  /* Set Systick to 1ms in using frequency set to SystemCoreClock */
  LL_Init1msTick(SystemCoreClock);

  /* Initialize LD1 */
  LED_Init();

  /* Configure SYSCLK for MCO */
  MCO_ConfigGPIO();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    LL_GPIO_TogglePin(LD1_GPIO_PORT, LD1_PIN);
    LL_mDelay(1000);

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

  LL_FLASH_SetLatency(LL_FLASH_WAIT_STATES_1);

   /* Enable the RC64MPLL*/
  LL_RCC_RC64MPLL_Enable();

   /* Wait till RC64MPLL is ready */
  while(LL_RCC_RC64MPLL_IsReady() != 1)

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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Initialize LD1.
  * @param  None
  * @retval None
  */
void LED_Init(void)
{
  /* Enable the LD1 Clock */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  /* Configure IO in output push-pull mode to drive external LD1 */
  LL_GPIO_SetPinMode(LD1_GPIO_PORT, LD1_PIN, LL_GPIO_MODE_OUTPUT);
  /* Reset value is LL_GPIO_OUTPUT_PUSHPULL */
  LL_GPIO_SetPinOutputType(LD1_GPIO_PORT, LD1_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  /* Reset value is LL_GPIO_SPEED_FREQ_LOW */
  LL_GPIO_SetPinSpeed(LD1_GPIO_PORT, LD1_PIN, LL_GPIO_SPEED_FREQ_LOW);
  /* Reset value is LL_GPIO_PULL_NO */
  LL_GPIO_SetPinPull(LD1_GPIO_PORT, LD1_PIN, LL_GPIO_PULL_NO);

  /* Select MCO clock source and prescaler */
  LL_RCC_ConfigMCO(LL_RCC_MCOSOURCE_SYSCLK, LL_RCC_MCO_DIV_1);
}

/**
  * @brief  Configure MCO pin (PA.5).
  * @param  None
  * @retval None
  */
void MCO_ConfigGPIO(void)
{
  /* MCO Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  /* Configure the MCO pin in alternate function mode */
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
  LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_0);
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
  while (1)
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
     ex: printf("Wrong parameters value: file %s on line %d", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
