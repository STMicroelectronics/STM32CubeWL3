/**
  ******************************************************************************
  * @file    sfuota_server.c
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA server protocol state machine.
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
#include "sfuota_server.h"
#include "main.h"

/* Private define -----------------------------------------------------------*/
#define SFUOTA_VERSION            "1.1.0"               /*!< SFUOTA version */
#define SFUOTA_MAX_RETRY          (10U)                 /*!< Retries before falling back to the connection phase. */
#define SFUOTA_DEBUG_PERIOD_MS    (1000U)               /*!< Period for SFUOTA debug heartbeat print. */
#define SFUOTA_COMPLETE_TIMEOUT_MS (2000U)              /*!< Grace period for the peer notification after the last frame is served. */
#define SFUOTA_PEER_TIMEOUT_MS    (5000U)               /*!< Silence tolerated mid-session before the handshake is restarted. */
#define SFUOTA_PRESET_IMAGE_SIZE  (10240U)              /*!< Simulated image size used by SFUOTA_Ymodem_Preset(). */

/* Diagnostics, independent from SFUOTA_SERVER_MODE. */
#define SFUOTA_TRACE_TX_FRAMES    (0U)                  /*!< Dumps every transmitted frame as hex. Adds about 5 ms per frame. */
#define SFUOTA_CAPTURE_PAYLOAD    (1U)                  /*!< Mirrors the first bytes of the image in RAM for Live Watch. Costs YMODEM_CAPTURE_MAX_BYTES. */
#define YMODEM_DUMP_LINE_BYTES    (16U)                 /*!< Number of bytes per output line in YMODEM payload dump. */
#define YMODEM_CAPTURE_MAX_BYTES  (4096U)               /*!< Max number of payload bytes kept in RAM for Live Watch inspection. */

/* Private variables --------------------------------------------------------*/
/* SFUOTA transport TX staging buffer and queue state. */
static uint8_t tx_buffer[SFUOTA_RADIO_MAX_PACKET];  /*!< SFUOTA transport TX buffer. */
static uint8_t tx_length = 0U;                      /*!< Length of pending TX packet. */
static uint8_t tx_pending = 0U;                     /*!< TX pending flag. */

/* SFUOTA protocol/session state and retry counters. */
static sfuota_state_t sfuota_state = SFUOTA_ST_CONNECTION;        /*!< SFUOTA protocol/session state. */
static sfuota_state_t sfuota_waiting_ack = SFUOTA_ST_CONNECTION;  /*!< Handshake phase currently waiting for peer acknowledgement while in SFUOTA_ST_ONGOING. */
static uint16_t seq_num = 0U;                                     /*!< Current SFUOTA sequence number. */
static uint16_t seq_num_max = 0U;                                 /*!< Total frames expected for the announced image size. */
static uint16_t retry_connection = 0U;                            /*!< Retry counter for CONNECTION handshake phase. */

/* Server transfer/Ymodem context. */
static sfuota_token_t app_token = SFUOTA_TOKEN_FREE;                    /*!< Ownership token shared with Ymodem side. */
static sfuota_ymodem_state_t sfuota_ymodem_state = SFUOTA_YMODEM_SIZE;  /*!< Current Ymodem sub-state. */
static uint8_t last_frame = 0U;                                         /*!< Last-frame indicator for SFUOTA SENDATA completion. */
static uint32_t last_frame_ms = 0U;                                     /*!< Timestamp of the last time the final frame was served. */
static uint32_t last_rx_ms = 0U;                                        /*!< Timestamp of the last frame received from the peer. */
static uint32_t ymodem_packet_index = 0U;                               /*!< Current Ymodem packet index used by Ymodem_Receive sequencing. */
static uint8_t image_tmp[SFUOTA_FRAME_PAYLOAD + 8U];                    /*!< Temporary SFUOTA SENDATA frame payload buffer. */
#if (SFUOTA_CAPTURE_PAYLOAD == 1U)
static uint32_t ymodem_dump_offset = 0U;                                /*!< Cumulative byte offset used while printing received YMODEM payload. */
uint8_t ymodem_capture_buffer[YMODEM_CAPTURE_MAX_BYTES];                /*!< Persistent payload capture buffer for debugger inspection. */
uint32_t ymodem_capture_size = 0U;                                      /*!< Number of valid bytes currently stored in ymodem_capture_buffer. */
uint32_t ymodem_capture_overflow = 0U;                                  /*!< Number of payload bytes dropped once capture buffer is full. */
static uint8_t ymodem_capture_printed = 0U;                             /*!< Ensures capture dump is printed once per transfer. */
#endif

/* Debug telemetry counters. */
static uint32_t dbg_rx_ok_count = 0U;
static uint32_t dbg_tx_done_count = 0U;
static uint32_t dbg_rx_err_count = 0U;
static uint32_t dbg_last_print_ms = 0U;
static uint8_t dbg_first_packet_queued = 0U;
static uint8_t dbg_last_rx_header = 0U;

/* Private functions prototypes ----------------------------------------------*/

/* Debug output helpers. */
static void SFUOTA_PrintHeartbeat(uint32_t now_ms);
#if (SFUOTA_CAPTURE_PAYLOAD == 1U)
static void SFUOTA_Ymodem_PrintChunk(const uint8_t *buffer, uint32_t length);
static void SFUOTA_Ymodem_CaptureChunk(const uint8_t *buffer, uint32_t length);
#endif

/* SFUOTA protocol state callbacks. */
static uint8_t SFUOTA_ConnectionCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_SizeCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_StartCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_DataRequestCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_SendDataCallback(const uint8_t *packet, uint16_t length);

/* SFUOTA context and packet helpers. */
static void SFUOTA_ResetContext(uint8_t full_reset);
static void SFUOTA_QueueTx(const uint8_t *packet, uint8_t length);
static uint8_t SFUOTA_ProcessReceivedPacket(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_GetTxPacket(const uint8_t **packet, uint8_t *length);

/* Radio scheduling helper. */
static void SFUOTA_RadioService(void);

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Fills the staging window with a known pattern, bypassing Ymodem.
  * @note   Used only in SFUOTA_MODE_RADIO, to exercise the radio path alone.
  * @retval None.
  */
void SFUOTA_Ymodem_Preset(void)
{
  SFUOTA_Image_Preset(SFUOTA_PRESET_IMAGE_SIZE);
}

/**
  * @brief  Initializes SFUOTA context and radio event handling.
  *
  * Resets SFUOTA server runtime variables (including Ymodem sub-state and shared
  * ownership token), initializes the radio adaptation layer and prints startup
  * debug information.
  * @param  None.
  * @retval 0U always.
  */
uint8_t SFUOTA_Init(void)
{
  /* Reset full server context (SFUOTA transfer + Ymodem staging ownership). */
  SFUOTA_ResetContext(1U);

  /* Reception is armed later by SFUOTA_Tick(), together with the first connection frame. */
  SFUOTA_Radio_Init();

  APP_DBG_MSG("SFUOTA server (version: %s)\r\n", SFUOTA_VERSION);
  return 0U;
}

/**
  * @brief  Runs one SFUOTA state machine iteration and services radio events.
  *
  * Consumes pending RX/TX/error events latched by SFUOTA_IRQHandler(),
  * executes tick-driven server-side SFUOTA states, arbitrates access with the
  * Ymodem side, and services radio TX/RX scheduling.
  *
  * @retval 0U always for this server transfer flow.
  */
uint8_t SFUOTA_Tick(void)
{
  uint32_t now_ms = HAL_GetTick();
  sfuota_radio_event_t phy_events;

  /* Defer SFUOTA activity while Ymodem owns the shared staging context. */
  if (app_token == SFUOTA_TOKEN_YMODEM)
  {
    /* Still service radio so queued retransmissions can be transmitted. */
    SFUOTA_RadioService();

    /* Keep heartbeat visible even while SFUOTA work is deferred by token ownership. */
    SFUOTA_PrintHeartbeat(now_ms);
    return 0U;
  }
  app_token = SFUOTA_TOKEN_TRANSFER;

  /* Collect the radio events latched by SFUOTA_Radio_IRQHandler(). */
  SFUOTA_Radio_GetEvents(&phy_events);

  /* Handle RX-complete event. */
  if (phy_events.rx_ok == TRUE)
  {
    dbg_rx_ok_count++;
    last_rx_ms = now_ms;
    if (phy_events.rx_length != 0U)
    {
      dbg_last_rx_header = phy_events.rx_data[0];
    }
    (void)SFUOTA_ProcessReceivedPacket(phy_events.rx_data, phy_events.rx_length);
  }

  /* Handle TX-complete event. */
  if (phy_events.tx_done == TRUE)
  {
    dbg_tx_done_count++;
    BSP_LED_Toggle(LD2);
  }

  /* Handle deferred RX error event. Only the connection phase escalates: in the other
     phases the session restarts when the client re-sends CONNECTION. */
  if (phy_events.rx_error == TRUE)
  {
    dbg_rx_err_count++;
    BSP_LED_Toggle(LD3);
    APP_DBG_MSG("SFUOTA_RXERR st=%u wait=%u seq=%u err=0x%02X tot=%lu\r\n",
                (unsigned)sfuota_state,
                (unsigned)sfuota_waiting_ack,
                (unsigned)seq_num,
                phy_events.rx_error_flags,
                dbg_rx_err_count);

    if ((sfuota_state == SFUOTA_ST_CONNECTION) ||
        ((sfuota_state == SFUOTA_ST_ONGOING) && (sfuota_waiting_ack == SFUOTA_ST_CONNECTION)))
    {
      retry_connection++;
      if (retry_connection > SFUOTA_MAX_RETRY)
      {
        retry_connection = 0U;
        sfuota_state = SFUOTA_ST_CONNECTION;
      }
    }
  }

  /* Prolonged silence means the peer reset or drifted out of sync. The staging window
     holds a single chunk, so the transfer cannot resume: ask for the image again. */
  if ((sfuota_state != SFUOTA_ST_CONNECTION) &&
      ((now_ms - last_rx_ms) >= SFUOTA_PEER_TIMEOUT_MS))
  {
    APP_DBG_MSG("SFUOTA_PEER_TIMEOUT st=%u seq=%u/%u, session dropped\r\n",
                (unsigned)sfuota_state,
                (unsigned)seq_num,
                (unsigned)seq_num_max);
    SFUOTA_ResetContext(1U);
    Ymodem_Init();
    APP_DBG_MSG("Ready for a new image over YMODEM\r\n");
    last_rx_ms = now_ms;
  }

  /* Execute only states that require tick-side actions. */
  switch (sfuota_state)
  {
    case SFUOTA_ST_CONNECTION:
      /* Nothing to serve until Ymodem has announced a new image. */
      if (SFUOTA_Image_GetSize() == 0U)
      {
        break;
      }

      /* Start a new SFUOTA session using the image size collected by Ymodem. */
      last_frame = 0U;
      seq_num = 0U;
      retry_connection = 0U;
      last_rx_ms = now_ms;
      seq_num_max = (uint16_t)(SFUOTA_Image_GetSize() / SFUOTA_FRAME_PAYLOAD);
      if ((SFUOTA_Image_GetSize() % SFUOTA_FRAME_PAYLOAD) != 0U)
      {
        seq_num_max++;
      }

      tx_buffer[0] = SFUOTA_HDR_CONNECTION;
      dbg_first_packet_queued = 1U;
      SFUOTA_QueueTx(tx_buffer, 1U);
      sfuota_waiting_ack = SFUOTA_ST_CONNECTION;
      sfuota_state = SFUOTA_ST_ONGOING;
      break;

    case SFUOTA_ST_SIZE:
      tx_buffer[0] = SFUOTA_HDR_SIZE;
      tx_buffer[1] = (uint8_t)(SFUOTA_Image_GetSize() >> 24);
      tx_buffer[2] = (uint8_t)(SFUOTA_Image_GetSize() >> 16);
      tx_buffer[3] = (uint8_t)(SFUOTA_Image_GetSize() >> 8);
      tx_buffer[4] = (uint8_t)SFUOTA_Image_GetSize();
      SFUOTA_QueueTx(tx_buffer, 5U);
      sfuota_waiting_ack = SFUOTA_ST_SIZE;
      sfuota_state = SFUOTA_ST_ONGOING;
      break;

    case SFUOTA_ST_SENDATA:
    {
      uint32_t byte_offset = ((uint32_t)seq_num * SFUOTA_FRAME_PAYLOAD);
      uint32_t available = SFUOTA_Image_GetAvailable();
      uint8_t data_length;

      /* Build one SFUOTA data frame from the current image staging window. */
      if ((byte_offset + SFUOTA_FRAME_PAYLOAD) <= available)
      {
        data_length = SFUOTA_FRAME_PAYLOAD;
      }
      else if ((byte_offset + 1U) > available)
      {
        /* Chunk not staged yet: hold the state so the frame leaves as soon as the loader
           refills, rather than costing the peer a full request timeout. */
        if (available >= SFUOTA_Image_GetSize())
        {
          sfuota_state = SFUOTA_ST_DATAREQ;
        }
        break;
      }
      else
      {
        data_length = (uint8_t)(available - byte_offset);
      }

      /* Last frame of the session: this chunk reaches the size announced by the loader. */
      if ((byte_offset + (uint32_t)data_length) >= SFUOTA_Image_GetSize())
      {
        last_frame = 1U;
        last_frame_ms = now_ms;
      }

      image_tmp[0] = SFUOTA_HDR_DATA;
      image_tmp[1] = (uint8_t)(seq_num >> 8);
      image_tmp[2] = (uint8_t)seq_num;

      if (SFUOTA_Image_Read(byte_offset, &image_tmp[3], data_length) == 0U)
      {
        sfuota_state = SFUOTA_ST_DATAREQ;
        break;
      }

      SFUOTA_QueueTx(image_tmp, (uint8_t)(data_length + 3U));
      sfuota_state = SFUOTA_ST_DATAREQ;
      break;
    }

    case SFUOTA_ST_COMPLETE:
      /* End of session: drop the served image and wait for the next Ymodem transfer. */
      APP_DBG_MSG("SFUOTA_COMPLETE\r\n");
      SFUOTA_ResetContext(1U);
      Ymodem_Init();
      APP_DBG_MSG("Ready for a new image over YMODEM\r\n");
      break;

    case SFUOTA_ST_DATAREQ:
      /* The peer notification can be lost: close the session anyway once the image is fully served. */
      if ((last_frame != 0U) && ((now_ms - last_frame_ms) >= SFUOTA_COMPLETE_TIMEOUT_MS))
      {
        APP_DBG_MSG("SFUOTA_NO_PEER_ACK, closing session\r\n");
        sfuota_state = SFUOTA_ST_COMPLETE;
      }
      break;

    case SFUOTA_ST_START:
    case SFUOTA_ST_ONGOING:
    default:
      /* Passive states: progression occurs from deferred events and callbacks. */
      break;
  }

  /* Service radio scheduling: send pending TX first, otherwise ensure RX is armed. */
  SFUOTA_RadioService();

  /* Periodic heartbeat to diagnose handshake stalls without flooding logs. */
  SFUOTA_PrintHeartbeat(now_ms);

  /* Release shared SFUOTA/Ymodem ownership token. */
  if (app_token == SFUOTA_TOKEN_TRANSFER)
  {
    app_token = SFUOTA_TOKEN_FREE;
  }

  return 0U;
}

/**
  * @brief  Runs one Ymodem state-machine step and updates SFUOTA payload staging.
  *
  * Executes exactly one Ymodem sub-state step while holding the shared
  * SFUOTA/Ymodem token, updates staged payload metadata, and releases the token
  * before returning.
  * @retval 0 when no terminal/error receive condition is reported.
  * @retval Non-zero Ymodem return code on terminal/error receive condition.
  */
int32_t SFUOTA_Ymodem_Tick(void)
{
  int32_t ret = 0;
  int32_t ret_code = 0;
  uint32_t ymodem_data_len = 0U;
  uint32_t ymodem_image_size = 0U;
  uint32_t tmp_size = 0U;
  uint8_t tmp_buff[PACKET_1K_SIZE + 8U];

  if (app_token == SFUOTA_TOKEN_TRANSFER)
  {
    return 0;
  }

  app_token = SFUOTA_TOKEN_YMODEM;

  switch (sfuota_ymodem_state)
  {
    case SFUOTA_YMODEM_SIZE:
      ret = Ymodem_Receive(tmp_buff, 0U, &ymodem_image_size, ymodem_packet_index++);
      if (ret != YMODEM_CONTINUE)
      {
        sfuota_ymodem_state = SFUOTA_YMODEM_ABORT;
        ret_code = ret;
      }
      else
      {
        SFUOTA_Image_SetSize(ymodem_image_size);
        APP_DBG_MSG("YMODEM header received, image_size=%lu bytes\r\n", ymodem_image_size);
#if (SFUOTA_CAPTURE_PAYLOAD == 1U)
        ymodem_dump_offset = 0U;
        ymodem_capture_size = 0U;
        ymodem_capture_overflow = 0U;
        ymodem_capture_printed = 0U;
#endif
        sfuota_ymodem_state = SFUOTA_YMODEM_LOAD;
      }
      break;

    case SFUOTA_YMODEM_LOAD:
      Ymodem_SendAck();

      ret = Ymodem_Receive(SFUOTA_Image_GetStagingBuffer(), 0U, &ymodem_data_len, ymodem_packet_index++);
      if (ret != YMODEM_CONTINUE)
      {
        sfuota_ymodem_state = SFUOTA_YMODEM_ABORT;
        ret_code = ret;
        break;
      }

#if (SFUOTA_CAPTURE_PAYLOAD == 1U)
      SFUOTA_Ymodem_CaptureChunk(SFUOTA_Image_GetStagingBuffer(), ymodem_data_len);
      ymodem_dump_offset += ymodem_data_len;
#endif

      if (SFUOTA_Image_CommitStaging(ymodem_data_len) != 0U)
      {
        sfuota_ymodem_state = SFUOTA_YMODEM_CLOSE;
        Ymodem_SendAck();
      }
      else
      {
        sfuota_ymodem_state = SFUOTA_YMODEM_WAIT;
      }
      break;

    /* Refill is driven by the peer, not by what has been served: see YMODEM_WAIT. */
    case SFUOTA_YMODEM_WAIT:
      /* Refill only once the peer asks past the staged chunk: that request proves the
         previous one arrived, and until then it must stay available for retransmission. */
      if (((uint32_t)seq_num * SFUOTA_FRAME_PAYLOAD) >= SFUOTA_Image_GetAvailable())
      {
        sfuota_ymodem_state = SFUOTA_YMODEM_LOAD;
      }
      break;

    case SFUOTA_YMODEM_CLOSE:
      sfuota_ymodem_state = SFUOTA_YMODEM_COMPLETE;

      ret = Ymodem_Receive(tmp_buff, 0U, &tmp_size, ymodem_packet_index++);
      if (ret != YMODEM_DONE)
      {
        Ymodem_Abort();
        ret_code = ret;
      }
      Ymodem_SendAck();
      break;

    case SFUOTA_YMODEM_COMPLETE:
#if (SFUOTA_CAPTURE_PAYLOAD == 1U)
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
        SFUOTA_Ymodem_PrintChunk(ymodem_capture_buffer, preview_len);
        ymodem_capture_printed = 1U;
      }
#endif
      break;

    case SFUOTA_YMODEM_ABORT:
      /* Terminal state otherwise: rearm the loader so a new file can be sent without a reset. */
      Ymodem_Abort();
      Ymodem_Init();
      SFUOTA_Image_Reset();
      ymodem_packet_index = 0U;
      sfuota_ymodem_state = SFUOTA_YMODEM_SIZE;
      APP_DBG_MSG("YMODEM aborted, ready for a new image\r\n");
      ret_code = -1;
      break;

    default:
      break;
  }

  app_token = SFUOTA_TOKEN_FREE;
  return ret_code;
}

/**
  * @brief  Forwards the radio interrupt to the PHY adaptation layer.
  *
  * Keeps ISR work minimal by clearing hardware IRQ flags and storing software
  * event indicators consumed later in task context.
  * @param  None.
  * @retval None.
  */
void SFUOTA_IRQHandler(void)
{
  SFUOTA_Radio_IRQHandler();
}


/**
  * @brief  Returns current SFUOTA protocol state-machine status.
  * @retval Current value from @ref sfuota_state_t.
  */
sfuota_state_t SFUOTA_GetState(void)
{
  return sfuota_state;
}

/**
  * @brief  Returns current ownership token between SFUOTA and Ymodem state machines.
  * @retval Current token value from @ref sfuota_token_t.
  */
sfuota_token_t SFUOTA_GetTokenState(void)
{
  return app_token;
}

/**
  * @brief  Pushes received UART bytes into Ymodem RX buffer.
  * @param  data_buffer Pointer to UART payload bytes.
  * @param  nb_bytes Number of bytes in @p data_buffer.
  * @note   If the RX buffer becomes full, remaining bytes are dropped.
  */
void SFUOTA_Ymodem_PushBytes(uint8_t *data_buffer, uint16_t nb_bytes)
{
  uint16_t i;

  /* Ignore invalid UART input buffers. */
  if ((data_buffer == NULL) || (nb_bytes == 0U))
  {
    return;
  }

  /* Start of a new fill: reset the RX pointers. */
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
  * @brief  Prints the periodic server heartbeat when the debug period elapsed.
  * @param  now_ms Current millisecond tick.
  * @retval None.
  */
static void SFUOTA_PrintHeartbeat(uint32_t now_ms)
{
  if ((now_ms - dbg_last_print_ms) < SFUOTA_DEBUG_PERIOD_MS)
  {
    return;
  }

  dbg_last_print_ms = now_ms;
  APP_DBG_MSG("SFUOTA_SRV hb: st=%u wait=%u ymodem=%u img=%lu data=%lu seq=%u/%u 1st=%u txp=%u txb=%u rxa=%u rxok=%lu hd=0x%02X txd=%lu rxerr=%lu\r\n",
              (unsigned)sfuota_state,
              (unsigned)sfuota_waiting_ack,
              (unsigned)sfuota_ymodem_state,
              SFUOTA_Image_GetSize(),
              SFUOTA_Image_GetAvailable(),
              (unsigned)seq_num,
              (unsigned)seq_num_max,
              dbg_first_packet_queued,
              tx_pending,
              SFUOTA_Radio_IsTxBusy(),
              SFUOTA_Radio_IsRxArmed(),
              dbg_rx_ok_count,
              dbg_last_rx_header,
              dbg_tx_done_count,
              dbg_rx_err_count);
}

#if (SFUOTA_CAPTURE_PAYLOAD == 1U)
/**
  * @brief  Prints one received YMODEM payload chunk as hexadecimal bytes.
  * @param  buffer Pointer to payload bytes.
  * @param  length Number of valid bytes in @p buffer.
  * @retval None.
  */
static void SFUOTA_Ymodem_PrintChunk(const uint8_t *buffer, uint32_t length)
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
static void SFUOTA_Ymodem_CaptureChunk(const uint8_t *buffer, uint32_t length)
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
#endif /* SFUOTA_CAPTURE_PAYLOAD */

/**
  * @brief  Handles callback logic for the CONNECTION state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the connection frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_ConnectionCallback(const uint8_t *packet, uint16_t length)
{
  /* Accept only the CONNECTION acknowledgement in this phase. */
  if ((packet == NULL) || (length == 0U) || (packet[0] != SFUOTA_HDR_CONNECTION))
  {
    return 0U;
  }

  if ((sfuota_state == SFUOTA_ST_CONNECTION) ||
      ((sfuota_state == SFUOTA_ST_ONGOING) && (sfuota_waiting_ack == SFUOTA_ST_CONNECTION)))
  {
    BSP_LED_Toggle(LD1);
    APP_DBG_MSG("SFUOTA_CONNECTION\r\n");
    retry_connection = 0U;
    sfuota_waiting_ack = SFUOTA_ST_SIZE;
    sfuota_state = SFUOTA_ST_SIZE;
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
static uint8_t SFUOTA_SizeCallback(const uint8_t *packet, uint16_t length)
{
  /* Accept only the SIZE acknowledgement in this phase. */
  if ((packet == NULL) || (length == 0U) || (packet[0] != SFUOTA_HDR_SIZE))
  {
    return 0U;
  }

  if ((sfuota_state == SFUOTA_ST_ONGOING) && (sfuota_waiting_ack == SFUOTA_ST_SIZE))
  {
    APP_DBG_MSG("SFUOTA_SIZE\r\n");
    sfuota_waiting_ack = SFUOTA_ST_START;
    sfuota_state = SFUOTA_ST_START;
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
static uint8_t SFUOTA_StartCallback(const uint8_t *packet, uint16_t length)
{
  /* Accept only the START frame in this phase. */
  if ((packet == NULL) || (length == 0U) || (packet[0] != SFUOTA_HDR_START))
  {
    return 0U;
  }

  if (sfuota_state == SFUOTA_ST_START)
  {
    APP_DBG_MSG("SFUOTA_START\r\n");
    tx_buffer[0] = SFUOTA_HDR_START;
    SFUOTA_QueueTx(tx_buffer, 1U);
    sfuota_state = SFUOTA_ST_DATAREQ;
    return 1U;
  }

  return 0U;
}

/**
  * @brief  Handles callback logic for the DATA REQUEST state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the data request frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_DataRequestCallback(const uint8_t *packet, uint16_t length)
{
  /* A data request carries a 2-byte sequence number and nothing else. */
  if ((packet == NULL) || (length != 3U) || (packet[0] != SFUOTA_HDR_DATAREQ))
  {
    return 0U;
  }

  if (sfuota_state != SFUOTA_ST_DATAREQ)
  {
    return 0U;
  }

  seq_num = (((uint16_t)packet[1]) << 8) | ((uint16_t)packet[2]);
  sfuota_state = SFUOTA_ST_SENDATA;
  return 1U;
}

/**
  * @brief  Handles callback logic for the SEND DATA acknowledgement state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the send-data acknowledgement frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_SendDataCallback(const uint8_t *packet, uint16_t length)
{
  (void)length;

  /* Accept the peer notification only while a transfer is in progress. */
  if ((packet == NULL) || (packet[0] != SFUOTA_HDR_DATA) || (sfuota_state != SFUOTA_ST_DATAREQ))
  {
    return 0U;
  }

  if (last_frame != 0U)
  {
    sfuota_state = SFUOTA_ST_COMPLETE;
  }
  else
  {
    sfuota_state = SFUOTA_ST_DATAREQ;
  }

  return 1U;
}

/**
  * @brief  Resets SFUOTA server runtime context to initial state.
  * @param  full_reset If non-zero, also reset Ymodem-side staging context.
  * @retval None.
  */
static void SFUOTA_ResetContext(uint8_t full_reset)
{
  /* Reset SFUOTA-side transfer/session state. */
  sfuota_state = SFUOTA_ST_CONNECTION;
  sfuota_waiting_ack = SFUOTA_ST_CONNECTION;
  seq_num = 0U;
  seq_num_max = 0U;
  retry_connection = 0U;
  last_frame = 0U;
  tx_pending = 0U;
  tx_length = 0U;
  app_token = SFUOTA_TOKEN_FREE;

  /* Reset deferred radio events and local radio scheduler indicators. */
  SFUOTA_Radio_Reset();
  dbg_rx_ok_count = 0U;
  dbg_tx_done_count = 0U;
  dbg_rx_err_count = 0U;
  dbg_last_print_ms = HAL_GetTick();
  dbg_first_packet_queued = 0U;
  dbg_last_rx_header = 0U;

  /* Optionally reset Ymodem-side staging context (used at SFUOTA_Init). */
  if (full_reset != 0U)
  {
    sfuota_ymodem_state = SFUOTA_YMODEM_SIZE;
    ymodem_packet_index = 0U;
    SFUOTA_Image_Reset();
  }
}

/**
  * @brief  Queues an SFUOTA packet for deferred radio transmission.
  * @param  packet Pointer to the packet payload to enqueue.
  * @param  length Packet length in bytes.
  * @retval None.
  */
static void SFUOTA_QueueTx(const uint8_t *packet, uint8_t length)
{
  /* Ignore invalid pointers or empty packets. */
  if ((packet == NULL) || (length == 0U))
  {
    return;
  }

  /* Copy into the buffer consumed by SFUOTA_RadioService(). */
  for (uint8_t index = 0U; index < length; index++)
  {
    tx_buffer[index] = packet[index];
  }

  /* Mark one packet as pending for transmission. */
  tx_length = length;
  tx_pending = 1U;
}

/**
  * @brief  Validates and dispatches a received SFUOTA packet to the active-state handler.
  * @param  packet Pointer to received packet data.
  * @param  length Received packet length in bytes.
  * @retval 1U if the packet is accepted/processed by the current state callback.
  * @retval 0U if the packet is invalid or not handled in the current state.
  */
static uint8_t SFUOTA_ProcessReceivedPacket(const uint8_t *packet, uint16_t length)
{
  /* Reject invalid pointers and out-of-range lengths. */
  if ((packet == NULL) || (length == 0U) || (length > SFUOTA_RADIO_MAX_PACKET))
  {
    return 0U;
  }

  /* Routed by state: a valid frame in the wrong phase is dropped. */
  switch (sfuota_state)
  {
    case SFUOTA_ST_CONNECTION:
      return SFUOTA_ConnectionCallback(packet, length);

    case SFUOTA_ST_START:
      if (packet[0] == SFUOTA_HDR_CONNECTION)
      {
        return SFUOTA_ConnectionCallback(packet, length);
      }
      return SFUOTA_StartCallback(packet, length);

    case SFUOTA_ST_ONGOING:
      if (sfuota_waiting_ack == SFUOTA_ST_CONNECTION)
      {
        return SFUOTA_ConnectionCallback(packet, length);
      }
      if (sfuota_waiting_ack == SFUOTA_ST_SIZE)
      {
        if (packet[0] == SFUOTA_HDR_CONNECTION)
        {
          return SFUOTA_ConnectionCallback(packet, length);
        }
        return SFUOTA_SizeCallback(packet, length);
      }
      if (sfuota_waiting_ack == SFUOTA_ST_START)
      {
        if (packet[0] == SFUOTA_HDR_SIZE)
        {
          return SFUOTA_SizeCallback(packet, length);
        }
        return SFUOTA_StartCallback(packet, length);
      }
      return 0U;

    case SFUOTA_ST_DATAREQ:
      if (packet[0] == SFUOTA_HDR_DATAREQ)
      {
        return SFUOTA_DataRequestCallback(packet, length);
      }
      if (packet[0] == SFUOTA_HDR_DATA)
      {
        return SFUOTA_SendDataCallback(packet, length);
      }
      return 0U;

    default:
      return 0U;
  }
}

/**
  * @brief  Returns the currently queued SFUOTA transmit packet.
  * @param  packet Output pointer receiving the internal TX buffer address.
  * @param  length Output pointer receiving the TX packet length.
  * @retval 1U if a packet was pending and returned.
  * @retval 0U if no packet is pending or output pointers are invalid.
  */
static uint8_t SFUOTA_GetTxPacket(const uint8_t **packet, uint8_t *length)
{
  /* Reject invalid outputs or an empty queue. */
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
  * @brief  Services SFUOTA radio scheduling between TX and RX operations.
  * @param  None.
  * @retval None.
  */
static void SFUOTA_RadioService(void)
{
  const uint8_t *packet;
  uint8_t length;

  /* Send a pending packet first, otherwise keep the receiver armed. */
  if (tx_pending != 0U)
  {
    /* Radio not available yet: retry on the next tick. */
    if (SFUOTA_Radio_PrepareTx() != SFUOTA_RADIO_READY)
    {
      return;
    }

    if (SFUOTA_GetTxPacket(&packet, &length) != 0U)
    {
      if (SFUOTA_TRACE_TX_FRAMES == 1U)
      {
        for (uint8_t index = 0U; index < length; index++)
        {
          APP_DBG_MSG("%02X ", packet[index]);
        }
        APP_DBG_MSG("\r\n");
      }

      SFUOTA_Radio_Transmit(packet, length);
    }
    return;
  }

  /* If nothing to send, keep receiver armed. */
  SFUOTA_Radio_EnsureReceiving();
}
