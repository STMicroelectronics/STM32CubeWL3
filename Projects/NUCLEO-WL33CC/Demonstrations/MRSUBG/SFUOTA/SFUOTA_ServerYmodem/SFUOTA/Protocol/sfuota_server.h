/**
  ******************************************************************************
  * @file    sfuota_server.h
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA server protocol interface.
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
#ifndef SFUOTA_SERVER_H
#define SFUOTA_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "sfuota_frames.h"
#include "sfuota_radio.h"
#include "sfuota_image.h"
#include "ymodem.h"

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  SFUOTA server state-machine states.
  */
typedef enum
{
  SFUOTA_ST_CONNECTION = 0,  /*!< Connection handshake state */
  SFUOTA_ST_SIZE = 1,        /*!< Image-size exchange state */
  SFUOTA_ST_START = 2,       /*!< Download-start state */
  SFUOTA_ST_DATAREQ = 3,     /*!< Data-request state */
  SFUOTA_ST_SENDATA = 4,     /*!< Data-send state */
  SFUOTA_ST_COMPLETE = 6,    /*!< Transfer-complete state */
  SFUOTA_ST_ONGOING = 7,     /*!< Waiting for peer acknowledgement */
} sfuota_state_t;

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  Ymodem sub-state machine states used by the SFUOTA server.
  */
typedef enum
{
  SFUOTA_YMODEM_SIZE = 0U,      /*!< Receive filename/header packet and extract image size. */
  SFUOTA_YMODEM_LOAD = 1U,      /*!< Receive payload data packets. */
  SFUOTA_YMODEM_WAIT = 2U,      /*!< Wait for SFUOTA side to consume staged payload. */
  SFUOTA_YMODEM_CLOSE = 4U,     /*!< Final payload chunk reached, prepare transfer closure. */
  SFUOTA_YMODEM_COMPLETE = 5U,  /*!< Ymodem transfer completed successfully. */
  SFUOTA_YMODEM_ABORT = 9U,     /*!< Ymodem transfer aborted due to protocol error. */
} sfuota_ymodem_state_t;

/**
  * @brief  Ownership token shared between SFUOTA and Ymodem state machines.
  */
typedef enum
{
  SFUOTA_TOKEN_FREE = 0,      /*!< Shared context is free. */
  SFUOTA_TOKEN_YMODEM = 1,    /*!< Shared context is owned by Ymodem processing. */
  SFUOTA_TOKEN_TRANSFER = 2,  /*!< Shared context is owned by SFUOTA radio processing. */
} sfuota_token_t;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Preset the staged image with a known pattern for Ymodem-less debugging.
  */
void SFUOTA_Ymodem_Preset(void);

/**
  * @brief  Initialize the SFUOTA state machine and its protocol context.
  * @retval 0U if the SFUOTA initialization completes successfully.
  * @retval Non-zero value if the SFUOTA module cannot be initialized.
  */
uint8_t SFUOTA_Init(void);

/**
  * @brief  Execute one SFUOTA state-machine step.
  * @retval 0U if the state-machine step completes successfully.
  * @retval Non-zero value if the SFUOTA transfer detects a protocol error.
  */
uint8_t SFUOTA_Tick(void);

/**
  * @brief  Execute one Ymodem state-machine step.
  * @retval 0 when the step is processed normally.
  * @retval Non-zero Ymodem status/error code when a terminal condition is reached.
  */
int32_t SFUOTA_Ymodem_Tick(void);

/**
  * @brief  Forward MRSubG IRQ events to SFUOTA radio handling.
  */
void SFUOTA_IRQHandler(void);

/**
  * @brief  Get current SFUOTA protocol state-machine status.
  * @retval Current value of @ref sfuota_state_t.
  */
sfuota_state_t SFUOTA_GetState(void);

/**
  * @brief  Get current ownership token between SFUOTA and Ymodem state machines.
  * @retval Current token value from @ref sfuota_token_t.
  */
sfuota_token_t SFUOTA_GetTokenState(void);

/**
  * @brief  Push received UART bytes into the Ymodem RX ring/buffer.
  * @param  data_buffer Pointer to the received byte buffer.
  * @param  nb_bytes Number of bytes available in @p data_buffer.
  */
void SFUOTA_Ymodem_PushBytes(uint8_t *data_buffer, uint16_t nb_bytes);

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_SERVER_H */
