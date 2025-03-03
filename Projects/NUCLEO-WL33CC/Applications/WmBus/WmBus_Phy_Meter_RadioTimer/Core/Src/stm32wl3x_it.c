/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32wl3x_it.c
 * @author  MCD Application Team
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
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
#include "stm32wl3x_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {
    /* USER CODE BEGIN SVCall_IRQn 0 */

    /* USER CODE END SVCall_IRQn 0 */
    /* USER CODE BEGIN SVCall_IRQn 1 */

    /* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
    /* USER CODE BEGIN PendSV_IRQn 1 */

    /* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
    /* USER CODE BEGIN SysTick_IRQn 0 */

    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();
    /* USER CODE BEGIN SysTick_IRQn 1 */

    /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32WL3x Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32wl3x.s).                    */
/******************************************************************************/

/**
 * @brief  This function handles MRSUBG interrupt request.
 * @param  None
 * @retval None
 */
void MRSUBG_IRQHandler(void) {
    /* USER CODE BEGIN MRSUBG_IRQn 0 */
    wmbphy_IRQHandler();
    /* USER CODE END MRSUBG_IRQn 0 */
    HAL_MRSubG_IRQHandler();
    /* USER CODE BEGIN MRSUBG_IRQn 1 */

    /* USER CODE END MRSUBG_IRQn 1 */
}

#ifdef RADIO_TIMER_ENABLED
void CPU_WKUP_IRQHandler(void) {
#ifdef SPECIAL_CODE
    uint32_t status;

    GPIOB->BRR = 1 << 4;
    LL_MRSUBG_TIMER_DisableCPUWakeupTimer(MR_SUBG_GLOB_RETAINED);
    /* Clear the interrupt */
    LL_MRSUBG_TIMER_ClearFlag_CPUWakeup(MR_SUBG_GLOB_MISC);
    status = LL_MRSUBG_TIMER_IsActiveFlag_CPUWakeup(MR_SUBG_GLOB_MISC);
#ifdef NO_DEEPSTOP
    HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_500us);
#else
    HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_10s);
#endif
    GPIOB->BSRR = 1 << 4;
#else
    HAL_MRSUBG_TIMER_TimeoutCallback();

#endif
    /* USER CODE BEGIN CPU_WKUP_IRQn 1 */

    /* USER CODE END CPU_WKUP_IRQn 1 */
}
#endif

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */