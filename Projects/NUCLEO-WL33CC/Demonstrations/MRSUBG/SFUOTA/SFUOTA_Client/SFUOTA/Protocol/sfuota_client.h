/**
  ******************************************************************************
  * @file    sfuota_client.h
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA client protocol interface.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SFUOTA_CLIENT_H
#define SFUOTA_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "sfuota_frames.h"
#include "sfuota_radio.h"
#include "sfuota_boot.h"
#include "sfuota_image.h"

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  SFUOTA client state-machine states.
  */
typedef enum
{
  SFUOTA_ST_CONNECTION = 0,  /*!< Connection handshake state */
  SFUOTA_ST_SIZE = 1,        /*!< Image-size exchange state */
  SFUOTA_ST_START = 2,       /*!< Download-start state */
  SFUOTA_ST_DATAREQ = 3,     /*!< Data-request state */
  SFUOTA_ST_GETDATA = 4,     /*!< Data-reception state */
  SFUOTA_ST_FLASHDATA = 5,   /*!< Flash-programming state */
  SFUOTA_ST_COMPLETE = 6,    /*!< Download-complete state */
} sfuota_state_t;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Initialize the SFUOTA state machine and its protocol context.
  * @retval 0U if the SFUOTA initialization completes successfully.
  * @retval Non-zero value if the SFUOTA module cannot be initialized.
  */
uint8_t SFUOTA_Init(void);

/**
  * @brief  Execute one SFUOTA state-machine step.
  * @retval 0U if the state-machine step completes successfully.
  * @retval Non-zero value if the SFUOTA transfer detects a protocol or flash-processing error.
  */
uint8_t SFUOTA_Tick(void);

/**
  * @brief  Forward MRSubG IRQ events to SFUOTA radio handling.
  */
void SFUOTA_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_CLIENT_H */
