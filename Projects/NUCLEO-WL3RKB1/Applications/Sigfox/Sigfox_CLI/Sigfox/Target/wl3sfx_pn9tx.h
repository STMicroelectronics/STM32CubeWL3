/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    wl3sfx_pn9tx.h
  * @author  GPM WBL Application Team
  * @brief   Automatic, interrupt-based generator for a pseudorandom (PN9) sequence of Sigfox DBPSK symbols.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __WL3SFX_PN9TX
#define __WL3SFX_PN9TX

/**
 * @addtogroup WL3SFX
 * @{
 *
 * @addtogroup WL3SFX_PN9TX Pseudorandom TX Generator
 * @brief Automatic pseudorandom (PN9) transmit symbol sequence generator for continuous transmission emissions testing.
 * @{
 */

/**
 * @brief Start PN9 DBPSK generator engine.
 * Uses PN9 Polynomial to generate pseudorandom bit sequence and copies corresponding DBPSK symbols to transmit buffers.
 *
 * @retval None
 */
void wl3sfx_pn9tx_start(void);

/**
 * @brief Stop PN9 DBPSK generator engine
 *
 * @retval None
 */
void wl3sfx_pn9tx_stop(void);

/** @}@} */

#endif
