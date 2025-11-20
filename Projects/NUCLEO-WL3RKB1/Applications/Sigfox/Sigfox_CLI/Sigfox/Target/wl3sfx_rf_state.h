/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    wl3sfx_rf_state.h
  * @author  GPM WBL Application Team
  * @brief   Management of the SubGHz state machine's state (Idle / TX / RX) for Sigfox on the STM32WL3.
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

#include <stdint.h>

#ifndef __WL3SFX_RF_STATE_H
#define __WL3SFX_RF_STATE_H

/**
 * @addtogroup WL3SFX
 * @{
 * @addtogroup WL3SFX_RF_STATE SubGHz RF State Control
 * @brief Manages the state of the SubGHz hardware's finite state machine (TX / RX / IDLE)
 * @{
 */

typedef enum { WL3SFX_RF_STATE_IDLE, WL3SFX_RF_STATE_TX, WL3SFX_RF_STATE_RX } WL3SFX_RF_State;

/** Result of state transition request */
typedef enum {
  /** State transition was successful */
  WL3SFX_RF_STATE_TRANSITION_OK = 0,

  /** State transition is illegal */
  WL3SFX_RF_STATE_TRANSITION_ILLEGAL,

  /** New state is not valid (not in WL3SFX_RF_State) */
  WL3SFX_RF_STATE_TRANSITION_INVALID
} WL3SFX_RF_StateTransitionResult;

/**
 * @brief Put SubGHz radio and FEM into specified RF state
 *
 * @param state New radio and FEM state
 *
 * @retval 0 if successful, 1 if state transition is illegal
 */
uint8_t wl3sfx_rf_state_transition(WL3SFX_RF_State state);

/** @}@} */

#endif
