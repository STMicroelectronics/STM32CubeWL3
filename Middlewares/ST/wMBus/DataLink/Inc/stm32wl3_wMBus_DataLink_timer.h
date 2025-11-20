/**
  ******************************************************************************
  * @file    stm32wl3_wMBus_DataLink_timer.h
  * @author  MCD Application Team
  * @brief   Header file of wM-Bus DataLink Timer module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WL3_WMBUS_DATALINK_TIMER_H
#define STM32WL3_WMBUS_DATALINK_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32WL3_wM-Bus
  * @{
  */

/** @addtogroup DataLink
  * @{
  */

/** @addtogroup Timer
  * @{
  */

/** @defgroup Timer_Private_Constants Timer Private Constants
  * @{
  */

/**
  * @}
  */

/** @defgroup Timer_Exported_Types Timer Exported Types
  * @{
  */

/**
  * @}
  */

/** @defgroup Timer_Exported_Variables Timer Exported Variables
  * @{
  */
extern VTIMER_HandleType_t timerHandle;  /*!< Handler for the first virtual timer */
extern uint8_t VTimer_Callback_flag;   /*!< Flag for the first virtual timer callback */
extern VTIMER_HandleType_t timerHandle2; /*!< Handler for the second virtual timer */
extern uint8_t VTimer_Callback2_flag;  /*!< Flag for the second virtual timer callback */
extern VTIMER_HandleType_t timerHandle3; /*!< Handler for the third virtual timer */
extern uint8_t VTimer_Callback3_flag;  /*!< Flag for the third virtual timer callback */
extern VTIMER_HandleType_t timerHandle4; /*!< Handler for the fourth virtual timer */
extern uint8_t VTimer_Callback4_flag;  /*!< Flag for the fourth virtual timer callback */
/**
  * @}
  */

/** @defgroup Timer_Exported_Functions Timer Exported Functions
  * @{
  */

/**
  * @brief Initializes the wM-Bus DataLink Timer module.
  */
void wMBus_DataLink_Timer_Init(void);

/**
  * @brief Timeout callback function for the first virtual timer.
  * @param timerHandle Pointer to the timer handle.
  */
void TimeoutCallback(void *timerHandle);

/**
  * @brief Timeout callback function for the second virtual timer.
  * @param timerHandle Pointer to the timer handle.
  */
void TimeoutCallback2(void *timerHandle);

/**
  * @brief Timeout callback function for the third virtual timer.
  * @param timerHandle Pointer to the timer handle.
  */
void TimeoutCallback3(void *timerHandle);

/**
  * @brief Timeout callback function for the fourth virtual timer.
  * @param timerHandle Pointer to the timer handle.
  */
void TimeoutCallback4(void *timerHandle);

/**
  * @brief Resets all virtual timer callback flags.
  */
void reset_VTimer_Flags(void);

/**
  * @brief Interrupt handler for the wM-Bus DataLink wakeup event.
  */
void wMBus_DataLink_WKUP_IRQHandler(void);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32WL3_WMBUS_DATALINK_TIMER_H */
