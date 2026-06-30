/**
  ******************************************************************************
  * @file    radio_ota.c
  * @author  GPAM Wireless Application Team
  * @brief   OTA server utilities for the WL33 SFUOTA server application.
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

/* Includes ------------------------------------------------------------------*/
#include "radio_ota.h"

/* Private define -----------------------------------------------------------*/
#define RFSEQ_IRQ_ERROR_MASK   (MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F     | \
                                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F   | \
                                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_AHB_ACCESS_ERROR_F | \
                                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_COMMAND_REJECTED_F)      /*!< Radio IRQ error sources consumed by OTA_Tick retry policy. */

#define CFG_OTA_REBOOT_VAL_MSG RAM_VR.bootloader_vr   /*!< Mapping of OTA retained message in SRAM. */
#define OTA_DEBUG_PERIOD_MS     (1000U)               /*!< Period for OTA debug heartbeat print. */
#define OTA_ACK_TIMEOUT_MS      (200U)                /*!< Timeout to retransmit handshake frame while waiting for peer ACK. */
#define OTA_TX_STUCK_TIMEOUT_MS (300U)                /*!< Watchdog timeout to recover from missed TX_DONE event. */
#define YMODEM_DUMP_LINE_BYTES  (16U)                 /*!< Number of bytes per output line in YMODEM payload dump. */
#define YMODEM_CAPTURE_MAX_BYTES (4096U)              /*!< Max number of payload bytes kept in RAM for Live Watch inspection. */

/* Private variables --------------------------------------------------------*/
/* Radio runtime buffers and deferred IRQ events. */
static __attribute__((aligned(4))) uint8_t RadioRxPacketBuffer[MAX_PACKET_LENGTH];  /*!< Radio RX packet buffer. */
static __attribute__((aligned(4))) uint8_t RadioTxPacketBuffer[MAX_PACKET_LENGTH];  /*!< Radio TX packet buffer. */
static uint16_t RxReceivedByteCount = 0U;                                           /*!< Number of bytes received in last RX event. */
static volatile uint8_t RxOkEventPending = FALSE;                                   /*!< RX OK event flag. */
static volatile uint8_t TxDoneEventPending = FALSE;                                 /*!< TX done event flag. */
static volatile uint8_t RxErrorEventPending = FALSE;                                /*!< RX error event flag. */
static volatile uint32_t RxErrorIrqStatus = 0U;                                     /*!< RX error IRQ status. */
static uint8_t RadioTxBusy = 0U;                                                    /*!< Radio TX busy flag. */
static uint32_t RadioTxStartMs = 0U;                                                /*!< Tick when current TX started, used to recover from missed TX_DONE. */
static uint8_t RadioRxArmed = 0U;                                                   /*!< Radio RX armed flag. */
static uint8_t RadioAbortInProgress = 0U;                                           /*!< RX abort in progress before switching to TX. */

/* OTA transport TX staging buffer and queue state. */
static uint8_t tx_buffer[MAX_PACKET_LENGTH];                                        /*!< OTA transport TX buffer. */
static uint8_t tx_length = 0U;                                                      /*!< Length of pending TX packet. */
static uint8_t tx_pending = 0U;                                                     /*!< TX pending flag. */

/* OTA protocol/session state and retry counters. */
static ota_state_machine_t ota_state_machine_g = OTA_CONNECTION;                    /*!< OTA protocol/session state. */
static ota_state_machine_t ota_waiting_ack_state_g = OTA_CONNECTION;                /*!< Handshake phase currently waiting for peer acknowledgement while in OTA_ONGOING. */
static uint16_t seq_num = 0U;                                                       /*!< Current OTA sequence number. */
static uint16_t seq_num_max = 0U;                                                   /*!< Maximum OTA sequence number. */
static uint16_t retry_connection = 0U;                                              /*!< Retry counter for CONNECTION handshake phase. */
static uint16_t retry_size = 0U;                                                    /*!< Retry counter for SIZE handshake phase. */
static uint16_t retry_start = 0U;                                                   /*!< Retry counter for START handshake phase. */
static uint16_t retry_datareq = 0U;                                                 /*!< Retry counter for DATAREQ phase. */

/* Server transfer/Ymodem context. */
static ota_app_token_t app_token = TOKEN_RELEASE;                                   /*!< Ownership token shared with Ymodem side. */
static ymodem_state_t ymodem_state_machine_g = YMODEM_SIZE_STATE;                   /*!< Current Ymodem sub-state. */
static uint8_t last_frame = 0U;                                                     /*!< Last-frame indicator for OTA SENDATA completion. */
static uint32_t data_len = 0U;                                                      /*!< Number of Ymodem payload bytes currently staged. */
static uint32_t image_size = 0U;                                                    /*!< Total image size parsed from Ymodem filename packet. */
static uint32_t ymodem_packet_index = 0U;                                           /*!< Current Ymodem packet index used by Ymodem_Receive sequencing. */
static uint8_t image[PACKET_1K_SIZE + 8U];                                          /*!< Ymodem payload staging chunk. */
static uint8_t image_tmp[BYTE_IN_FRAME + 8U];                                       /*!< Temporary OTA SENDATA frame payload buffer. */
static uint32_t ymodem_dump_offset = 0U;                                            /*!< Cumulative byte offset used while printing received YMODEM payload. */
uint8_t ymodem_capture_buffer[YMODEM_CAPTURE_MAX_BYTES];                            /*!< Persistent payload capture buffer for debugger inspection. */
uint32_t ymodem_capture_size = 0U;                                                  /*!< Number of valid bytes currently stored in ymodem_capture_buffer. */
uint32_t ymodem_capture_overflow = 0U;                                              /*!< Number of payload bytes dropped once capture buffer is full. */
static uint8_t ymodem_capture_printed = 0U;                                         /*!< Ensures capture dump is printed once per transfer. */

/* Debug telemetry counters. */
static uint32_t dbg_rx_ok_count = 0U;
static uint32_t dbg_tx_done_count = 0U;
static uint32_t dbg_rx_err_count = 0U;
static uint32_t dbg_last_print_ms = 0U;
static uint8_t dbg_first_ota_packet_queued = 0U;
//static uint32_t dbg_conn_retry_since_ms = 0U;

/* Private functions prototypes ----------------------------------------------*/

/* OTA protocol state callbacks. */
static uint8_t OTA_ConnectionCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_SizeCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_StartCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_DataRequestCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_SendDataCallback(const uint8_t *packet, uint16_t length);

/* OTA context and packet helpers. */
static void OTA_ResetServerContext(uint8_t full_reset);
static void OTA_QueueTx(const uint8_t *packet, uint8_t length);
static uint8_t OTA_ProcessReceivedPacket(const uint8_t *packet, uint16_t length);
static uint8_t OTA_GetTxPacket(const uint8_t **packet, uint8_t *length);

/* Radio operation helpers. */
static void OTA_Radio_StartReception(void);
static void OTA_Radio_StartTransmission(const uint8_t *packet, uint8_t length);
static void OTA_Radio_Service(void);
static void OTA_PrintYmodemPayloadChunk(const uint8_t *buffer, uint32_t length);
static void OTA_CaptureYmodemPayloadChunk(const uint8_t *buffer, uint32_t length);

/* Functions Definition -----------------------------------------------------*/

void preset_Ymodem_image(void)
{
  /* This function is used in YMODEM debug only mode to preset the image buffer and size with known values to allow testing the OTA state machine without needing an actual YMODEM transfer. */
  uint32_t i;

  image_size = 10240U; /* Example image size of 10 KB, can be set to any value up to the maximum supported by the application. */
  data_len = image_size;

  for (i = 0U; i < image_size; i++)
  {
    image[i % PACKET_1K_SIZE] = (uint8_t)(i & 0xFF); /* Fill the image buffer with a known pattern. */
  }
}


/**
  * @brief  Initializes OTA context and radio event handling.
  *
  * Resets OTA server runtime variables (including Ymodem sub-state and shared
  * ownership token), enables/clears required RFSEQ IRQ sources, initializes
  * software event flags, arms radio reception, and prints startup debug
  * information.
  * @param  None.
  * @retval 0U always.
  */
uint8_t OTA_Init(void)
{
  /* Reset full server context (OTA transfer + Ymodem staging ownership). */
  OTA_ResetServerContext(1U);

  /* Enable RX/TX and RF error IRQ sources used by OTA event processing. */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E |
  //                                  MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SABORT_DONE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_AHB_ACCESS_ERROR_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_COMMAND_REJECTED_E);

  /* Clear pending RFSEQ IRQ flags before starting OTA activity. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F |
                                    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F |
                                    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F |
                                    RFSEQ_IRQ_ERROR_MASK);

  /* Put radio in receive mode; first connection frame is queued in OTA_Tick(). */
 // OTA_Radio_StartReception();

  /* Print OTA server startup information. */
  APP_DBG_MSG("OTA manager server (version: %s)\r\n", RADIO_OTA_MANAGER_VERSION_STRING);
  return 0U;
}

/**
  * @brief  Runs one OTA state machine iteration and services radio events.
  *
  * Consumes pending RX/TX/error events latched by OTA_Radio_IRQHandler(),
  * executes tick-driven server-side OTA states, arbitrates access with the
  * Ymodem loader, and services radio TX/RX scheduling.
  *
  * @retval 0U always for this server transfer flow.
  */
uint8_t OTA_Tick(void)
{
  uint32_t now_ms = HAL_GetTick();

  /* Defer OTA activity while Ymodem owns the shared staging context. */
  if (app_token == TOKEN_TAKEN_YMODEM)
  {
    /* Still service radio so queued retransmissions can be transmitted. */
    OTA_Radio_Service();

    /* Keep heartbeat visible even while OTA work is deferred by token ownership. */
    if ((now_ms - dbg_last_print_ms) >= OTA_DEBUG_PERIOD_MS)
    {
      dbg_last_print_ms = now_ms;
      APP_DBG_MSG("OTA_SRV hb: ota=%u wait=%u ymodem=%u img=%lu data=%lu txp=%u txb=%u rxa=%u rxok=%lu hd=0x%02X txd=%lu rxerr=%lu\r\n",
                  (unsigned)ota_state_machine_g,
                  (unsigned)ota_waiting_ack_state_g,
                  (unsigned)ymodem_state_machine_g,
                  image_size,
                  data_len,
                  tx_pending,
                  RadioTxBusy,
                  RadioRxArmed,
                  dbg_rx_ok_count,
                  RadioRxPacketBuffer[0],
                  dbg_tx_done_count,
                  dbg_rx_err_count);
    }
    return 0U;
  }
  app_token = TOKEN_TAKEN_OTA;

  /* Handle RX-complete event latched by OTA_Radio_IRQHandler(). */
  if (RxOkEventPending == TRUE)
  {
    RxOkEventPending = FALSE;
    dbg_rx_ok_count++;
    RadioRxArmed = 0U;
   // BSP_LED_Toggle(LD1);
    RxReceivedByteCount = __HAL_MRSUBG_GET_DATABUFFER_COUNT();
    (void)OTA_ProcessReceivedPacket(RadioRxPacketBuffer, RxReceivedByteCount);
  }

  /* Handle TX-complete event latched by OTA_Radio_IRQHandler(). */
  if (TxDoneEventPending == TRUE)
  {
    TxDoneEventPending = FALSE;
    dbg_tx_done_count++;
    RadioTxBusy = 0U;
    RadioTxStartMs = 0U;
    BSP_LED_Toggle(LD3);
  }

  /* Recover from a missed TX_DONE IRQ that would otherwise keep TX busy forever. */
  if ((RadioTxBusy != 0U) && (RadioTxStartMs != 0U) && ((now_ms - RadioTxStartMs) >= OTA_TX_STUCK_TIMEOUT_MS))
  {
    RadioTxBusy = 0U;
    RadioTxStartMs = 0U;
    if (RadioRxArmed == 0U)
    {
      OTA_Radio_StartReception();
    }
  }

  /* Handle deferred RX error event and apply phase-specific retry policy.
     When OTA is in OTA_ONGOING, use ota_waiting_ack_state_g to know which
     handshake acknowledgement is expected from peer. */
  if (RxErrorEventPending == TRUE)
  {
    RxErrorEventPending = FALSE;
    dbg_rx_err_count++;
    RadioRxArmed = 0U;
    BSP_LED_Toggle(LD2);

    if ((ota_state_machine_g == OTA_CONNECTION) ||
        ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_CONNECTION)))
    {
      retry_connection++;
      if (retry_connection > MAX_RETRY)
      {
        retry_connection = 0U;
        ota_state_machine_g = OTA_CONNECTION;
      }
    }
    else if ((ota_state_machine_g == OTA_SIZE) ||
             ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_SIZE)))
    {
      /* Keep SIZE phase and rely on timeout-based retransmission instead of resetting to CONNECTION. */
      retry_size++;
      if (retry_size > MAX_RETRY)
      {
        retry_size = 0U;
      }
    }
    else if ((ota_state_machine_g == OTA_START) ||
             ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_START)))
    {
      /* Keep START phase and rely on timeout-based retransmission instead of resetting to CONNECTION. */
      retry_start++;
      if (retry_start > MAX_RETRY)
      {
        retry_start = 0U;
      }
    }
    else if ((ota_state_machine_g == OTA_DATAREQ) ||
             ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_DATAREQ)))
    {
      retry_datareq++;
      if (retry_datareq > 250U)
      {
        retry_datareq = 0U;
      }
    }
  }

  /* Execute only states that require tick-side actions. */
  switch (ota_state_machine_g)
  {
    case OTA_CONNECTION:
      /* Start a new OTA session using the image size collected by Ymodem. */
      last_frame = 0U;
      seq_num = 0U;
      retry_connection = 0U;
      seq_num_max = (uint16_t)(image_size / BYTE_IN_FRAME);
      if ((image_size % BYTE_IN_FRAME) != 0U)
      {
        seq_num_max++;
      }

      tx_buffer[0] = HEADER_CONNECTION;
      tx_buffer[1] = 0U;
      if (dbg_first_ota_packet_queued == 0U)
      {
        dbg_first_ota_packet_queued = 1U;
      }
      OTA_QueueTx(tx_buffer, 2U);
      // dbg_conn_retry_since_ms = now_ms;
      ota_waiting_ack_state_g = OTA_CONNECTION;
      ota_state_machine_g = OTA_ONGOING;
      break;

    case OTA_SIZE:
      tx_buffer[0] = HEADER_SIZE;
      tx_buffer[1] = 4U;
      tx_buffer[2] = (uint8_t)(image_size >> 24);
      tx_buffer[3] = (uint8_t)(image_size >> 16);
      tx_buffer[4] = (uint8_t)(image_size >> 8);
      tx_buffer[5] = (uint8_t)image_size;
      OTA_QueueTx(tx_buffer, 6U);
      // dbg_conn_retry_since_ms = now_ms;
      ota_waiting_ack_state_g = OTA_SIZE;
      ota_state_machine_g = OTA_ONGOING;
      break;

    case OTA_START:
      APP_DBG_MSG("OTA_START\r\n");
      break;

    case OTA_SENDATA:
    {
      uint32_t ota_pkt_idx = ((uint32_t)seq_num * BYTE_IN_FRAME);
      uint8_t payload_length;
      uint8_t data_length;
      uint8_t i;
      
      /* Build one OTA data frame from the current Ymodem staging chunk. */
      if ((ota_pkt_idx + BYTE_IN_FRAME) <= data_len)
      {
        payload_length = BYTE_IN_FRAME + 2U;
      }
      else if ((ota_pkt_idx + 1U) > data_len)
      {
        /* Nothing to send for this sequence, request next chunk negotiation. */
        ota_state_machine_g = OTA_DATAREQ;
        break;
      }
      else
      {
        payload_length = (uint8_t)((data_len - ota_pkt_idx) + 2U);
        if (ymodem_state_machine_g == YMODEM_CLOSE_STATE)
        {
          last_frame = 1U;
        }
      }

      data_length = (uint8_t)(payload_length - 2U);
      image_tmp[0] = HEADER_SENDATA;
      image_tmp[1] = payload_length;
      image_tmp[2] = (uint8_t)(seq_num >> 8);
      image_tmp[3] = (uint8_t)seq_num;
      
      for (i = 0U; i < data_length; i++)
      {
        image_tmp[4U + i] = image[(ota_pkt_idx % PACKET_1K_SIZE) + i];
      }

      OTA_QueueTx(image_tmp, (uint8_t)(payload_length + 2U));
      ota_state_machine_g = OTA_DATAREQ;
      break;
    }

    case OTA_COMPLETE:
      /* End of current transfer session: restart handshake for the next image. */
      APP_DBG_MSG("OTA_COMPLETE\r\n");
      ota_state_machine_g = OTA_CONNECTION;
      last_frame = 0U;
      seq_num = 0U;
      break;

    case OTA_DATAREQ:
    case OTA_ONGOING:
    default:
      /* Passive states: progression occurs from deferred events and callbacks. */
      break;
  }

  /* While waiting for handshake ack, periodically re-queue the current handshake frame. */
//  if ((ota_state_machine_g == OTA_ONGOING) &&
//      (tx_pending == 0U) &&
//      (RadioTxBusy == 0U) &&
//      ((now_ms - dbg_conn_retry_since_ms) >= OTA_ACK_TIMEOUT_MS))
//  {
//    if (ota_waiting_ack_state_g == OTA_CONNECTION)
//    {
//      tx_buffer[0] = HEADER_CONNECTION;
//      tx_buffer[1] = 0U;
//      OTA_QueueTx(tx_buffer, 2U);
//      dbg_conn_retry_since_ms = now_ms;
//    }
//    else if (ota_waiting_ack_state_g == OTA_SIZE)
//    {
//      tx_buffer[0] = HEADER_SIZE;
//      tx_buffer[1] = 4U;
//      tx_buffer[2] = (uint8_t)(image_size >> 24);
//      tx_buffer[3] = (uint8_t)(image_size >> 16);
//      tx_buffer[4] = (uint8_t)(image_size >> 8);
//      tx_buffer[5] = (uint8_t)image_size;
//      OTA_QueueTx(tx_buffer, 6U);
//      dbg_conn_retry_since_ms = now_ms;
//    }
//    else if (ota_waiting_ack_state_g == OTA_START)
//    {
//      tx_buffer[0] = HEADER_START;
//      tx_buffer[1] = 0U;
//      OTA_QueueTx(tx_buffer, 2U);
//      dbg_conn_retry_since_ms = now_ms;
//    }
//  }

  /* Service radio scheduling: send pending TX first, otherwise ensure RX is armed. */
  OTA_Radio_Service();

  /* Periodic heartbeat to diagnose handshake stalls without flooding logs. */
  if ((now_ms - dbg_last_print_ms) >= OTA_DEBUG_PERIOD_MS)
  {
    dbg_last_print_ms = now_ms;
    APP_DBG_MSG("OTA_SRV hb: ota=%u wait=%u ymodem=%u img=%lu data=%lu txp=%u txb=%u rxa=%u rxok=%lu hd=0x%02X txd=%lu rxerr=%lu\r\n",
                (unsigned)ota_state_machine_g,
                (unsigned)ota_waiting_ack_state_g,
                (unsigned)ymodem_state_machine_g,
                image_size,
                data_len,
                tx_pending,
                RadioTxBusy,
                RadioRxArmed,
                dbg_rx_ok_count,
                RadioRxPacketBuffer[0],
                dbg_tx_done_count,
                dbg_rx_err_count);
  }

  /* Release shared OTA/Ymodem ownership token. */
  if (app_token == TOKEN_TAKEN_OTA)
  {
    app_token = TOKEN_RELEASE;
  }

  return 0U;
}

/**
  * @brief  Runs one Ymodem state-machine step and updates OTA payload staging.
  *
  * Executes exactly one Ymodem sub-state step while holding the shared
  * OTA/Ymodem token, updates staged payload metadata, and releases the token
  * before returning.
  * @retval 0 when no terminal/error receive condition is reported.
  * @retval Non-zero Ymodem return code on terminal/error receive condition.
  */
int32_t OTA_ymodem_tick(void)
{
  int32_t ret = 0;
  int32_t ret_code = 0;
  uint32_t ymodem_data_len = 0U;
  uint32_t tmp_size = 0U;
  uint8_t tmp_buff[PACKET_1K_SIZE + 8U];
  uint32_t OTA_bytes_sent = 0U;
  uint32_t remaining = 0U;
  

  if (app_token == TOKEN_TAKEN_OTA)
  {
    return 0;
  }

  app_token = TOKEN_TAKEN_YMODEM;

  switch (ymodem_state_machine_g)
  {
    case YMODEM_SIZE_STATE:
      ret = Ymodem_Receive(tmp_buff, 0U, &image_size, ymodem_packet_index++);
      if (ret != YMODEM_CONTINUE)
      {
        ymodem_state_machine_g = YMODEM_ABORT_STATE;
        ret_code = ret;
      }
      else
      {
        APP_DBG_MSG("YMODEM header received, image_size=%lu bytes\r\n", image_size);
        ymodem_dump_offset = 0U;
        ymodem_capture_size = 0U;
        ymodem_capture_overflow = 0U;
        ymodem_capture_printed = 0U;
        ymodem_state_machine_g = YMODEM_LOAD_STATE;
      }
      break;

    case YMODEM_LOAD_STATE:
      Ymodem_SendAck();

      ret = Ymodem_Receive(image, 0U, &ymodem_data_len, ymodem_packet_index++);
      if (ret != YMODEM_CONTINUE)
      {
        ymodem_state_machine_g = YMODEM_ABORT_STATE;
        ret_code = ret;
        break;
      }

      OTA_CaptureYmodemPayloadChunk(image, ymodem_data_len);
      ymodem_dump_offset += ymodem_data_len;

      if ((ymodem_data_len + data_len) > image_size)
      {
        ymodem_state_machine_g = YMODEM_CLOSE_STATE;
        Ymodem_SendAck();
        data_len = image_size;
      }
      else
      {
        data_len += ymodem_data_len;
        ymodem_state_machine_g = YMODEM_WAIT_STATE;
      }
      break;

    /* Pauses YMODEM transfer until almost all data has been sent OTA */
    case YMODEM_WAIT_STATE:
      OTA_bytes_sent = (uint32_t)seq_num * BYTE_IN_FRAME;
      remaining  = (OTA_bytes_sent < data_len) ? (data_len - OTA_bytes_sent) : 0U;
      if (remaining <= BYTE_IN_FRAME)
      {
        ymodem_state_machine_g = YMODEM_LOAD_STATE;
      }
      break;

    case YMODEM_CLOSE_STATE:
      ymodem_state_machine_g = YMODEM_COMPLETE_STATE;

      ret = Ymodem_Receive(tmp_buff, 0U, &tmp_size, ymodem_packet_index++);
      if (ret != YMODEM_DONE)
      {
        Ymodem_Abort();
        ret_code = ret;
      }
      Ymodem_SendAck();
      break;

    case YMODEM_COMPLETE_STATE:
      if (ymodem_capture_printed == 0U)
      {
        uint32_t preview_len = ymodem_capture_size;

        APP_DBG_MSG("YMODEM capture complete: size=%lu, dropped=%lu, total=%lu\r\n",
                    ymodem_capture_size,
                    ymodem_capture_overflow,
                    ymodem_capture_size + ymodem_capture_overflow);

        if (preview_len > 256U)
        {
          preview_len = 256U;
          APP_DBG_MSG("YMODEM dump preview: first %lu bytes\r\n", preview_len);
        }
        else
        {
          APP_DBG_MSG("YMODEM dump preview: full capture (%lu bytes)\r\n", preview_len);
        }

        ymodem_dump_offset = 0U;
        OTA_PrintYmodemPayloadChunk(ymodem_capture_buffer, preview_len);
        ymodem_capture_printed = 1U;
      }
      break;

    case YMODEM_ABORT_STATE:
      Ymodem_Abort();
      ret_code = -1;
      break;

    default:
      break;
  }

  app_token = TOKEN_RELEASE;
  return ret_code;
}

/**
  * @brief  Latches OTA-related MRSubG IRQ events for deferred processing in OTA_Tick().
  *
  * Keeps ISR work minimal by clearing hardware IRQ flags and storing software
  * event indicators consumed later in task context.
  * @param  None.
  * @retval None.
  */
void OTA_Radio_IRQHandler(void)
{
  /* Snapshot RFSEQ IRQ status once for coherent per-interrupt handling. */
  uint32_t irq_status = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();

  /* Latch TX completion and clear the corresponding IRQ flag. */
  if ((irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F) != 0U)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
    TxDoneEventPending = TRUE;

    /* Re-arm RX immediately after TX complete to catch peer responses with very short turnaround. */
    if (RadioRxArmed == 0U)
    {
      OTA_Radio_StartReception();
    }
  }

  /* Latch RX completion and clear the corresponding IRQ flag. */
  if ((irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F) != 0U)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F);
    RxOkEventPending = TRUE;
  }

  /* Latch RX/error source flags for deferred processing in OTA_Tick(). */
  if ((irq_status & RFSEQ_IRQ_ERROR_MASK) != 0U)
  {
    RxErrorIrqStatus = (irq_status & RFSEQ_IRQ_ERROR_MASK);
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(RxErrorIrqStatus);
    RxErrorEventPending = TRUE;
  }
}


/**
  * @brief  Returns current OTA protocol state-machine status.
  * @retval Current value from @ref ota_state_machine_t.
  */
ota_state_machine_t OTA_GetStatus(void)
{
  return ota_state_machine_g;
}

/**
  * @brief  Returns current ownership token between OTA and Ymodem state machines.
  * @retval Current token value from @ref ota_app_token_t.
  */
uint8_t OTA_Get_App_Token_Status(void)
{
  return app_token;
}

/**
  * @brief  Pushes received UART bytes into Ymodem RX buffer.
  * @param  data_buffer Pointer to UART payload bytes.
  * @param  nb_bytes Number of bytes in @p data_buffer.
  * @note   If the RX buffer becomes full, remaining bytes are dropped.
  */
void processInputData(uint8_t *data_buffer, uint16_t nb_bytes)
{
  uint16_t i;

  /* Ignore invalid UART input buffers. */
  if ((data_buffer == NULL) || (nb_bytes == 0U))
  {
    return;
  }

  /* Initialize Ymodem RX pointers when starting to fill an empty buffer. */
  if (ymodem_rx_buffer_size == 0U)
  {
    ymodem_rx_buffer_ptr = ymodem_rx_buffer;
    ymodem_rx_buffer_tail_ptr = ymodem_rx_buffer_ptr;
  }

  for (i = 0U; i < nb_bytes; i++)
  {
    /* Stop accepting bytes if the linear Ymodem RX buffer is full. */
    if ((uint32_t)(ymodem_rx_buffer_tail_ptr - ymodem_rx_buffer) >= RX_BUFFER_SIZE)
    {
      break;
    }

    *ymodem_rx_buffer_tail_ptr++ = data_buffer[i];
    ymodem_rx_buffer_size++;
  }
}

/* Private functions --------------------------------------------------------*/

/**
  * @brief  Prints one received YMODEM payload chunk as hexadecimal bytes.
  * @param  buffer Pointer to payload bytes.
  * @param  length Number of valid bytes in @p buffer.
  * @retval None.
  */
static void OTA_PrintYmodemPayloadChunk(const uint8_t *buffer, uint32_t length)
{
  uint32_t index;

  if ((buffer == NULL) || (length == 0U))
  {
    return;
  }

  APP_DBG_MSG("YMODEM data chunk: %lu bytes\r\n", length);

  for (index = 0U; index < length; index++)
  {
    if ((index % YMODEM_DUMP_LINE_BYTES) == 0U)
    {
      APP_DBG_MSG("%08lX: ", (unsigned long)(ymodem_dump_offset + index));
    }

    APP_DBG_MSG("%02X ", buffer[index]);

    if (((index % YMODEM_DUMP_LINE_BYTES) == (YMODEM_DUMP_LINE_BYTES - 1U)) ||
        (index == (length - 1U)))
    {
      APP_DBG_MSG("\r\n");
    }
  }
}

/**
  * @brief  Captures YMODEM payload bytes in a persistent RAM buffer for debugger inspection.
  * @param  buffer Pointer to payload bytes.
  * @param  length Number of valid bytes in @p buffer.
  * @retval None.
  */
static void OTA_CaptureYmodemPayloadChunk(const uint8_t *buffer, uint32_t length)
{
  uint32_t index;

  if ((buffer == NULL) || (length == 0U))
  {
    return;
  }

  for (index = 0U; index < length; index++)
  {
    if (ymodem_capture_size < YMODEM_CAPTURE_MAX_BYTES)
    {
      ymodem_capture_buffer[ymodem_capture_size++] = buffer[index];
    }
    else
    {
      ymodem_capture_overflow++;
    }
  }
}

/**
  * @brief  Handles callback logic for the CONNECTION state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the connection frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t OTA_ConnectionCallback(const uint8_t *packet, uint16_t length)
{
  if ((packet == NULL) || (length == 0U) || (packet[0] != HEADER_CONNECTION))
  {
    return 0U;
  }

  if ((ota_state_machine_g == OTA_CONNECTION) ||
      ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_CONNECTION)))
  {
    BSP_LED_Toggle(LD1);
    retry_connection = 0U;
    retry_size = 0U;
    ota_waiting_ack_state_g = OTA_SIZE;
    ota_state_machine_g = OTA_SIZE;
    return 1U;
  }

  return 0U;
}

/**
  * @brief  Handles callback logic for the SIZE state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the size frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t OTA_SizeCallback(const uint8_t *packet, uint16_t length)
{
  if ((packet == NULL) || (length == 0U) || (packet[0] != HEADER_SIZE))
  {
    return 0U;
  }

  if ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_SIZE))
  {
    retry_size = 0U;
    retry_start = 0U;
    ota_waiting_ack_state_g = OTA_START;
    ota_state_machine_g = OTA_START;
    return 1U;
  }

  return 0U;
}

/**
  * @brief  Handles callback logic for the START state. ACKs the client frame
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the start frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t OTA_StartCallback(const uint8_t *packet, uint16_t length)
{
  if ((packet == NULL) || (length == 0U) || (packet[0] != HEADER_START))
  {
    return 0U;
  }

  if (ota_state_machine_g == OTA_START)
  {
    //retry_start = 0U;
    //retry_datareq = 0U;
    tx_buffer[0] = HEADER_START;
    tx_buffer[1] = 0U;
    OTA_QueueTx(tx_buffer, 2U);
    ota_state_machine_g = OTA_DATAREQ;
    return 1U;
  }

 /*  if ((ota_state_machine_g == OTA_ONGOING) && (ota_waiting_ack_state_g == OTA_START))
  {
    retry_start = 0U;
    retry_datareq = 0U;
    ota_waiting_ack_state_g = OTA_DATAREQ;
    ota_state_machine_g = OTA_DATAREQ;
    return 1U;
  } */

  return 0U;
}

/**
  * @brief  Handles callback logic for the DATA REQUEST state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the data request frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t OTA_DataRequestCallback(const uint8_t *packet, uint16_t length)
{
  if ((packet == NULL) || (length < 4U) || (packet[0] != HEADER_DATAREQ) || (packet[1] != 2U))
  {
    return 0U;
  }

  if (ota_state_machine_g != OTA_DATAREQ)
  {
    return 0U;
  }

  seq_num = (((uint16_t)packet[2]) << 8) | ((uint16_t)packet[3]);
  retry_datareq = 0U;
  //HAL_Delay(100);
  ota_state_machine_g = OTA_SENDATA;
  return 1U;
}

/**
  * @brief  Handles callback logic for the SEND DATA acknowledgement state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the send-data acknowledgement frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t OTA_SendDataCallback(const uint8_t *packet, uint16_t length)
{
  (void)length;

  if ((packet == NULL) || (packet[0] != HEADER_SENDATA) || (ota_state_machine_g != OTA_DATAREQ))
  {
    return 0U;
  }

  if (last_frame != 0U)
  {
    ota_state_machine_g = OTA_COMPLETE;
  }
  else
  {
    ota_state_machine_g = OTA_DATAREQ;
  }

  return 1U;
}

/**
  * @brief  Resets OTA server runtime context to initial state.
  * @param  full_reset If non-zero, also reset Ymodem-side staging context.
  * @retval None.
  */
static void OTA_ResetServerContext(uint8_t full_reset)
{
  /* Reset OTA-side transfer/session state. */
  ota_state_machine_g = OTA_CONNECTION;
  ota_waiting_ack_state_g = OTA_CONNECTION;
  seq_num = 0U;
  seq_num_max = 0U;
  retry_connection = 0U;
  retry_size = 0U;
  retry_start = 0U;
  retry_datareq = 0U;
  last_frame = 0U;
  tx_pending = 0U;
  tx_length = 0U;
  app_token = TOKEN_RELEASE;

  /* Reset deferred radio events and local radio scheduler indicators. */
  RxReceivedByteCount = 0U;
  RxOkEventPending = FALSE;
  TxDoneEventPending = FALSE;
  RxErrorEventPending = FALSE;
  RxErrorIrqStatus = 0U;
  RadioTxBusy = 0U;
  RadioTxStartMs = 0U;
  RadioRxArmed = 0U;
  RadioAbortInProgress = 0U;
  dbg_rx_ok_count = 0U;
  dbg_tx_done_count = 0U;
  dbg_rx_err_count = 0U;
  dbg_last_print_ms = HAL_GetTick();
  dbg_first_ota_packet_queued = 0U;
  // dbg_conn_retry_since_ms = dbg_last_print_ms;

  /* Optionally reset Ymodem-side staging context (used at OTA_Init). */
  if (full_reset != 0U)
  {
    ymodem_state_machine_g = YMODEM_SIZE_STATE;
    ymodem_packet_index = 0U;
    data_len = 0U;
    image_size = 0U;
  }
}

/**
  * @brief  Queues an OTA packet for deferred radio transmission.
  * @param  packet Pointer to the packet payload to enqueue.
  * @param  length Packet length in bytes.
  * @retval None.
  */
static void OTA_QueueTx(const uint8_t *packet, uint8_t length)
{
  /* Ignore invalid pointers or empty packets. */
  if ((packet == NULL) || (length == 0U))
  {
    return;
  }

  /* Copy payload into the internal TX buffer consumed by OTA_Radio_Service. */
  for (uint8_t index = 0U; index < length; index++)
  {
    tx_buffer[index] = packet[index];
  }

  /* Mark one packet as pending for transmission. */
  tx_length = length;
  tx_pending = 1U;
}

/**
  * @brief  Validates and dispatches a received OTA packet to the active-state handler.
  * @param  packet Pointer to received packet data.
  * @param  length Received packet length in bytes.
  * @retval 1U if the packet is accepted/processed by the current state callback.
  * @retval 0U if the packet is invalid or not handled in the current state.
  */
static uint8_t OTA_ProcessReceivedPacket(const uint8_t *packet, uint16_t length)
{
  /* Reject invalid packet pointers and out-of-range lengths. */
  if ((packet == NULL) || (length == 0U) || (length > MAX_PACKET_LENGTH))
  {
    return 0U;
  }

  /* Route packet processing according to current OTA state. */
  switch (ota_state_machine_g)
  {
    case OTA_CONNECTION:
      return OTA_ConnectionCallback(packet, length);

    case OTA_START:
      if (packet[0] == HEADER_CONNECTION)
      {
        return OTA_ConnectionCallback(packet, length);
      }
      return OTA_StartCallback(packet, length);

    case OTA_ONGOING:
      if (ota_waiting_ack_state_g == OTA_CONNECTION)
      {
        return OTA_ConnectionCallback(packet, length);
      }
      if (ota_waiting_ack_state_g == OTA_SIZE)
      {
        if (packet[0] == HEADER_CONNECTION)
        {
          return OTA_ConnectionCallback(packet, length);
        }
        return OTA_SizeCallback(packet, length);
      }
      if (ota_waiting_ack_state_g == OTA_START)
      {
        if (packet[0] == HEADER_SIZE)
        {
          return OTA_SizeCallback(packet, length);
        }
        return OTA_StartCallback(packet, length);
      }
      return 0U;

    case OTA_DATAREQ:
      if (packet[0] == HEADER_DATAREQ)
      {
        return OTA_DataRequestCallback(packet, length);
      }
      if (packet[0] == HEADER_SENDATA)
      {
        return OTA_SendDataCallback(packet, length);
      }
      return 0U;

    default:
      return 0U;
  }
}

/**
  * @brief  Returns the currently queued OTA transmit packet.
  * @param  packet Output pointer receiving the internal TX buffer address.
  * @param  length Output pointer receiving the TX packet length.
  * @retval 1U if a packet was pending and returned.
  * @retval 0U if no packet is pending or output pointers are invalid.
  */
static uint8_t OTA_GetTxPacket(const uint8_t **packet, uint8_t *length)
{
  /* Reject invalid output pointers and empty TX queue state. */
  if ((packet == NULL) || (length == NULL) || (tx_pending == 0U))
  {
    return 0U;
  }

  /* Expose queued packet metadata and mark it as consumed. */
  *packet = tx_buffer;
  *length = tx_length;
  tx_pending = 0U;
  return 1U;
}

/**
  * @brief  Configures and starts radio reception for OTA packets.
  * @param  None.
  * @retval None.
  */
static void OTA_Radio_StartReception(void)
{
  /* Configure RX mode and point radio data buffer to the OTA RX packet buffer. */
  __HAL_MRSUBG_SET_RX_MODE(RX_NORMAL);
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)RadioRxPacketBuffer);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(MAX_PACKET_LENGTH);
  HAL_MRSubG_PktBasicSetPayloadLength(MAX_PACKET_LENGTH-1);

  /* Clear pending RX/error IRQ flags and start reception. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F | RFSEQ_IRQ_ERROR_MASK);
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  /* Track local RX armed state for scheduler decisions. */
  RadioRxArmed = 1U;
}

/**
  * @brief  Configures and starts radio transmission of one OTA packet.
  * @param  packet Pointer to packet payload to transmit.
  * @param  length Packet length in bytes.
  * @retval None.
  */
static void OTA_Radio_StartTransmission(const uint8_t *packet, uint8_t length)
{
  /* Ignore invalid packet input. */
  if ((packet == NULL) || (length == 0U))
  {
    return;
  }

  /* Copy payload to radio TX buffer. */
  for (uint8_t index = 0U; index < length; index++)
  {
    RadioTxPacketBuffer[index] = packet[index];
    if (OTA_DEBUG_ONLY == 1U) {
      APP_DBG_MSG("%02X ", packet[index]);
    }
  }
  APP_DBG_MSG("\r\n");

  /* Configure TX mode, payload length, and data buffer settings. */
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  HAL_MRSubG_PktBasicSetPayloadLength(length);
  //APP_DBG_MSG("len1: 0x%08X \r\n", READ_REG(MR_SUBG_GLOB_DYNAMIC->PCKTLEN_CONFIG));
  //APP_DBG_MSG("len2: 0x%08X \r\n", READ_REG(MR_SUBG_GLOB_STATIC->DATABUFFER_SIZE));
  
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)RadioTxPacketBuffer);
//  __HAL_MRSUBG_SET_DATABUFFER_SIZE(MAX_PACKET_LENGTH);
  /* Clear TX-done IRQ flag and start TX command. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
  __HAL_MRSUBG_STROBE_CMD(CMD_TX);
  BSP_LED_Toggle(LD2);
  /* Update scheduler flags: TX is busy and RX is no longer armed. */
  RadioTxBusy = 1U;
  RadioTxStartMs = HAL_GetTick();
  RadioRxArmed = 0U;
}

/**
  * @brief  Services OTA radio scheduling between TX and RX operations.
  * @param  None.
  * @retval None.
  */
static void OTA_Radio_Service(void)
{
  const uint8_t *packet;
  uint8_t length;
  uint32_t irq_status;

  /* If TX is in progress, defer any new operation. */
  if (RadioTxBusy != 0U)
  {
    return;
  }

  /* Wait for RX abort completion before issuing TX command. */
  if (RadioAbortInProgress != 0U)
  {
    irq_status = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();
    if ((irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F) == 0U)
    {
      return;
    }

    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
    RadioAbortInProgress = 0U;
  }

  /* Switch from RX to TX in two steps to avoid command rejection races. */
  if ((tx_pending != 0U) && (RadioRxArmed != 0U))
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
    __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
    RadioRxArmed = 0U;
    RadioAbortInProgress = 1U;
    return;
  }

  /* Prioritize pending TX packet transmission. */
  if (OTA_GetTxPacket(&packet, &length) != 0U)
  {
    OTA_Radio_StartTransmission(packet, length);
    return;
  }

  /* If nothing to send, keep receiver armed. */
  if (RadioRxArmed == 0U)
  {
    OTA_Radio_StartReception();
  }
}
