/**
  ******************************************************************************
  * @file    stm32wl3r_nucleo_conf.h
  * @author  MCD Application Team
  * @brief   STM32WL3R nucleo board configuration file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WL3R_NUCLEO_CONF_H
#define STM32WL3R_NUCLEO_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal.h"

/* Usage of nucleo board */
#define USE_NUCLEO_32      1U

/* Usage of COM feature */
#if (USE_VCP_CONNECTION == 1)
/* COM usage define */
#define USE_BSP_COM_FEATURE                 1U

/* COM log define */
#define USE_COM_LOG                         1U

#else
/* COM usage define */
#define USE_BSP_COM_FEATURE                 0U

/* COM log define */
#define USE_COM_LOG                         0U

#endif /* USE_VCP_CONNECTION */

/* Button interrupt priorities */
#define BSP_B1_IT_PRIORITY 0x0FUL  /* Default is lowest priority level */
#define BSP_B2_IT_PRIORITY 0x0FUL  /* Default is lowest priority level */
#define BSP_B3_IT_PRIORITY 0x0FUL  /* Default is lowest priority level */

#ifdef __cplusplus
}
#endif

#endif /* STM32WL3R_NUCLEO_CONF_H */
