/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples/LCSC/LCSC_BasicDemo/Inc/main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_ll_comp.h"
#include "stm32wl3x_ll_dac.h"
#include "stm32wl3x_ll_lcsc.h"
#include "stm32wl3x_ll_pwr.h"
#include "stm32wl3x_ll_rcc.h"
#include "stm32wl3x_ll_bus.h"
#include "stm32wl3x_ll_system.h"
#include "stm32wl3x_ll_cortex.h"
#include "stm32wl3x_ll_utils.h"
#include "stm32wl3x_ll_dma.h"
#include "stm32wl3x_ll_dmamux.h"
#include "stm32wl3x_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "math.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/**
  * @brief
  */
#define LCSC_T_SCAN_MAX                             13888           /* !< T_scan_max to be adopted, depending on wheel speed, and on oversampling (x8 minimum), in usec */
                                                                    /* T_scan_max chosen based on a wheel speed 9 Hz, Fscan_min 72 Hz, T_scan_max 13.89 msec */
#define LCSC_CALIB_DAMP_THRES_FACTOR                0.8             /* !< to be tuned depending on the coil-wheel distance */
#define LCSC_CALIB_MAX_LCAB_CNT_BOUND_FACTOR        1.2             /* !< to be tuned depending on the coil-wheel distance */
#define LCSC_CALIB_MIN_LCAB_CNT_BOUND_FACTOR        0.8             /* !< to be tuned depending on the coil-wheel distance */
#define LCSC_TREC_VCM                               0x0A            /* !< value to be retrieved by testing on bench */

/**
  * @brief Toggle periods for various blinking modes
  */
#define LED_BLINK_VERY_FAST   50  /* Toggle period very fast (unit: ms) */
#define LED_BLINK_FAST       100  /* Toggle period fast (unit: ms) */
#define LED_BLINK_SLOW       500  /* Toggle period slow (unit: ms) */
#define LED_BLINK_ERROR     1000  /* Toggle period very slow for error case (unit: ms) */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void LCSC_CallbackClockWiseDetect(void);
void LCSC_CallbackAntiClockWiseDetect(void);
void LCSC_CallbackTamperDetect(void);
void LCSC_CallbackOutOfBoundDetect(void);
void LCSC_CallbackFallingEdgeLCDetect(void);
void LCSC_CallbackRisingEdgeLCDetect(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
