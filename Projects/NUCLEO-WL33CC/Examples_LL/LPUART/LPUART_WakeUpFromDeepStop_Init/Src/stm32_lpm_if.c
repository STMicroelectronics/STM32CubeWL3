/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32_lpm_if.c
  * @author  GPM WBL Application Team
  * @brief   Low layer function to enter/exit low power modes (stop, sleep)
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
#include "stm32_lpm_if.h"
#include "device_context_switch.h"

/** @addtogroup TINY_LPM_IF
  * @{
  */

/* USER CODE BEGIN include */
#include "stm32wl3x_ll_bus.h"
#include "stm32wl3x_ll_rcc.h"
#include "stm32wl3x_ll_pwr.h"
/* USER CODE END include */

/* Exported variables --------------------------------------------------------*/
/** @defgroup TINY_LPM_IF_Exported_variables TINY LPM IF exported variables
  * @{
  */

/**
 * @}
 */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN Private_Function_Prototypes */

/* USER CODE END Private_Function_Prototypes */
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN Private_Typedef */

/* USER CODE END Private_Typedef */
/* Private define ------------------------------------------------------------*/
typedef struct clockContextS
{
  uint8_t directHSEenabled;
  uint8_t LSEenabled;
  uint8_t LSIenabled;
  uint32_t clkDiv;
} clockContextT;

/* USER CODE BEGIN Private_Define */

/* USER CODE END Private_Define */
/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Private_Macro */

/* USER CODE END Private_Macro */
/* Private variables ---------------------------------------------------------*/
static apb0PeriphT apb0={0};
static apb1PeriphT apb1={0};
static apb2PeriphT apb2={0};
static ahb0PeriphT ahb0={0};
static cpuPeriphT  cpuPeriph={0};
static uint32_t    cStackPreamble[CSTACK_PREAMBLE_NUMBER];
static clockContextT clockContext;

/* USER CODE BEGIN Private_Variables */
extern void CPUcontextSave(void);

/* USER CODE END Private_Variables */

/** @addtogroup TINY_LPM_IF_Exported_functions
 * @{
 */

void PWR_EnterOffMode( void )
{
  /* USER CODE BEGIN PWR_EnterOffMode */
  /* USER CODE END PWR_EnterOffMode */

  /* Save the clock configuration */
  clockContext.directHSEenabled = FALSE;
  clockContext.LSEenabled = FALSE;
  clockContext.LSIenabled = FALSE;
  if (LL_RCC_DIRECT_HSE_IsEnabled())
  {
    clockContext.directHSEenabled = TRUE;
  }
  clockContext.clkDiv = LL_RCC_GetCLKSYSPrescalerStatus();
  if (LL_RCC_LSE_IsEnabled())
  {
    clockContext.LSEenabled = TRUE;
    /* Enable pull down for LSE pins */
    LL_PWR_DisableGPIOPullUp(LL_PWR_GPIO_B, LL_PWR_GPIO_BIT_12|LL_PWR_GPIO_BIT_13);
    LL_PWR_EnableGPIOPullDown(LL_PWR_GPIO_B, LL_PWR_GPIO_BIT_12|LL_PWR_GPIO_BIT_13);
  }
  if (LL_RCC_LSI_IsEnabled())
  {
    clockContext.LSIenabled = TRUE;
  }

  /* Save all the peripheral registers and CPU peripipheral configuration */
  apb0.deepstop_wdg_state = ENABLE;
  prepareDeviceLowPower(&apb0, &apb1, &apb2, &ahb0, &cpuPeriph, cStackPreamble);

  /* DEEPSTOP configuration */
  /* Disable the low speed clock if requested */
  LL_RCC_LSE_Disable();
  LL_RCC_LSI_Disable();

  /* Enable the GPIO retention in DEEPSTOP configuration */
  LL_PWR_EnableGPIORET();

  /* Disable DIRECT HSE configuration to allow DEEPSTOP request */
  if (LL_RCC_DIRECT_HSE_IsEnabled()) {
    LL_RCC_DIRECT_HSE_Disable();
  }

  /* Wait until  the low speed clock if OFF */
  while(LL_RCC_LSE_IsReady() || LL_RCC_LSI_IsReady()) {}

  /* Clear all the wake-up pin flags */
  LL_PWR_ClearWakeupSource(LL_PWR_WAKEUP_PORTA, LL_PWR_WAKEUP_ALL);
  LL_PWR_ClearWakeupSource(LL_PWR_WAKEUP_PORTB, LL_PWR_WAKEUP_ALL);
  LL_PWR_ClearInternalWakeupSource(LL_PWR_WAKEUP_ALL);

  /* Enable the device DEEPSTOP configuration */
  LL_PWR_SetPowerMode(LL_PWR_MODE_DEEPSTOP);

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

  /* Setup the SYS CLK DIV with the reset value */
  if (clockContext.clkDiv == LL_RCC_RC64MPLL_DIV_1)
  {
    LL_RCC_SetRC64MPLLPrescaler(LL_RCC_RC64MPLL_DIV_4);
  }

  /* Save the CPU context & Wait for Interrupt Request to enter in DEEPSTOP */
  CPUcontextSave();
}

void PWR_ExitOffMode( void )
{
  /* USER CODE BEGIN PWR_ExitOffMode */

  /* USER CODE END PWR_ExitOffMode */

  /* Restore low speed clock configuration */
  if (clockContext.LSEenabled == TRUE)
  {
    LL_PWR_SetNoPullB(LL_PWR_GPIO_BIT_12 | LL_PWR_GPIO_BIT_13);
    LL_RCC_LSE_Enable();
  }
  if (clockContext.LSIenabled == TRUE)
  {
    LL_RCC_LSI_Enable();
  }

  /* Clear SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

  /* Restore all the peripheral registers and CPU peripipheral configuration */
  restoreDeviceLowPower(&apb0, &apb1, &apb2, &ahb0, &cpuPeriph, cStackPreamble);

#if defined(PWR_CR2_GPIORET)
  /* Disable the GPIO retention at wake DEEPSTOP configuration */
  LL_PWR_DisableGPIORET();
#endif

  /* Restore the CLK SYS DIV */
  if (clockContext.clkDiv == LL_RCC_RC64MPLL_DIV_1)
  {
    LL_RCC_SetRC64MPLLPrescaler(LL_RCC_RC64MPLL_DIV_1);
  }

  /* Wait until the HSE is ready */
  while(LL_RCC_HSE_IsReady() == 0U);

  /* Restore the DIRECT_HSE configuration */
  if (clockContext.directHSEenabled == TRUE)
  {
    LL_RCC_DIRECT_HSE_Enable();
    LL_RCC_RC64MPLL_Disable();
  }
  if (clockContext.LSEenabled == TRUE)
  {
    /* Wait until the LSE is ready */
    while(LL_RCC_LSE_IsReady() == 0U);
  }
  if (clockContext.LSIenabled == TRUE)
  {
    /* Wait until the LSI is ready */
    while(LL_RCC_LSI_IsReady() == 0U);
  }
  if (LL_APB2_GRP1_IsEnabledClock(LL_APB2_GRP1_PERIPH_ALL))
  {
    /* Wait untile the ABSOLUTE TIME clock correctly */
    while(MR_SUBG_GLOB_MISC->ABSOLUTE_TIME < 0x10);
  }

}

/* USER CODE BEGIN Private_Functions */

/* USER CODE END Private_Functions */

/**
 * @}
 */

/**
 * @}
 */

