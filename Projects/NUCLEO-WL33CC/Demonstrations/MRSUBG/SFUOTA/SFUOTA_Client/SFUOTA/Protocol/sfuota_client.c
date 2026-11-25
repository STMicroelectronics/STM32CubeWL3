/**
  ******************************************************************************
  * @file    sfuota_client.c
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA client protocol state machine.
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
#include "sfuota_client.h"
#include "main.h"

/* Private define -----------------------------------------------------------*/
#define SFUOTA_VERSION            "1.1.0"  /*!< SFUOTA version */
#define SFUOTA_MAX_RETRY          (10U)    /*!< Retries before falling back to the connection phase. */
#define SFUOTA_DEBUG_PERIOD_MS    (1000U)  /*!< Period for SFUOTA debug heartbeat print. */
#define SFUOTA_START_TIMEOUT_MS   (200U)   /*!< Timeout to retransmit handshake frame while waiting for peer ACK. */
#define SFUOTA_DATA_TIMEOUT_MS    (100U)   /*!< Timeout to reissue a data request whose answer never came. */
#define SFUOTA_SIZE_TIMEOUT_MS    (1000U)  /*!< Timeout to fall back when the announced size never arrives. */

/* Diagnostics. */
#define SFUOTA_TRACE_CHUNKS       (0U)     /*!< Logs every data request and flash step. Adds two lines per 16-byte chunk. */

/* Private variables --------------------------------------------------------*/
/* SFUOTA transport staging buffers and TX queue state. */
static uint8_t tx_buffer[SFUOTA_RADIO_MAX_PACKET];  /*!< SFUOTA transport TX buffer. */
static uint8_t tx_length = 0U;                      /*!< Length of pending TX packet. */
static uint8_t tx_pending = 0U;                     /*!< TX pending flag. */

/* SFUOTA protocol/session state and retry counters. */
static sfuota_state_t sfuota_state = SFUOTA_ST_CONNECTION;  /*!< SFUOTA protocol/session state. */
static uint16_t seq_num = 0U;                               /*!< Current SFUOTA sequence number. */
static uint16_t seq_num_max = 0U;                           /*!< Maximum SFUOTA sequence number. */
static uint32_t app_size = 0U;                              /*!< SFUOTA application size. */
static uint8_t retry_size = 0U;                             /*!< Retry counter for SIZE phase. */
static uint8_t retry_start = 0U;                            /*!< Retry counter for START phase. */
static uint8_t retry_getdata = 0U;                          /*!< Retry counter for GETDATA phase. */
static uint8_t bootloadingCompleted = 0U;                   /*!< SFUOTA bootloading completed flag. */
static uint8_t bootloadingCompleted_end = 0U;               /*!< SFUOTA bootloading completion end flag. */
static uint32_t handshake_retry_ms = 0U;                    /*!< Timestamp of the last handshake frame retransmission. */
static uint32_t data_retry_ms = 0U;                         /*!< Timestamp of the last data request sent to the peer. */

/* Debug telemetry counters. */
static uint32_t dbg_rx_ok_count = 0U;
static uint32_t dbg_tx_done_count = 0U;
static uint32_t dbg_rx_err_count = 0U;
static uint32_t dbg_rx_crc_err_count = 0U;
static uint32_t dbg_rx_other_err_count = 0U;
static uint32_t dbg_last_print_ms = 0U;

/* Private functions prototypes ----------------------------------------------*/
/* Image programming step. */
static uint8_t SFUOTA_FlashData(void);

/* SFUOTA protocol state callbacks. */
static uint8_t SFUOTA_ConnectionCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_SizeCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_StartCallback(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_GetDataCallback(const uint8_t *packet, uint16_t length);

/* Tick-driven protocol actions. */
static void SFUOTA_RejectImage(void);
static void SFUOTA_RequestNextChunk(void);

/* SFUOTA context and packet helpers. */
static void SFUOTA_ResetContext(uint8_t keep_size, uint8_t full_reset);
static void SFUOTA_QueueTx(const uint8_t *packet, uint8_t length);
static uint8_t SFUOTA_ProcessReceivedPacket(const uint8_t *packet, uint16_t length);
static uint8_t SFUOTA_GetTxPacket(const uint8_t **packet, uint8_t *length);

/* Radio scheduling helper. */
static void SFUOTA_RadioService(void);

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Initializes SFUOTA context and radio event handling.
  *
  * Performs a full SFUOTA context reset through SFUOTA_ResetContext(0U, 1U), then
  * initializes the radio adaptation layer and prints startup debug information.
  *
  * @param  None.
  * @retval 0U always.
  */
uint8_t SFUOTA_Init(void)
{
  /* Reset protocol, transfer, and radio runtime state before starting SFUOTA activity. */
  SFUOTA_ResetContext(0U, 1U);

  /* Enable radio event sources and arm reception. */
  SFUOTA_Radio_Init();
  SFUOTA_Radio_EnsureReceiving();

  /* Print SFUOTA client startup information. */
  APP_DBG_MSG("SFUOTA client (version: %s)\r\n", SFUOTA_VERSION);
  APP_DBG_MSG("Next free address location is: 0x%08lX\r\n", SFUOTA_Image_GetBaseAddress());
  return 0U;
}

/**
  * @brief  Processes deferred radio events and advances the SFUOTA client state machine.
  *
  * Consumes RX/TX/error events latched by SFUOTA_IRQHandler(), applies
  * phase-specific retry policy on RX errors, executes tick-driven states,
  * finalizes successful download completion, and services radio TX/RX scheduling.
  *
  * @param  None.
  * @retval 1U when SFUOTA completion end flag is set.
  * @retval 0U otherwise.
  */
uint8_t SFUOTA_Tick(void)
{
  uint32_t now_ms;
  sfuota_radio_event_t phy_events;

  /* Collect the radio events latched by SFUOTA_Radio_IRQHandler(). */
  SFUOTA_Radio_GetEvents(&phy_events);

  /* Handle RX-complete event. */
  if (phy_events.rx_ok == TRUE)
  {
    uint8_t packet_processed;

    BSP_LED_Toggle(LD1);
    packet_processed = SFUOTA_ProcessReceivedPacket(phy_events.rx_data, phy_events.rx_length);
    if (packet_processed != 0U)
    {
      dbg_rx_ok_count++;
    }
  }

  /* Handle TX-complete event. */
  if (phy_events.tx_done == TRUE)
  {
    dbg_tx_done_count++;
    BSP_LED_Toggle(LD2);
  }

  /* Handle deferred RX error event and apply phase-specific retry policy. */
  if (phy_events.rx_error == TRUE)
  {
    dbg_rx_err_count++;
    BSP_LED_Toggle(LD3);

    if ((phy_events.rx_error_flags & SFUOTA_RADIO_ERR_CRC) != 0U)
    {
      dbg_rx_crc_err_count++;
    }
    if ((phy_events.rx_error_flags & SFUOTA_RADIO_ERR_OTHER) != 0U)
    {
      dbg_rx_other_err_count++;
    }

    APP_DBG_MSG("SFUOTA_RXERR st=%u seq=%u err=0x%02X tot=%lu\r\n",
                (unsigned)sfuota_state,
                (unsigned)seq_num,
                phy_events.rx_error_flags,
                dbg_rx_err_count);

    if (sfuota_state == SFUOTA_ST_SIZE)
    {
      retry_size++;
      if (retry_size > SFUOTA_MAX_RETRY)
      {
        retry_size = 0U;
        sfuota_state = SFUOTA_ST_CONNECTION;
      }
    }
    else if (sfuota_state == SFUOTA_ST_START)
    {
      retry_start++;
      if (retry_start > SFUOTA_MAX_RETRY)
      {
        retry_start = 0U;
        sfuota_state = SFUOTA_ST_CONNECTION;
      }
    }
    else if (sfuota_state == SFUOTA_ST_GETDATA)
    {
      retry_getdata++;
      if (retry_getdata > SFUOTA_MAX_RETRY)
      {
        retry_getdata = 0U;
        sfuota_state = SFUOTA_ST_DATAREQ;
      }
    }
  }

  /* Sampled after the callbacks, which stamp their timers with HAL_GetTick():
     an earlier reading would sit in the past and fire every timeout at once. */
  now_ms = HAL_GetTick();

  /* Execute only states that require tick-side actions. */
  switch (sfuota_state)
  {
    case SFUOTA_ST_SIZE:
      /* Nothing is retransmitted from this side, so a lost SIZE frame would hang the
         session until the peer gave up: go back and let the handshake start over. */
      if ((now_ms - handshake_retry_ms) >= SFUOTA_SIZE_TIMEOUT_MS)
      {
        sfuota_state = SFUOTA_ST_CONNECTION;
      }
      break;

    case SFUOTA_ST_START:
      /* Re-queue the handshake frame until the peer acknowledges it. */
      if ((tx_pending == 0U) &&
          (SFUOTA_Radio_IsTxBusy() == 0U) &&
          ((now_ms - handshake_retry_ms) >= SFUOTA_START_TIMEOUT_MS))
      {
        tx_buffer[0] = SFUOTA_HDR_START;
        SFUOTA_QueueTx(tx_buffer, 1U);
        handshake_retry_ms = now_ms;
      }
      break;

    case SFUOTA_ST_DATAREQ:
      SFUOTA_RequestNextChunk();
      break;

    case SFUOTA_ST_FLASHDATA:
      /* Flash one assembled chunk/page and update SFUOTA progression. */
      if (SFUOTA_FlashData() == 0U)
      {
        return 0U;
      }
      break;

    case SFUOTA_ST_GETDATA:
      /* Half-duplex link: a lost request or answer raises no radio error, so only
         a timeout can break the deadlock with the peer waiting in DATAREQ. */
      if ((tx_pending == 0U) &&
          (SFUOTA_Radio_IsTxBusy() == 0U) &&
          ((now_ms - data_retry_ms) >= SFUOTA_DATA_TIMEOUT_MS))
      {
        retry_getdata++;
        if (retry_getdata > SFUOTA_MAX_RETRY)
        {
          APP_DBG_MSG("SFUOTA_DATA_LOST at seq %u, restarting session\r\n", (unsigned)seq_num);
          SFUOTA_ResetContext(0U, 1U);
        }
        else
        {
          sfuota_state = SFUOTA_ST_DATAREQ;
        }
      }
      break;

    case SFUOTA_ST_CONNECTION:
    case SFUOTA_ST_COMPLETE:
    default:
      /* Passive states: progression occurs from deferred events and callbacks. */
      break;
  }

  /* Finalize successful SFUOTA completion and queue terminal notification packet. */
  if (bootloadingCompleted != 0U)
  {
    SFUOTA_Boot_MarkImageValid();
    bootloadingCompleted = 0U;
    bootloadingCompleted_end = 1U;
    APP_DBG_MSG("** Over The Air FW upgrade completed with success! **\r\n");
    APP_DBG_MSG("** Application is JUMPING to new base address: 0x%08lX **\r\n", SFUOTA_Image_GetBaseAddress());

    tx_buffer[0] = SFUOTA_HDR_DATA;
    tx_buffer[1] = 0xFFU;
    tx_buffer[2] = 0xFFU;
    SFUOTA_QueueTx(tx_buffer, 3U);
  }

  /* Service radio scheduling: send pending TX first, otherwise ensure RX is armed. */
  SFUOTA_RadioService();

  /* Periodic heartbeat to diagnose handshake stalls without flooding logs. */
  if ((now_ms - dbg_last_print_ms) >= SFUOTA_DEBUG_PERIOD_MS)
  {
    dbg_last_print_ms = now_ms;
    APP_DBG_MSG("SFUOTA_CLI hb: st=%u app=%lu seq=%u/%u txp=%u txb=%u rxa=%u rxok=%lu txd=%lu rxerr=%lu crc=%lu oth=%lu\r\n",
                (unsigned)sfuota_state,
                app_size,
                (unsigned)seq_num,
                (unsigned)seq_num_max,
                tx_pending,
                SFUOTA_Radio_IsTxBusy(),
                SFUOTA_Radio_IsRxArmed(),
                dbg_rx_ok_count,
                dbg_tx_done_count,
                dbg_rx_err_count,
                dbg_rx_crc_err_count,
                dbg_rx_other_err_count);
  }

  /* Hold completion until the final notification frame has actually left the radio,
     otherwise the jump would truncate it and the server would stay in DATAREQ. */
  if ((bootloadingCompleted_end != 0U) &&
      ((tx_pending != 0U) || (SFUOTA_Radio_IsTxBusy() != 0U)))
  {
    return 0U;
  }

  return bootloadingCompleted_end;
}

/**
  * @brief  Forwards the radio interrupt to the PHY adaptation layer.
  * @param  None.
  * @retval None.
  */
void SFUOTA_IRQHandler(void)
{
  SFUOTA_Radio_IRQHandler();
}

/* Private functions --------------------------------------------------------*/
/**
  * @brief  Programs received SFUOTA data to flash memory during the FLASH DATA state.
  *
  * Delegates page assembly and programming to the image store, then advances the
  * state machine to the next data request or to completion.
  *
  * @retval 1U if the operation is successful.
  * @retval 0U on flash programming error (context is reset).
  */
static uint8_t SFUOTA_FlashData(void)
{
  if (SFUOTA_TRACE_CHUNKS == 1U)
  {
    APP_DBG_MSG("SFUOTA_FLASH %d/%d app_size %d ps %d\r\n", (int)seq_num, (int)seq_num_max, (int)app_size, (int)SFUOTA_Image_GetStagedSize());
  }

  if (SFUOTA_Image_Program(&app_size) == 0U)
  {
    APP_DBG_MSG("SFUOTA_FLASH_ERROR at seq %d, %d bytes left\r\n", (int)seq_num, (int)app_size);
    SFUOTA_ResetContext(0U, 0U);
    return 0U;
  }

  /* Advance to next data request or complete SFUOTA */
  if (app_size != 0U)
  {
    sfuota_state = SFUOTA_ST_DATAREQ;
  }
  else
  {
    sfuota_state = SFUOTA_ST_COMPLETE;
    APP_DBG_MSG("SFUOTA_COMPLETE\r\n");
    bootloadingCompleted = 1U;
  }

  return 1U;
}

/**
  * @brief  Handles callback logic for the CONNECTION state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the connection frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_ConnectionCallback(const uint8_t *packet, uint16_t length)
{
  /* Accept only the CONNECTION frame in this phase. */
  if ((packet == NULL) || (length == 0U) || (packet[0] != SFUOTA_HDR_CONNECTION))
  {
    return 0U;
  }

  /* Acknowledge connection and move to SIZE phase. */
  APP_DBG_MSG("SFUOTA_CONNECTION\r\n");
  tx_buffer[0] = SFUOTA_HDR_CONNECTION;
  SFUOTA_QueueTx(tx_buffer, 1U);
  handshake_retry_ms = HAL_GetTick();
  sfuota_state = SFUOTA_ST_SIZE;

  return 1U;
}

/**
  * @brief  Handles callback logic for the SIZE state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the size frame is processed.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_SizeCallback(const uint8_t *packet, uint16_t length)
{
  /* Expect SIZE header with 4-byte payload containing application size. */
  if ((packet == NULL) || (length != 5U) || (packet[0] != SFUOTA_HDR_SIZE))
  {
    retry_size++;
    if (retry_size > SFUOTA_MAX_RETRY)
    {
      retry_size = 0U;
      sfuota_state = SFUOTA_ST_CONNECTION;
    }
    return 0U;
  }

  app_size = 0U;
  /* Decode big-endian 32-bit application size. */
  for (uint8_t i = 0U; i < 4U; i++)
  {
    app_size |= ((uint32_t)packet[1U + i]) << ((3U - i) * 8U);
  }

  /* Reject zero-sized or oversized images. */
  if ((app_size == 0U) || (app_size > SFUOTA_Image_GetCapacity()))
  {
    APP_DBG_MSG("NOT OK for 0x%08lX %lu.%02lu KB max %lu.%02lu KB\r\n",
            app_size,
            (app_size / 1024UL),
            ((app_size % 1024UL) * 100UL) / 1024UL,
            (SFUOTA_Image_GetCapacity() / 1024UL),
            ((SFUOTA_Image_GetCapacity() % 1024UL) * 100UL) / 1024UL);

    SFUOTA_RejectImage();
    return 0U;
  }

  /* Acknowledge SIZE phase and move to START phase. */
  APP_DBG_MSG("SFUOTA_SIZE\r\n");
  APP_DBG_MSG("OK for 0x%08lX %lu.%02lu KB max %lu.%02lu KB\r\n",
          app_size,
          (app_size / 1024UL),
          ((app_size % 1024UL) * 100UL) / 1024UL,
          (SFUOTA_Image_GetCapacity() / 1024UL),
          ((SFUOTA_Image_GetCapacity() % 1024UL) * 100UL) / 1024UL);
  tx_buffer[0] = SFUOTA_HDR_SIZE;
  SFUOTA_QueueTx(tx_buffer, 1U);
  retry_size = 0U;

  seq_num_max = (uint16_t)(app_size / SFUOTA_FRAME_PAYLOAD);
  if ((app_size % SFUOTA_FRAME_PAYLOAD) != 0U)
  {
    seq_num_max++;
  }

  /* Backdate the retry stamp so the first START frame goes out on the next tick. */
  handshake_retry_ms = HAL_GetTick() - SFUOTA_START_TIMEOUT_MS;
  sfuota_state = SFUOTA_ST_START;

  return 1U;
}

/**
  * @brief  Handles callback logic for the START state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the start phase is processed.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_StartCallback(const uint8_t *packet, uint16_t length)
{
  (void)length;

  /* Accept only START header in this phase; otherwise keep retrying. */
  if ((packet == NULL) || (packet[0] != SFUOTA_HDR_START))
  {
    retry_start++;
    if (retry_start > SFUOTA_MAX_RETRY)
    {
      retry_start = 0U;
      sfuota_state = SFUOTA_ST_CONNECTION;
    }
    return 0U;
  }

  /* START accepted: preserve computed image size and current state,
    reset transfer context, erase destination area, then continue with DATAREQ phase. */
  APP_DBG_MSG("SFUOTA_START\r\n");
  retry_start = 0U;
  SFUOTA_ResetContext(1U, 0U);
  APP_DBG_MSG("START ACK received, requesting data\r\n");
  SFUOTA_Image_Erase();
  sfuota_state = SFUOTA_ST_DATAREQ;

  return 1U;
}

/**
  * @brief  Rejects the announced image and notifies the peer.
  * @retval None.
  */
static void SFUOTA_RejectImage(void)
{
  APP_DBG_MSG("SFUOTA_NOTSTART\r\n");
  /* Reset SFUOTA transfer context while keeping state machine ready for a new session. */
  SFUOTA_ResetContext(0U, 0U);
  /* Notify peer that START phase is rejected for current image parameters. */
  tx_buffer[0] = SFUOTA_HDR_NOTSTART;
  SFUOTA_QueueTx(tx_buffer, 1U);
}

/**
  * @brief  Requests the next image chunk, or moves to programming when all chunks are in.
  * @retval None.
  */
static void SFUOTA_RequestNextChunk(void)
{
  /* Wait until previous packet has been transmitted. */
  if (tx_pending != 0U)
  {
    return;
  }

  if (seq_num < seq_num_max)
  {
    /* Request next chunk by sequence number and switch to GETDATA phase. */
    if (SFUOTA_TRACE_CHUNKS == 1U)
    {
      APP_DBG_MSG("SFUOTA_DATAREQ %d/%d app_size %d\r\n", (int)seq_num, (int)seq_num_max, (int)app_size);
    }
    tx_buffer[0] = SFUOTA_HDR_DATAREQ;
    tx_buffer[1] = (uint8_t)(seq_num >> 8);
    tx_buffer[2] = (uint8_t)seq_num;
    SFUOTA_QueueTx(tx_buffer, 3U);
    data_retry_ms = HAL_GetTick();
    sfuota_state = SFUOTA_ST_GETDATA;
  }
  else
  {
    /* All chunks requested, continue with flash programming phase. */
    sfuota_state = SFUOTA_ST_FLASHDATA;
  }
}

/**
  * @brief  Handles callback logic for the GET DATA state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if a valid in-sequence data frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t SFUOTA_GetDataCallback(const uint8_t *packet, uint16_t length)
{
  uint16_t sequence_number_received;
  uint8_t payload_length;

  /* Validate packet pointer, frame header, and presence of at least one image byte. */
  if ((packet == NULL) || (length <= 3U) || (packet[0] != SFUOTA_HDR_DATA))
  {
    /* Increment retry counter and revert to data request state if max retries exceeded. */
    retry_getdata++;
    if (retry_getdata > SFUOTA_MAX_RETRY)
    {
      retry_getdata = 0U;
      sfuota_state = SFUOTA_ST_DATAREQ;
    }
    return 0U;
  }

  payload_length = (uint8_t)(length - 3U);
  if (((uint32_t)SFUOTA_Image_GetStagedSize() + (uint32_t)payload_length) > FLASH_PAGE_SIZE)
  {
    /* Reject frames that would overflow the page assembly buffer. */
    retry_getdata++;
    if (retry_getdata > SFUOTA_MAX_RETRY)
    {
      retry_getdata = 0U;
      sfuota_state = SFUOTA_ST_DATAREQ;
    }
    return 0U;
  }

  /* Extract sequence number from packet and check if it matches expected value. */
  sequence_number_received = (((uint16_t)packet[1]) << 8) | ((uint16_t)packet[2]);
  if (seq_num == sequence_number_received)
  {
    /* Image bytes start after header and sequence: 3 bytes in. */
    if (SFUOTA_Image_Stage(&packet[3], payload_length) == 0U)
    {
      return 0U;
    }

    seq_num++;
    retry_getdata = 0U;
    sfuota_state = SFUOTA_ST_FLASHDATA;
    return 1U;
  }

  /* Sequence mismatch: do not advance state, retry same chunk request. */
  retry_getdata++;
  if (retry_getdata > SFUOTA_MAX_RETRY)
  {
    retry_getdata = 0U;
  }
  sfuota_state = SFUOTA_ST_DATAREQ;
  return 0U;
}

/**
  * @brief  Resets SFUOTA transfer context and, optionally, protocol and radio runtime state.
  * @param  keep_size If non-zero, preserve negotiated size information and current SFUOTA state.
  * @param  full_reset If non-zero, also clear deferred radio events and local radio runtime flags.
  * @retval None.
  */
static void SFUOTA_ResetContext(uint8_t keep_size, uint8_t full_reset)
{
  /* Clear the TX buffer. */
  for (uint16_t index = 0U; index < (uint16_t)sizeof(tx_buffer); index++)
  {
    tx_buffer[index] = 0U;
  }

  /* Drop any staged image data and restart from the first destination address. */
  SFUOTA_Image_Reset();

  /* Reinitialize SFUOTA transfer state, retry counters, and completion flags. */
  tx_length = 0U;
  tx_pending = 0U;
  seq_num = 0U;
  retry_size = 0U;
  retry_start = 0U;
  retry_getdata = 0U;
  bootloadingCompleted = 0U;
  bootloadingCompleted_end = 0U;
  handshake_retry_ms = dbg_last_print_ms;

  /* Full context reset returns to connection state and drops negotiated size information. */
  if (keep_size == 0U)
  {
    app_size = 0U;
    seq_num_max = 0U;
    sfuota_state = SFUOTA_ST_CONNECTION;
  }

  /* Only full reset paths flush IRQ-deferred events and radio scheduler state. */
  if (full_reset != 0U)
  {
    /* Drop deferred radio events and clear the radio scheduler state. */
    SFUOTA_Radio_Reset();
    dbg_rx_ok_count = 0U;
    dbg_tx_done_count = 0U;
    dbg_rx_err_count = 0U;
    dbg_rx_crc_err_count = 0U;
    dbg_rx_other_err_count = 0U;
    dbg_last_print_ms = HAL_GetTick();
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

    case SFUOTA_ST_SIZE:
      return SFUOTA_SizeCallback(packet, length);

    case SFUOTA_ST_START:
      return SFUOTA_StartCallback(packet, length);

    case SFUOTA_ST_GETDATA:
      return SFUOTA_GetDataCallback(packet, length);

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

  /* Prioritize pending TX packet transmission. */
  if (tx_pending != 0U)
  {
    /* Radio not available yet: retry on the next tick. */
    if (SFUOTA_Radio_PrepareTx() != SFUOTA_RADIO_READY)
    {
      return;
    }

    if (SFUOTA_GetTxPacket(&packet, &length) != 0U)
    {
      SFUOTA_Radio_Transmit(packet, length);
    }
    return;
  }

  /* If nothing to send, keep receiver armed. */
  SFUOTA_Radio_EnsureReceiving();
}
