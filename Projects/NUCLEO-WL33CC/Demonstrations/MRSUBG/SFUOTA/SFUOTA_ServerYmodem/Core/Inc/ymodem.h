/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ymodem.h
  * @author  GPAM Wireless Application Team
  * @brief   YMODEM protocol interface.
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
/* USER CODE END Header */

#ifndef __YMODEM_H_
#define __YMODEM_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported constants --------------------------------------------------------*/
/* YMODEM RX FIFO and packet framing sizes. */
#define RX_BUFFER_SIZE            (2048U + 8U)  /*!< YMODEM shared RX FIFO size in bytes. */

/* Packet sequence fields ----------------------------------------------------*/
#define PACKET_SEQNO_INDEX        (1U)     /*!< Packet sequence number byte index. */
#define PACKET_SEQNO_COMP_INDEX   (2U)     /*!< Packet sequence complement byte index. */

/* Packet layout and payload sizes ------------------------------------------*/
#define PACKET_HEADER             (3U)     /*!< Packet header size in bytes. */
#define PACKET_TRAILER            (2U)     /*!< Packet trailer size in bytes. */
#define PACKET_OVERHEAD           (PACKET_HEADER + PACKET_TRAILER)  /*!< Header + trailer bytes. */
#define PACKET_SIZE               (128U)   /*!< SOH payload size in bytes. */
#define PACKET_1K_SIZE            (1024U)  /*!< STX payload size in bytes. */

/* File metadata field limits -----------------------------------------------*/
#define FILE_NAME_LENGTH          (256U)   /*!< Maximum filename field length in header packet. */
#define FILE_SIZE_LENGTH          (16U)    /*!< Maximum filesize field length in header packet. */

/* YMODEM control bytes ------------------------------------------------------*/
#define SOH                       (0x01U)  /*!< Start of 128-byte data packet. */
#define STX                       (0x02U)  /*!< Start of 1024-byte data packet. */
#define EOT                       (0x04U)  /*!< End of transmission. */
#define ACK                       (0x06U)  /*!< Acknowledge. */
#define NAK                       (0x15U)  /*!< Negative acknowledge. */
#define CA                        (0x18U)  /*!< Cancel transfer control byte. */
#define CRC16                     (0x43U)  /*!< 'C': request 16-bit CRC. */

#define ABORT1                    (0x41U)  /*!< 'A': abort by user. */
#define ABORT2                    (0x61U)  /*!< 'a': abort by user. */

/* YMODEM receive timing and retry policy -----------------------------------*/
#define NAK_TIMEOUT               (500U)   /*!< Receive timeout in milliseconds. */
#define MAX_ERRORS                (5U)     /*!< Maximum consecutive packet errors before abort. */

/* YMODEM receive state return codes. */
#define YMODEM_ABORTED            (0x01)   /*!< Transfer aborted by peer/user. */
#define YMODEM_CONTINUE           (0x02)   /*!< Transfer step completed, continue receiving. */
#define YMODEM_TOO_MANY_ERRORS    (0x03)   /*!< Aborted after exceeding retry limit. */
#define YMODEM_DONE               (0x04)   /*!< End-of-transfer received successfully. */
#define YMODEM_NO_FILE            (0x05)   /*!< Empty filename packet received. */
#define YMODEM_INVALID_FILE_SIZE  (0x06)   /*!< Invalid or unsupported file size field. */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Sends YMODEM ACK control byte.
  * @retval None.
  */
void Ymodem_SendAck(void);

/**
  * @brief  Aborts the current YMODEM session by sending CA twice.
  * @retval None.
  */
void Ymodem_Abort(void);

/**
  * @brief  Initializes YMODEM receive buffer state.
  * @retval None.
  */
void Ymodem_Init(void);

/**
  * @brief  Receives one YMODEM packet step.
  * @param  buf Pointer to output buffer for payload bytes.
  * @param  buf_size Size of @p buf in bytes.
  * @param  size Pointer receiving payload/file size information.
  * @param  packets_received Current expected packet index (0 for header packet).
  * @retval YMODEM_CONTINUE when packet step is processed and transfer continues.
  * @retval YMODEM_DONE when sender signals end of transmission.
  * @retval YMODEM_ABORTED when transfer is aborted.
  * @retval YMODEM_TOO_MANY_ERRORS when retry limit is exceeded.
  * @retval YMODEM_NO_FILE when empty filename packet is received.
  */
int32_t Ymodem_Receive(uint8_t *buf, uint32_t buf_size, uint32_t *size, uint32_t packets_received);

/* Exported variables --------------------------------------------------------*/
extern uint8_t ymodem_rx_buffer[RX_BUFFER_SIZE];      /*!< Shared YMODEM UART RX byte buffer. */
extern uint32_t ymodem_rx_buffer_size;                /*!< Number of unread bytes in ymodem_rx_buffer. */
extern uint8_t *ymodem_rx_buffer_ptr;                 /*!< Read pointer inside ymodem_rx_buffer. */
extern uint8_t *ymodem_rx_buffer_tail_ptr;            /*!< Write pointer inside ymodem_rx_buffer. */

#endif /* __YMODEM_H_ */
