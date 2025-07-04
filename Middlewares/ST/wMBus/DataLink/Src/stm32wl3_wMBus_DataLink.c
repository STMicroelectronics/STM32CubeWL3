/**
  ******************************************************************************
  * @file    stm32wl3_wMBus_DataLink.c
  * @author  MCD Application Team
  * @brief   wM-Bus DataLink module driver.
  *          This file provides firmware functions to manage the wM-Bus DataLink driver
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

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal.h"
#include "stm32wl3_wMBus_DataLink.h"

#include <string.h>

/** @addtogroup STM32WL3_wM-Bus
  * @{
  */

/** @addtogroup DataLink
  * @{
  */

/** @defgroup DataLink_Private_Variables DataLink Private Variables
  * @{
  */
uint8_t *rxBuffer;        /*!< Pointer to the RX buffer */
uint16_t rxBufferLength;  /*!< Length of the RX buffer */
uint8_t new_Rx_frame = 0; /*!< Flag indicating a new RX frame */

static uint8_t wMBus_RxBuffer[WMBUS_RADIO_BUFFER_SIZE];   /*!< Static buffer for wM-Bus RX data */
uint16_t wMBus_RxBuffer_length = WMBUS_RADIO_BUFFER_SIZE; /*!< Length of the wM-Bus RX buffer */
int32_t wMBus_RssiDbm = 0;                                /*!< RSSI value in dBm for the received frame */
uint32_t wMBus_RxSync;                                    /*!< Synchronization word for RX */

static uint64_t TxCompletedTimeStamp = 0; /*!< Timestamp of the last completed TX */
static uint64_t PreTxTimestamp = 0;       /*!< Timestamp before the last TX */
static uint64_t RxCompletedTimeStamp = 0; /*!< Timestamp of the last completed RX */

wMBus_DataLink_frame_t RxFrame = {0}; /*!< DataLink Rx Frame*/
wMBus_DataLink_frame_t TxFrame = {0}; /*!< DataLink Tx Frame*/

static uint8_t access_no = 0x00;      /*!< Access number for the frame */

volatile wMBus_DataLink_state_t wMBus_DataLink_state = DL_SM_IDLE; /*!< Current state of the DataLink state machine */

wMBus_DataLink_attributes_t wMBus_DataLink_attr = 
{
  .DevType = METER,
  .MField = {0, 0},
  .AField = {0, 0, 0, 0, 0, 0},
  .LinkFrameFormat = WMBUS_FORMAT_A,
  .LinkFreqAccessCycle = 0,
  .LinkTimeout = 10000,
  .direction = METER_TO_OTHER,
  .submode = C_MODE
};

/**
  * @brief Define LinkTimeout in us depending on mode.
  */
const uint32_t wMBus_LinkTimeout[WMBUS_MODE_NUMBER] =
{
  20000, /**< T-mode: 20000us=20ms */
  15000, /**< S-mode: 15000us=15ms */
  10000, /**< S1M-mode: 10000us=10ms */
  10000, /**< C-mode: 10000us=10ms */
  10000, /**< N-mode - N2a: 10000us=10ms */
  10000, /**< N-mode - N2b: 10000us=10ms */
  10000, /**< N-mode - N2c: 10000us=10ms */
  10000, /**< N-mode - N2d: 10000us=10ms */
  10000, /**< N-mode - N2e: 10000us=10ms */
  10000, /**< N-mode - N2f: 10000us=10ms */
  10000  /**< N-mode - N2g: 10000us=10ms */
};
/**
  * @}
  */

/** @defgroup DataLink_Private_Functions DataLink Private Functions
  * @{
  */
/**
  * @brief Set the TX buffer based on the frame format.
  *
  * @param frame Frame be transmitted.
  * @param framePtr pointer to the frame buffer.
  * @return uint8_t length of the TX buffer.
  */
wMBus_DataLink_status_t _setFrame(wMBus_DataLink_frame_t *frame, uint8_t **framePtr);

/**
  * @brief Parse a received wM-Bus frame from the provided buffer.
  *
  * @param rxBuff Pointer to the buffer containing the received frame.
  * @param rxBufLen Length of the received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  */
wMBus_DataLink_status_t _receiveFrame(uint8_t *rxBuff, uint16_t rxBufLen);

wMBus_DataLink_status_t _setFrame(wMBus_DataLink_frame_t *frame, uint8_t **framePtr)
{
  static uint8_t localFrame[WMBUS_RADIO_BUFFER_SIZE];
  uint8_t *localFramePtr = localFrame;

  /* Length-field (L):(C + M + A) + (CI + all subsequent bytes (excluding crc) */
  frame->l_field = DATALINK_LAYER_OFFSET + CI_FIELD_SIZE + frame->data_field.size;

  if (wMBus_DataLink_attr.LinkFrameFormat == WMBUS_FORMAT_B)
  {
    /* Adapt L-field to B format: depend on the size of the packet add +2 CRC bytes or +4 */
    if (frame->l_field <= ((uint8_t)WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + (uint8_t)WMBUS_SECOND_BLOCK_SIZE_FORMAT_B - CRC_FIELD_SIZE))
    {
      frame->l_field += CRC_FIELD_SIZE;
    }
    else
    {
      frame->l_field += 2 * CRC_FIELD_SIZE;
    }
  }

  *localFramePtr++ = frame->l_field; /* L-field */

  /* ******************** 1st block ******************** */
  *localFramePtr++ = frame->c_field;        /* C-field */
  memcpy(localFramePtr, frame->m_field, 2); /* M-field */
  localFramePtr += 2;
  memcpy(localFramePtr, frame->a_field, 6); /* A-field */
  localFramePtr += 6;

  /* ******************** 2nd block + data-field ******************** */
  *localFramePtr++ = frame->ci_field; /* CI-field */

  memcpy(localFramePtr, frame->data_field.payload, frame->data_field.size);

  *framePtr = localFrame;

  return LINK_STATUS_SUCCESS;
}

wMBus_DataLink_status_t _receiveFrame(uint8_t *rxBuff, uint16_t rxBuffLen)
{
  int16_t dataBytesRemaining = rxBuffLen;
  uint8_t blockDataSize = 0;

  if (wMBus_DataLink_attr.LinkFrameFormat == WMBUS_FORMAT_A)
  {
    if (rxBuffLen < (WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - CRC_FIELD_SIZE))
    {
      return LINK_STATUS_FRAME_ERROR;
    }
    RxFrame.l_field = rxBuff[0];            /* L-field */
    RxFrame.c_field = rxBuff[1];            /* C-field */
    memcpy(RxFrame.m_field, &rxBuff[2], 2); /* M-field */
    memcpy(RxFrame.a_field, &rxBuff[4], 6); /* A-field */

    /* Update remaining bytes considering 1st block size(12bytes) without CRC(2bytes) */
    dataBytesRemaining = dataBytesRemaining - (WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - CRC_FIELD_SIZE);
    if (dataBytesRemaining < 0)
    {
      return LINK_STATUS_FRAME_ERROR;
    }

    /* ******************** 2nd block ******************** */
    RxFrame.ci_field = rxBuff[10]; /* CI-field */

    dataBytesRemaining = dataBytesRemaining - CI_FIELD_SIZE;
    memcpy(RxFrame.data_field.payload, &rxBuff[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + CI_FIELD_SIZE - CRC_FIELD_SIZE], dataBytesRemaining);
    RxFrame.data_field.size = dataBytesRemaining;
  }
  else if (wMBus_DataLink_attr.LinkFrameFormat == WMBUS_FORMAT_B)
  {
    if (rxBuffLen < WMBUS_FIRST_BLOCK_SIZE_FORMAT_B)
    {
      return LINK_STATUS_FRAME_ERROR;
    }
    RxFrame.l_field = rxBuff[0];            /* L-field */
    RxFrame.c_field = rxBuff[1];            /* C-field */
    memcpy(RxFrame.m_field, &rxBuff[2], 2); /* M-field */
    memcpy(RxFrame.a_field, &rxBuff[4], 6); /* A-field */

    /* Update remaining bytes considering 1st block size(10bytes) */
    dataBytesRemaining = dataBytesRemaining - WMBUS_FIRST_BLOCK_SIZE_FORMAT_B;

    /* ******************** 2nd block ******************** */
    RxFrame.ci_field = rxBuff[10]; /* CI-field */

    /* Determine second block size (118 bytes) - CI-field(1byte) */
    if (dataBytesRemaining <= (WMBUS_SECOND_BLOCK_SIZE_FORMAT_B - CI_FIELD_SIZE))
    {
      blockDataSize = dataBytesRemaining - CI_FIELD_SIZE;
    }
    else
    {
      blockDataSize = WMBUS_SECOND_BLOCK_SIZE_FORMAT_B - CI_FIELD_SIZE;
    }
    memcpy(RxFrame.data_field.payload, &rxBuff[WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + CI_FIELD_SIZE], blockDataSize); /* Data-field */
    RxFrame.data_field.size = blockDataSize;                                                                     /* Data-field size */

    /* Update remaining bytes considering 2nd block size(118bytes) */
    dataBytesRemaining = dataBytesRemaining - blockDataSize + CI_FIELD_SIZE;

    /* ******************** Optional block ******************** */
    if (dataBytesRemaining)
    {
      /* Determine optional block size (128 bytes) */
      blockDataSize = dataBytesRemaining;
      memcpy(&RxFrame.data_field.payload[RxFrame.data_field.size], &rxBuff[(uint8_t)WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + (uint8_t)WMBUS_SECOND_BLOCK_SIZE_FORMAT_B], blockDataSize - CRC_FIELD_SIZE); /* Data-field */
      RxFrame.data_field.size = RxFrame.data_field.size + blockDataSize - CRC_FIELD_SIZE;                                                                                                          /* Data-field size */
    }
  }
  else
  {
    return LINK_STATUS_NOT_PERMITTED;
  }
  return LINK_STATUS_SUCCESS;
}
/**
  * @}
  */

/** @defgroup DataLink_Exported_Functions DataLink Exported Functions
  * @{
  */
wMBus_DataLink_status_t wMBus_DataLink_init(uint8_t *m_field, uint8_t *a_field, wMBus_Phy_meter_modes_t dev_type, wMBus_Phy_submode_t submode, wMBus_Phy_frame_format_t frame_format, wMBus_Phy_direction_t direction)
{
  /* Static arrays to hold the last received M-field, A-field and payload */
  static uint8_t RxMField[2];
  static uint8_t RxAField[6];
  static uint8_t RxPayload[WMBUS_RADIO_BUFFER_SIZE];

  /* Set device type */
  wMBus_DataLink_attr.DevType = dev_type;

  /* Copy M-field and A-field attributes */
  memcpy(wMBus_DataLink_attr.MField, m_field, 2);
  memcpy(wMBus_DataLink_attr.AField, a_field, 6);

  /* Set frame format, direction and other link attributes */
  wMBus_DataLink_attr.LinkFrameFormat = frame_format;
  wMBus_DataLink_attr.direction = direction;
  wMBus_DataLink_attr.submode = submode;
  wMBus_DataLink_attr.LinkTimeout = wMBus_LinkTimeout[submode]; // Timeout value in milliseconds
  wMBus_DataLink_attr.LinkFreqAccessCycle = 0;

  /* Set RxFrame fields to point to the static arrays */
  RxFrame.m_field = RxMField;
  RxFrame.a_field = RxAField;
  RxFrame.data_field.payload = RxPayload;

  /* Set TxFrame fields to point to the wMBus_DataLink_attr fields */
  TxFrame.m_field = wMBus_DataLink_attr.MField;
  TxFrame.a_field = wMBus_DataLink_attr.AField;

  /* Set the Link Layer state to idle */
  wMBus_DataLink_state = DL_SM_IDLE;

  /* Initialize the wM-Bus PHY layer */
  wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
  wMBus_DataLink_Timer_Init();
  wMBus_Phy_register_LL_buffer(wMBus_RxBuffer, &wMBus_RxBuffer_length, (uint8_t *)&wMBus_DataLink_attr.LinkFrameFormat, &wMBus_RxSync, &wMBus_RssiDbm);

#ifdef WMBUS_DEBUG
  wMBus_Phy_initialize_debug_log();
#endif

  return LINK_STATUS_SUCCESS;
}

void wMBus_DataLink_state_machine(void)
{
  wMBus_DataLink_C_field_t resp = DL_NO_FRAME;
  uint8_t ret_val = SUCCESS;
  uint8_t ret_val2 = SUCCESS;
  uint8_t ret_val4 = SUCCESS;
  wMBus_Phy_tRO_t timing;
  timing = wMBus_Phy_getTiming();

  HAL_MRSUBG_TIMER_Tick();

  switch (wMBus_DataLink_state)
  {
  case DL_SM_IDLE:
    break;
  case DL_SM_SEND_NKE:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_SND_NKE(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SENT_SND_IR:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - TxCompletedTimeStamp));
    ret_val2 = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle2, timing.tRO_max - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - TxCompletedTimeStamp) + wMBus_DataLink_attr.LinkTimeout);
    HAL_MRSUBG_TIMER_Tick();
    if ((ret_val == SUCCESS) && (ret_val2 == SUCCESS))
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      while (!(VTimer_Callback_flag == 1) && !(VTimer_Callback2_flag == 0))
      { // wait until tRO_min expired and tRO_max not expired: This should not happen
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_startRx();

      while (!(VTimer_Callback2_flag == 1))
      { // until tRO_max expired
#ifdef PROCESS_FRAME
        wMBus_DataLink_receive_frame(&RxFrame);
#else
        wMBus_DataLink_receive_frame(&rxBuffer, &rxBufferLength);
#endif
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SEND_CNF_IR:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_CNF_IR(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SENT_SND_NR:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - TxCompletedTimeStamp));
    ret_val2 = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle2, timing.tRO_max - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - TxCompletedTimeStamp) + wMBus_DataLink_attr.LinkTimeout);
    HAL_MRSUBG_TIMER_Tick();
    if ((ret_val == SUCCESS) && (ret_val2 == SUCCESS))
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }

      while (!(VTimer_Callback_flag == 1) && !(VTimer_Callback2_flag == 0))
      { // wait until tRO_min expired and tRO_max not expired: This should not happen
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_startRx();

      while (!(VTimer_Callback2_flag == 1))
      { // until tRO_max expired
#ifdef PROCESS_FRAME
        wMBus_DataLink_receive_frame(&RxFrame);
#else
        wMBus_DataLink_receive_frame(&rxBuffer, &rxBufferLength);
#endif
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    if ((RxFrame.c_field == DL_SND_UD) || (RxFrame.c_field == DL_SND_UD2))
    {
      reset_VTimer_Flags();
      ret_val4 = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle4, (wMBus_Phy_getTxD() * 1000) - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - PreTxTimestamp));
      if (ret_val4 == SUCCESS)
      {
        while (!(VTimer_Callback4_flag == 1))
        { // wait until tXD expired
          HAL_MRSUBG_TIMER_Tick();
        }
        if ((RxFrame.c_field == DL_SND_UD))
        {
          wMBus_DataLink_send_ACK(&rxBuffer, &rxBufferLength);
        }
        else if (RxFrame.c_field == DL_SND_UD2)
        {
          wMBus_DataLink_send_RSP_UD(&rxBuffer, &rxBufferLength);
        }
        else if (RxFrame.c_field == DL_REQ_UD1)
        {
          resp = wMBus_DataLink_received_REQ_UD1();
          if (resp == DL_ACK)
          {
            wMBus_DataLink_send_ACK(&rxBuffer, &rxBufferLength);
          }
          else if (resp == DL_RSP_UD)
          {
            wMBus_DataLink_send_RSP_UD(&rxBuffer, &rxBufferLength);
          }
        }
        else if (RxFrame.c_field == DL_REQ_UD2)
        {
          wMBus_DataLink_send_RSP_UD(&rxBuffer, &rxBufferLength);
        }
        wMBus_Phy_abort();
        VTimer_Callback4_flag = 0;
      }
    }
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SEND_SND_UD:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_SND_UD(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SEND_SND_UD2:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_SND_UD2(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SEND_REQ_UD1:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_REQ_UD1(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SEND_REQ_UD2:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_REQ_UD2(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SEND_ACK:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - RxCompletedTimeStamp));
    HAL_MRSUBG_TIMER_Tick();
    if (ret_val == SUCCESS)
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_send_ACK(&rxBuffer, &rxBufferLength);
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_SM_SENT_ACC_DMD:
    reset_VTimer_Flags();
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, timing.tRO_min - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - TxCompletedTimeStamp));
    ret_val2 = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle2, timing.tRO_max - (HAL_MRSUBG_TIMER_GetCurrentSysTime() - TxCompletedTimeStamp) + wMBus_DataLink_attr.LinkTimeout);
    HAL_MRSUBG_TIMER_Tick();
    if ((ret_val == SUCCESS) && (ret_val2 == SUCCESS))
    {
      while (!(VTimer_Callback_flag == 1))
      { // wait until tRO_min expired
        HAL_MRSUBG_TIMER_Tick();
      }
      while (!(VTimer_Callback_flag == 1) && !(VTimer_Callback2_flag == 0))
      { // wait until tRO_min expired and tRO_max not expired: This should not happen
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_init(wMBus_DataLink_attr.submode, OTHER_TO_METER, wMBus_DataLink_attr.LinkFrameFormat);
      wMBus_DataLink_startRx();

      while (!(VTimer_Callback2_flag == 1))
      { // until tRO_max expired
#ifdef PROCESS_FRAME
        wMBus_DataLink_receive_frame(&RxFrame);
#else
        wMBus_DataLink_receive_frame(&rxBuffer, &rxBufferLength);
#endif
        HAL_MRSUBG_TIMER_Tick();
      }
      wMBus_Phy_abort();
      reset_VTimer_Flags();
    }
    wMBus_Phy_init(wMBus_DataLink_attr.submode, wMBus_DataLink_attr.direction, wMBus_DataLink_attr.LinkFrameFormat);
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  case DL_INVALID_EVENT:
    /* Invalid event, reset to idle */
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  default:
    /* Unknown state, reset to idle */
    wMBus_DataLink_state = DL_SM_IDLE;
    break;
  }
}

void wMBus_DataLink_send_frame(wMBus_DataLink_C_field_t c_field, wMBus_DataLink_CI_field_t ci_field, uint8_t *data, uint8_t length, uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  uint8_t *framePtr;
  uint8_t frameLength;

  /* Prepare the frame */
  TxFrame.c_field = c_field;
  TxFrame.ci_field = ci_field;
  TxFrame.data_field.payload = data;
  TxFrame.data_field.size = length;

  _setFrame(&TxFrame, &framePtr);
  frameLength = TxFrame.l_field + 1;

  wMBus_Phy_abort();
  PreTxTimestamp = HAL_MRSUBG_TIMER_GetCurrentSysTime();
  wMBus_Phy_prepare_Tx_CRC_mngt(framePtr, frameLength, wMBus_DataLink_attr.LinkFrameFormat);
  wMBus_Phy_start_transmission();
  while (!wMBus_Phy_check_radio_events(WMBUS_TX_COMPLETED));

  TxCompletedTimeStamp = HAL_MRSUBG_TIMER_GetCurrentSysTime();

  if (wMBus_DataLink_attr.DevType == METER)
  {
    switch (c_field)
    {
    case DL_SND_NR:
      wMBus_DataLink_state = DL_SM_SENT_SND_NR;
      wMBus_DataLink_state_machine();
      break;
    case DL_SND_IR:
      wMBus_DataLink_state = DL_SM_SENT_SND_IR;
      wMBus_DataLink_state_machine();
      break;
    case DL_ACC_DMD:
      wMBus_DataLink_state = DL_SM_SENT_ACC_DMD;
      wMBus_DataLink_state_machine();
      break;
    default:
      /* Unknown C-Field */
      break;
    }
  }

  /* Save RX pointers */
  *rxBuf = rxBuffer;
  *rxBufferLen = rxBufferLength;
}

wMBus_DataLink_status_t wMBus_DataLink_startRx()
{
  new_Rx_frame = 0;
  wMBus_Phy_abort();
  wMBus_Phy_prepare_Rx();
  wMBus_Phy_start_continuousRx();

  return LINK_STATUS_SUCCESS;
}

#ifdef PROCESS_FRAME
wMBus_DataLink_status_t wMBus_DataLink_receive_frame(wMBus_DataLink_frame_t *frame)
{
#else
wMBus_DataLink_status_t wMBus_DataLink_receive_frame(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
#endif
  wMBus_DataLink_C_field_t resp = DL_NO_FRAME;
  if (wMBus_DataLink_attr.DevType == OTHER)
  {
    wMBus_DataLink_startRx();
  }

  if (wMBus_DataLink_attr.DevType == METER)
  {
    /* wait for WMBUS_RX_VALID_SYNC_DETECTED event flag set */
    while (!wMBus_Phy_check_radio_events(WMBUS_RX_VALID_SYNC_DETECTED))
    {
      /* while no sync is received - we can check other event */
      HAL_MRSUBG_TIMER_Tick();
      if (VTimer_Callback2_flag == 1)
      {
        wMBus_Phy_abort();
        return PHY_RX_TIMEOUT;
      }
    }

    // allow extra time to receive the entire packet
    VTimer_Callback3_flag = 0;
    if (HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle3, LINK_TIMEOUT) != SUCCESS)
    {
      return PHY_RX_TIMEOUT;
    }
  }

  /* now wait for CRC OK*/
  while (!wMBus_Phy_check_radio_events(WMBUS_RX_COMPLETED_WITH_VALID_CRC))
  {
    /* while no valid packet is received - we can check other event */
    if (wMBus_Phy_check_radio_events(WMBUS_RX_COMPLETED_WITH_CRC_ERROR) == 1)
    {
      return PHY_RX_COMPLETED_WITH_CRC_ERROR;
    }
    if (wMBus_Phy_check_radio_events(WMBUS_FIRST_CRC_ERROR) == 1)
    {
      return PHY_FIRST_CRC_ERROR;
    }
    if (wMBus_Phy_check_radio_events(WMBUS_RX_VALID_HEADER_ERROR) == 1)
    {
      return PHY_RX_VALID_HEADER_ERROR;
    }
    if (wMBus_Phy_check_radio_events(WMBUS_RX_OVERFLOW_ERROR) == 1)
    {
      return PHY_RX_OVERFLOW_ERROR;
    }
    HAL_MRSUBG_TIMER_Tick();

    if (wMBus_DataLink_attr.DevType == METER)
    {
      if (VTimer_Callback3_flag == 1)
      {
        wMBus_Phy_abort();
        return PHY_RX_TIMEOUT;
      }
    }
    else if (wMBus_DataLink_attr.DevType == OTHER)
    {
      if (VTimer_Callback2_flag == 1)
      {
        wMBus_Phy_abort();
        return PHY_RX_TIMEOUT;
      }
    }
  }

  RxCompletedTimeStamp = HAL_MRSUBG_TIMER_GetCurrentSysTime();
  new_Rx_frame = 1;

  // A valid packet has been received, parse it into the RxFrame structure
  wMBus_DataLink_status_t status = _receiveFrame(wMBus_RxBuffer, wMBus_RxBuffer_length);
  if (status != LINK_STATUS_SUCCESS)
  {
    return status; // Return error if parsing the frame fails
  }
#ifdef PROCESS_FRAME
  // Copy the parsed frame into the provided RxFrame structure
  memcpy(frame, &RxFrame, sizeof(wMBus_DataLink_frame_t));
#else
  /* here it means valid packet has been received */
  *rxBuf = wMBus_RxBuffer;
  *rxBufferLen = wMBus_RxBuffer_length;
#endif

  if (wMBus_DataLink_attr.DevType == OTHER)
  {
    switch (RxFrame.c_field)
    {
    case DL_SND_IR:
      wMBus_DataLink_state = DL_SM_SEND_CNF_IR;
      wMBus_DataLink_state_machine();
      break;
    case DL_ACC_DMD:
      wMBus_DataLink_state = DL_SM_SEND_ACK;
      wMBus_DataLink_state_machine();
      break;
    case DL_SND_NR:
      resp = wMBus_DataLink_received_SND_NR();
      if (resp == DL_SND_UD)
      {
        wMBus_DataLink_state = DL_SM_SEND_SND_UD;
        wMBus_DataLink_state_machine();
      }
      else if (resp == DL_SND_UD2)
      {
        wMBus_DataLink_state = DL_SM_SEND_SND_UD2;
        wMBus_DataLink_state_machine();
      }
      else if (resp == DL_SND_NKE)
      {
        wMBus_DataLink_state = DL_SM_SEND_NKE;
        wMBus_DataLink_state_machine();
      }
      else if (resp == DL_REQ_UD1)
      {
        wMBus_DataLink_state = DL_SM_SEND_REQ_UD1;
        wMBus_DataLink_state_machine();
      }
      else if (resp == DL_REQ_UD2)
      {
        wMBus_DataLink_state = DL_SM_SEND_REQ_UD2;
        wMBus_DataLink_state_machine();
      }
      else if (resp == DL_NO_FRAME)
      {
        break;
      }
      break;
    default:
      return LINK_STATUS_SUCCESS;
      break;
    }
  }
  else
  {
    return LINK_STATUS_SUCCESS;
  }

  return LINK_STATUS_SUCCESS;
}

/* ******************** Callbacks ******************** */
__weak wMBus_DataLink_status_t wMBus_DataLink_send_CNF_IR(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_CNF_IR;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[12] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], 0x00, 0x00, 0x00, 0x00};
  uint8_t length = 12;
  if (RxFrame.data_field.size > 0)
  {
    data[8] = RxFrame.data_field.payload[0]; // Copy ACC from SND-IR
  }
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_ACK(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_ACK;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[12] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], 0x00, 0x00, 0x00, 0x00};
  uint8_t length = 12;
  if (RxFrame.data_field.size > 9)
  {
    data[8] = RxFrame.data_field.payload[8]; // Copy ACC from Received Frame
  }
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_RSP_UD(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_RSP_UD;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[15] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t length = 15;
  if (RxFrame.data_field.size > 9)
  {
    data[0] = RxFrame.data_field.payload[8]; // Copy ACC from Received Frame
  }
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_C_field_t wMBus_DataLink_received_SND_NR()
{
  return DL_SND_UD;
}

__weak wMBus_DataLink_C_field_t wMBus_DataLink_received_REQ_UD1()
{
  return DL_ACK;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_SND_UD(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_SND_UD;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;

  uint8_t data[15] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], access_no++, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t length = 15;
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_SND_UD2(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_SND_UD2;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[15] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], access_no++, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t length = 15;
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_SND_NKE(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_SND_NKE;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[12] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], access_no++, 0x00, 0x00, 0x00};
  uint8_t length = 12;
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_REQ_UD1(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_REQ_UD1;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[12] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], access_no++, 0x00, 0x00, 0x00};
  uint8_t length = 12;
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}

__weak wMBus_DataLink_status_t wMBus_DataLink_send_REQ_UD2(uint8_t **rxBuf, uint16_t *rxBufferLen)
{
  wMBus_DataLink_C_field_t c_field = DL_REQ_UD2;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  uint8_t data[12] = {RxFrame.a_field[0], RxFrame.a_field[1], RxFrame.a_field[2], RxFrame.a_field[3], RxFrame.m_field[0], RxFrame.m_field[1], RxFrame.a_field[4], RxFrame.a_field[5], access_no++, 0x00, 0x00, 0x00};
  uint8_t length = 12;
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, rxBuf, rxBufferLen);

  return LINK_STATUS_SUCCESS;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */