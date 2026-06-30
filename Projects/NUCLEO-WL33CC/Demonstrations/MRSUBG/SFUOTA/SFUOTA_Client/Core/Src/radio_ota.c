/**
  ******************************************************************************
  * @file    radio_ota.c
  * @author  GPAM Wireless Application Team
  * @brief   OTA client utilities for the WL33 SFUOTA client application.
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

#define OTA_WORD_SIZE (4U)    /*!< Flash write granularity used by OTA_FlashData. */

#define CFG_OTA_REBOOT_VAL_MSG RAM_VR.bootloader_vr   /*!< Mapping of OTA retained message in SRAM. */
#define OTA_DEBUG_PERIOD_MS     (1000U)               /*!< Period for OTA debug heartbeat print. */
#define OTA_START_TIMEOUT_MS      (200U)                /*!< Timeout to retransmit handshake frame while waiting for peer ACK. */

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
static uint8_t RadioRxArmed = 0U;                                                   /*!< Radio RX armed flag. */

/* OTA transport staging buffers and TX queue state. */
static uint8_t tx_buffer[MAX_PACKET_LENGTH];                                        /*!< OTA transport TX buffer. */
static uint8_t tx_length = 0U;                                                      /*!< Length of pending TX packet. */
static uint8_t tx_pending = 0U;                                                     /*!< TX pending flag. */

/* OTA protocol/session state and retry counters. */
static ota_state_machine_t ota_state_machine_g = OTA_CONNECTION;                    /*!< OTA protocol/session state. */
static uint16_t seq_num = 0U;                                                       /*!< Current OTA sequence number. */
static uint16_t seq_num_max = 0U;                                                   /*!< Maximum OTA sequence number. */
static uint32_t app_size = 0U;                                                      /*!< OTA application size. */
static uint8_t retry_size = 0U;                                                     /*!< Retry counter for SIZE phase. */
static uint8_t retry_start = 0U;                                                    /*!< Retry counter for START phase. */
static uint8_t retry_getdata = 0U;                                                  /*!< Retry counter for GETDATA phase. */
static uint16_t retry_datareq = 0U;                                                 /*!< Retry counter for DATAREQ phase. */
static uint8_t bootloadingCompleted = 0U;                                           /*!< OTA bootloading completed flag. */
static uint8_t bootloadingCompleted_end = 0U;                                       /*!< OTA bootloading completion end flag. */

/* Flash destination and current page assembly context. */
static uint32_t write_address = APP_WITH_OTA_SERVICE_ADDRESS;                       /*!< Flash destination write address. */
static __attribute__((aligned(4))) uint8_t page_buffer[FLASH_PAGE_SIZE] = {0U};     /*!< Flash page assembly buffer. */
static uint16_t page_size = 0U;                                                     /*!< Current page buffer size. */
static uint16_t page_currently_written = 0U;                                        /*!< Number of pages written so far. */

/* Debug telemetry counters. */
static uint32_t dbg_rx_ok_count = 0U;
static uint32_t dbg_tx_done_count = 0U;
static uint32_t dbg_rx_err_count = 0U;
static uint32_t dbg_rx_crc_err_count = 0U;
static uint32_t dbg_rx_other_err_count = 0U;
static uint32_t dbg_last_print_ms = 0U;
static uint32_t dbg_conn_retry_since_ms = 0U;

/* Private functions prototypes ----------------------------------------------*/
/* Flash/tag helpers. */
static void OTA_Set_Application_Tag_Value(uint32_t address, uint32_t data);
static void OTA_Erase_Flash(uint16_t startNumber, uint16_t endNumber);
static HAL_StatusTypeDef OTA_ProgramWord(uint32_t address, uint32_t data);
static uint8_t OTA_FlashData(void);

/* OTA protocol state callbacks. */
static uint8_t OTA_ConnectionCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_SizeCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_StartCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_NotStartCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_DataRequestCallback(const uint8_t *packet, uint16_t length);
static uint8_t OTA_GetDataCallback(const uint8_t *packet, uint16_t length);

/* OTA context and packet helpers. */
static void OTA_ResetContext(uint8_t keep_size, uint8_t full_reset);
static void OTA_QueueTx(const uint8_t *packet, uint8_t length);
static uint8_t OTA_ProcessReceivedPacket(const uint8_t *packet, uint16_t length);
static uint8_t OTA_GetTxPacket(const uint8_t **packet, uint8_t *length);

/* Radio operation helpers. */
static void OTA_Radio_StartReception(void);
static void OTA_Radio_StartTransmission(const uint8_t *packet, uint8_t length);
static void OTA_Radio_Service(void);

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Initializes OTA context and radio event handling.
  *
  * Performs a full OTA context reset through OTA_ResetContext(0U, 1U),
  * enables and clears the RFSEQ IRQ sources used by OTA processing, arms
  * radio reception, and prints startup debug information.
  *
  * @param  None.
  * @retval 0U always.
  */
uint8_t OTA_Init(void)
{
  /* Reset protocol, transfer, and radio runtime state before starting OTA activity. */
  OTA_ResetContext(0U, 1U);

  /* Enable RX/TX and RF error IRQ sources used by OTA event processing. */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E |
                           //         MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SABORT_DONE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_AHB_ACCESS_ERROR_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_COMMAND_REJECTED_E);

  /* Clear pending RFSEQ IRQ flags before starting OTA activity. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F |
                                    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F |
                                    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F |
                                    RFSEQ_IRQ_ERROR_MASK);

  /* Put radio in receive mode to wait for OTA peer frames. */
  OTA_Radio_StartReception();

  /* Print OTA client startup information. */
  APP_DBG_MSG("OTA manager client (version: %s)\r\n", RADIO_OTA_MANAGER_VERSION_STRING);
  APP_DBG_MSG("Next free address location is: 0x%08lX\r\n", APP_WITH_OTA_SERVICE_ADDRESS);
  return 0U;
}

/**
  * @brief  Processes deferred radio events and advances the OTA client state machine.
  *
  * Consumes RX/TX/error events latched by OTA_Radio_IRQHandler(), applies
  * phase-specific retry policy on RX errors, executes tick-driven states,
  * finalizes successful download completion, and services radio TX/RX scheduling.
  *
  * @param  None.
  * @retval 1U when OTA completion end flag is set.
  * @retval 0U otherwise.
  */
uint8_t OTA_Tick(void)
{
  uint32_t now_ms = HAL_GetTick();

  /* Handle RX-complete event latched by OTA_Radio_IRQHandler(). */
  if (RxOkEventPending == TRUE)
  {
    uint8_t packet_processed;

    RxOkEventPending = FALSE;
    RadioRxArmed = 0U;
    BSP_LED_Toggle(LD1);
    RxReceivedByteCount = __HAL_MRSUBG_GET_DATABUFFER_COUNT();
    packet_processed = OTA_ProcessReceivedPacket(RadioRxPacketBuffer, RxReceivedByteCount);
    if (packet_processed != 0U)
    {
      dbg_rx_ok_count++;
    }
  }

  /* Handle TX-complete event latched by OTA_Radio_IRQHandler(). */
  if (TxDoneEventPending == TRUE)
  {
    TxDoneEventPending = FALSE;
    dbg_tx_done_count++;
    RadioTxBusy = 0U;
    BSP_LED_Toggle(LD3);
  }

  /* Handle deferred RX error event and apply phase-specific retry policy. */
  if (RxErrorEventPending == TRUE)
  {
    uint32_t irq_error;

    RxErrorEventPending = FALSE;
    dbg_rx_err_count++;
    RadioRxArmed = 0U;
    BSP_LED_Toggle(LD2);
    irq_error = RxErrorIrqStatus;
    if ((irq_error & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F) != 0U)
    {
      dbg_rx_crc_err_count++;
    }
    if ((irq_error & (~MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F)) != 0U)
    {
      dbg_rx_other_err_count++;
    }
    RxErrorIrqStatus = 0U;

    if (ota_state_machine_g == OTA_SIZE)
    {
      retry_size++;
      if (retry_size > MAX_RETRY)
      {
        retry_size = 0U;
        ota_state_machine_g = OTA_CONNECTION;
      }
    }
    else if (ota_state_machine_g == OTA_START)
    {
      retry_start++;
      if (retry_start > MAX_RETRY)
      {
        retry_start = 0U;
        ota_state_machine_g = OTA_CONNECTION;
      }
    }
    else if (ota_state_machine_g == OTA_GETDATA)
    {
      retry_getdata++;
      if (retry_getdata > MAX_RETRY)
      {
        retry_getdata = 0U;
        ota_state_machine_g = OTA_DATAREQ;
      }
    }
    else if (ota_state_machine_g == OTA_DATAREQ)
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
    case OTA_SIZE:
      /* SIZE payload parsing and state transition are handled in OTA_SizeCallback(). */
     //APP_DBG_MSG("OTA_SIZE\r\n");
      break;

    case OTA_START:
      /* app_size validity is guaranteed by OTA_SizeCallback before entering OTA_START. */
      if ((tx_pending == 0U) && (RadioTxBusy == 0U))
      {
        APP_DBG_MSG("OK for 0x%08lX %lu.%02lu KB max %lu.%02lu KB\r\n",
                app_size,
                (app_size / 1024UL),
                ((app_size % 1024UL) * 100UL) / 1024UL,
                (SM_APP_SIZE / 1024UL),
                ((SM_APP_SIZE % 1024UL) * 100UL) / 1024UL);
        APP_DBG_MSG("OTA_START\r\n");
        tx_buffer[0] = HEADER_START;
        tx_buffer[1] = 0U;

        seq_num_max = (uint16_t)(app_size / BYTE_IN_FRAME);
        if ((app_size % BYTE_IN_FRAME) != 0U)
        {
          seq_num_max++;
        }

        OTA_QueueTx(tx_buffer, 2U);
      }
      break;

    case OTA_DATAREQ:
      (void)OTA_DataRequestCallback(NULL, 0U);
      break;

    case OTA_FLASHDATA:
      /* Flash one assembled chunk/page and update OTA progression. */
      if (OTA_FlashData() == 0U)
      {
        return 0U;
      }
      break;
      
    case OTA_CONNECTION:
    case OTA_GETDATA:
    case OTA_COMPLETE:
    case OTA_ONGOING:
    default:
      /* Passive states: progression occurs from deferred events and callbacks. */
      break;
  }

    /* While waiting for handshake ack, periodically re-queue the current handshake frame. */
  if ((ota_state_machine_g == OTA_START) &&
      (tx_pending == 0U) &&
      (RadioTxBusy == 0U) &&
      ((now_ms - dbg_conn_retry_since_ms) >= OTA_START_TIMEOUT_MS))
  {
    tx_buffer[0] = HEADER_START;
    tx_buffer[1] = 0U;
    OTA_QueueTx(tx_buffer, 2U);
    dbg_conn_retry_since_ms = now_ms;
    }
  
  /* Finalize successful OTA completion and queue terminal notification packet. */
  if (bootloadingCompleted != 0U)
  {
    OTA_Set_Application_Tag_Value(APP_WITH_OTA_SERVICE_ADDRESS, OTA_VALID_TAG);
    bootloadingCompleted = 0U;
    bootloadingCompleted_end = 1U;
    APP_DBG_MSG("** Over The Air FW upgrade completed with success! **\r\n");
    APP_DBG_MSG("** Application is JUMPING to new base address: 0x%08lX **\r\n", APP_WITH_OTA_SERVICE_ADDRESS);

    tx_buffer[0] = HEADER_SENDATA;
    tx_buffer[1] = 2U;
    tx_buffer[2] = 0xFFU;
    tx_buffer[3] = 0xFFU;
    OTA_QueueTx(tx_buffer, 4U);
  }

  /* Service radio scheduling: send pending TX first, otherwise ensure RX is armed. */
  OTA_Radio_Service();

  /* Periodic heartbeat to diagnose handshake stalls without flooding logs. */
  if ((now_ms - dbg_last_print_ms) >= OTA_DEBUG_PERIOD_MS)
  {
    dbg_last_print_ms = now_ms;
    APP_DBG_MSG("OTA_CLI hb: ota=%u app=%lu seq=%u/%u txp=%u txb=%u rxa=%u rxok=%lu txd=%lu rxerr=%lu crc=%lu oth=%lu\r\n",
                (unsigned)ota_state_machine_g,
                app_size,
                (unsigned)seq_num,
                (unsigned)seq_num_max,
                tx_pending,
                RadioTxBusy,
                RadioRxArmed,
                dbg_rx_ok_count,
                dbg_tx_done_count,
                dbg_rx_err_count,
                dbg_rx_crc_err_count,
                dbg_rx_other_err_count);
  }

  return bootloadingCompleted_end;
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
  * @brief  Handles retained boot command for OTA Service Manager request.
  *
  * If retained RAM requests OTA manager boot, this function invalidates the
  * downloadable application tag and clears the retained request marker.
  *
  * @param  None.
  * @retval None.
  */
void OTA_Check_ServiceManager_Operation(void)
{
  if (CFG_OTA_REBOOT_VAL_MSG == OTA_SWITCH_TO_OTA_RESET_MANAGER)
  {
    /* Set invalid tag for the OTA application to allow jumping to OTA service manager. */
    OTA_Set_Application_Tag_Value(APP_WITH_OTA_SERVICE_ADDRESS, OTA_INVALID_OLD_TAG);

    /* Reset the retained service manager request after it has been consumed. */
    CFG_OTA_REBOOT_VAL_MSG = 0U;
  }
}

/**
  * @brief  Selects the boot target by checking OTA validity tags.
  *
  * Compares service-manager and downloadable-application tags and returns the
  * base address of the image to run.
  *
  * @param  None.
  * @retval Base address of selected application image.
  * @retval 0U when no valid application target is available.
  */
uint32_t OTA_Check_Application_Tags_Value(void)
{
  uint32_t appAddress = 0U;

  /* If OTA_SERVICE is tagged as service manager and APP is in-progress/invalid-old, go to OTA service application. */
  if (((TAG_VALUE(APP_OTA_SERVICE_ADDRESS) == OTA_SERVICE_MANAGER_TAG) &&
       (TAG_VALUE(APP_WITH_OTA_SERVICE_ADDRESS) == OTA_IN_PROGRESS_TAG)) ||
      ((TAG_VALUE(APP_OTA_SERVICE_ADDRESS) == OTA_SERVICE_MANAGER_TAG) &&
       (TAG_VALUE(APP_WITH_OTA_SERVICE_ADDRESS) == OTA_INVALID_OLD_TAG)))
  {
    /* Jump to OTA Service Manager application. */
    appAddress = APP_OTA_SERVICE_ADDRESS;
  }
  /* If OTA_SERVICE is tagged as service manager and APP is valid, go to the main application. */
  else if ((TAG_VALUE(APP_OTA_SERVICE_ADDRESS) == OTA_SERVICE_MANAGER_TAG) &&
           (TAG_VALUE(APP_WITH_OTA_SERVICE_ADDRESS) == OTA_VALID_TAG))
  {
    /* Jump to application using OTA service manager. */
    appAddress = APP_WITH_OTA_SERVICE_ADDRESS;
  }

  return appAddress;
}

/**
  * @brief  Jumps to the selected valid application image.
  * @param  None
  * @retval None
  */
void OTA_Jump_To_New_Application(void)
{
  uint32_t applicationAddress = OTA_Check_Application_Tags_Value();
  uint32_t appStackPointer;
  uint32_t appResetHandler;
  void (*applicationResetHandler)(void);

  /* Abort if no valid application address is available. */
  if (applicationAddress == 0U)
  {
    return;
  }

  /* Read initial stack pointer and reset handler from application vector table. */
  appStackPointer = *((volatile uint32_t *)applicationAddress);
  appResetHandler = *((volatile uint32_t *)(applicationAddress + 4U));
  applicationResetHandler = (void (*)(void))appResetHandler;

  /* Stop current runtime context before handing control to the application. */
 __disable_irq();
 HAL_DeInit();
 SysTick->CTRL = 0U;
 SysTick->LOAD = 0U;
 SysTick->VAL = 0U;

  /* Switch vector table and stack pointer, then jump to application reset handler. */
  RAM_VR.AppBase = applicationAddress;
  SCB->VTOR = applicationAddress;
  __set_MSP(appStackPointer);
  applicationResetHandler();
}

/* Private functions --------------------------------------------------------*/
/**
  * @brief  Programs OTA validity tag at the application tag vector-table offset.
  * @param  address Base address of the application image.
  * @param  data Tag value to program.
  * @retval None.
  */
static void OTA_Set_Application_Tag_Value(uint32_t address, uint32_t data)
{
  /* Program tag word using the WL3 FLASH HAL API. */
  (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET, data);
}

/**
  * @brief  Erases destination flash pages before starting OTA upgrade session.
  * @param  startNumber First flash page to erase.
  * @param  endNumber Last flash page to erase (inclusive).
  * @retval None.
  */
static void OTA_Erase_Flash(uint16_t startNumber, uint16_t endNumber)
{
  FLASH_EraseInitTypeDef p_erase_init = {0};
  uint32_t page_error = 0U;

  p_erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  p_erase_init.Page = startNumber;
  p_erase_init.NbPages = (endNumber - startNumber) + 1U;

  /* Erase the selected page range. */
  (void)HAL_FLASHEx_Erase(&p_erase_init, &page_error);
}

/**
  * @brief  Programs one 32-bit word to flash memory.
  * @param  address Destination flash address.
  * @param  data 32-bit data word to program.
  * @retval HAL status returned by HAL_FLASH_Program.
  *
  * @note Used by OTA_FlashData during page-buffer flush to flash.
  */
static HAL_StatusTypeDef OTA_ProgramWord(uint32_t address, uint32_t data)
{
  return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
}

/**
  * @brief  Programs received OTA data to flash memory during the FLASH DATA state.
  *
  * Writes the contents of the page buffer to flash, handling alignment and vector table
  * overwrite for the first page. Erases the buffer after writing, updates progress, and
  * transitions the OTA state machine to the next state or completion.
  *
  * @retval 1U if the operation is successful.
  * @retval 0U on flash programming error (context is reset).
  */
static uint8_t OTA_FlashData(void)
{
  int32_t app_size_tmp;
  uint16_t page_limit;
  uint32_t data_word;

  /* Print debug info: current sequence, total, app size, and page size */
  APP_DBG_MSG("OTA_FD %d/%d app_size %d ps %d\r\n", (int)seq_num, (int)seq_num_max, (int)app_size, (int)page_size);

  /* Check if a full page or the last chunk is ready to be written */
  if ((page_size == FLASH_PAGE_SIZE) || ((app_size - page_size) == 0U))
  {
    app_size_tmp = (int32_t)app_size - (int32_t)page_size;
    if (app_size_tmp < 0)
    {
      return 0U;
    }

    /* Overwrite vector table entry with 0xFF for the first page */
    if ((page_currently_written == 0U) && (OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET < FLASH_PAGE_SIZE))
    {
      page_buffer[OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET + 0U] = 0xFFU;
      page_buffer[OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET + 1U] = 0xFFU;
      page_buffer[OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET + 2U] = 0xFFU;
      page_buffer[OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET + 3U] = 0xFFU;
    }

    page_limit = page_size;
    /* Pad buffer to OTA_WORD_SIZE alignment if needed */
    if ((page_limit % OTA_WORD_SIZE) != 0U)
    {
      for (uint16_t i = page_limit; i < (uint16_t)(page_limit + (OTA_WORD_SIZE - (page_limit % OTA_WORD_SIZE))); i++)
      {
        page_buffer[i] = 0xFFU;
      }
      page_limit = (uint16_t)(page_limit + (OTA_WORD_SIZE - (page_limit % OTA_WORD_SIZE)));
    }

    /* Program each word to flash */
    for (uint16_t i = 0U; i < page_limit; i = (uint16_t)(i + OTA_WORD_SIZE))
    {
      data_word = ((uint32_t)page_buffer[i + 0U]) |
                  (((uint32_t)page_buffer[i + 1U]) << 8) |
                  (((uint32_t)page_buffer[i + 2U]) << 16) |
                  (((uint32_t)page_buffer[i + 3U]) << 24);

      if (OTA_ProgramWord(write_address + i, data_word) != HAL_OK)
      {
        OTA_ResetContext(0U, 0U);
        return 0U;
      }
    }

    /* Update write address and clear buffer */
    write_address += page_limit;
    page_currently_written++;
    for (uint16_t i = 0U; i < page_size; i++)
    {
      page_buffer[i] = 0U;
    }
    app_size = (uint32_t)app_size_tmp;
    page_size = 0U;

    /* Print progress */
    APP_DBG_MSG("app_size 0x%08lX %d.%02d KB\r\n",
        app_size,
        (int)(app_size / 1024UL),
        (int)(((app_size % 1024UL) * 100UL) / 1024UL));
  }

  /* Advance to next data request or complete OTA */
  if (app_size != 0U)
  {
    ota_state_machine_g = OTA_DATAREQ;
  }
  else
  {
    ota_state_machine_g = OTA_COMPLETE;
    APP_DBG_MSG("OTA_COMPLETE\r\n");
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
static uint8_t OTA_ConnectionCallback(const uint8_t *packet, uint16_t length)
{
  if ((packet == NULL) || (length == 0U) || (packet[0] != HEADER_CONNECTION))
  {
    return 0U;
  }

  /* Acknowledge connection and move to SIZE phase. */
  tx_buffer[0] = HEADER_CONNECTION;
  tx_buffer[1] = 0U;
  OTA_QueueTx(tx_buffer, 2U);
  ota_state_machine_g = OTA_SIZE;

  return 1U;
}

/**
  * @brief  Handles callback logic for the SIZE state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the size frame is processed.
  * @retval 0U otherwise.
  */
static uint8_t OTA_SizeCallback(const uint8_t *packet, uint16_t length)
{
  /* Expect SIZE header with 4-byte payload containing application size. */
  if ((packet == NULL) || (length < 6U) || (packet[0] != HEADER_SIZE) || (packet[1] != 4U))
  {
    retry_size++;
    if (retry_size > MAX_RETRY)
    {
      retry_size = 0U;
      ota_state_machine_g = OTA_CONNECTION;
    }
    return 0U;
  }

  app_size = 0U;
  /* Decode big-endian 32-bit application size. */
  for (uint8_t i = 0U; i < 4U; i++)
  {
    app_size |= ((uint32_t)packet[2U + i]) << ((3U - i) * 8U);
  }

  /* Reject zero-sized or oversized images. */
  if ((app_size == 0U) || (app_size > SM_APP_SIZE))
  {
    APP_DBG_MSG("NOT OK for 0x%08lX %lu.%02lu KB max %lu.%02lu KB\r\n",
            app_size,
            (app_size / 1024UL),
            ((app_size % 1024UL) * 100UL) / 1024UL,
            (SM_APP_SIZE / 1024UL),
            ((SM_APP_SIZE % 1024UL) * 100UL) / 1024UL);

    return OTA_NotStartCallback(packet, length);
  }

  /* Acknowledge SIZE phase and move to START phase. */
  tx_buffer[0] = HEADER_SIZE;
  tx_buffer[1] = 0U;
  OTA_QueueTx(tx_buffer, 2U);
  retry_size = 0U;
  ota_state_machine_g = OTA_START;

  return 1U;
}

/**
  * @brief  Handles callback logic for the START state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the start phase is processed.
  * @retval 0U otherwise.
  */
static uint8_t OTA_StartCallback(const uint8_t *packet, uint16_t length)
{
  (void)length;

  /* Accept only START header in this phase; otherwise keep retrying. */
  if ((packet == NULL) || (packet[0] != HEADER_START))
  {
    retry_start++;
    if (retry_start > MAX_RETRY)
    {
      retry_start = 0U;
      ota_state_machine_g = OTA_CONNECTION;
    }
    return 0U;
  }

  /* START accepted: preserve computed image size and current state,
    reset transfer context, erase destination area, then continue with DATAREQ phase. */
  retry_start = 0U;
  OTA_ResetContext(1U, 0U);
  APP_DBG_MSG("START ACK received, requesting data\r\n");
  OTA_Erase_Flash(APP_WITH_OTA_SERVICE_PAGE_NUMBER_START, APP_WITH_OTA_SERVICE_PAGE_NUMBER_END);
  ota_state_machine_g = OTA_DATAREQ;

  return 1U;
}

/**
  * @brief  Handles callback logic for the NOT START state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U always.
  */
static uint8_t OTA_NotStartCallback(const uint8_t *packet, uint16_t length)
{
  (void)packet;
  (void)length;

  APP_DBG_MSG("OTA_NOTSTART\r\n");
  /* Reset OTA transfer context while keeping state machine ready for a new session. */
  OTA_ResetContext(0U, 0U);
  /* Notify peer that START phase is rejected for current image parameters. */
  tx_buffer[0] = HEADER_NOTSTART;
  tx_buffer[1] = 0U;
  OTA_QueueTx(tx_buffer, 2U);

  return 1U;
}

/**
  * @brief  Handles callback logic for the DATA REQUEST state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if the request is queued.
  * @retval 0U if a packet is still pending.
  */
static uint8_t OTA_DataRequestCallback(const uint8_t *packet, uint16_t length)
{
  (void)packet;
  (void)length;

  /* Wait until previous packet has been transmitted. */
  if (tx_pending != 0U)
  {
    return 0U;
  }

  if (seq_num < seq_num_max)
  {
    /* Request next chunk by sequence number and switch to GETDATA phase. */
    // APP_DBG_MSG("OTA_DATAREQ %d/%d app_size %d\r\n", (int)seq_num, (int)seq_num_max, (int)app_size);
    tx_buffer[0] = HEADER_DATAREQ;
    tx_buffer[1] = 2U;
    tx_buffer[2] = (uint8_t)(seq_num >> 8);
    tx_buffer[3] = (uint8_t)seq_num;
    OTA_QueueTx(tx_buffer, 4U);
    retry_datareq = 0U;
    ota_state_machine_g = OTA_GETDATA;
  }
  else
  {
    /* All chunks requested, continue with flash programming phase. */
    ota_state_machine_g = OTA_FLASHDATA;
  }

  return 1U;
}

/**
  * @brief  Handles callback logic for the GET DATA state.
  * @param  packet Received packet buffer.
  * @param  length Received packet length.
  * @retval 1U if a valid in-sequence data frame is accepted.
  * @retval 0U otherwise.
  */
static uint8_t OTA_GetDataCallback(const uint8_t *packet, uint16_t length)
{
  uint16_t sequence_number_received;
  uint8_t payload_length;

  /* Validate packet pointer, frame header, payload length field, and minimum body length. */
  if ((packet == NULL) || (length < 4U) || (packet[0] != HEADER_GETDATA) || (packet[1] <= 2U))
  {
    /* Increment retry counter and revert to data request state if max retries exceeded. */
    retry_getdata++;
    if (retry_getdata > MAX_RETRY)
    {
      retry_getdata = 0U;
      ota_state_machine_g = OTA_DATAREQ;
    }
    return 0U;
  }

  /* Packet[1] is protocol payload length (sequence + data) and total frame is payload + 2 header bytes. */
  if (length < (uint16_t)(packet[1] + 2U))
  {
    retry_getdata++;
    if (retry_getdata > MAX_RETRY)
    {
      retry_getdata = 0U;
      ota_state_machine_g = OTA_DATAREQ;
    }
    return 0U;
  }

  payload_length = (uint8_t)(packet[1] - 2U);
  if (((uint32_t)page_size + (uint32_t)payload_length) > FLASH_PAGE_SIZE)
  {
    /* Reject frames that would overflow the page assembly buffer. */
    retry_getdata++;
    if (retry_getdata > MAX_RETRY)
    {
      retry_getdata = 0U;
      ota_state_machine_g = OTA_DATAREQ;
    }
    return 0U;
  }

  /* Extract sequence number from packet and check if it matches expected value. */
  sequence_number_received = (((uint16_t)packet[2]) << 8) | ((uint16_t)packet[3]);
  if (seq_num == sequence_number_received)
  {
    /* Copy received data to page buffer. */
    for (uint8_t i = 0U; i < payload_length; i++)
    {
      page_buffer[page_size + i] = packet[4U + i];
    }

    /* Update page size and sequence number. */
    page_size = (uint16_t)(page_size + payload_length);
    seq_num++;
    retry_getdata = 0U;
    ota_state_machine_g = OTA_FLASHDATA;
    return 1U;
  }

  /* Sequence mismatch: do not advance state, retry same chunk request. */
  retry_getdata++;
  if (retry_getdata > MAX_RETRY)
  {
    retry_getdata = 0U;
  }
  ota_state_machine_g = OTA_DATAREQ;
  return 0U;
}

/**
  * @brief  Resets OTA transfer context and, optionally, protocol and radio runtime state.
  * @param  keep_size If non-zero, preserve negotiated size information and current OTA state.
  * @param  full_reset If non-zero, also clear deferred radio events and local radio runtime flags.
  * @retval None.
  */
static void OTA_ResetContext(uint8_t keep_size, uint8_t full_reset)
{
  /* Clear runtime buffers used by OTA TX and flash page staging. */
  for (uint16_t index = 0U; index < (uint16_t)sizeof(tx_buffer); index++)
  {
    tx_buffer[index] = 0U;
  }
  for (uint16_t index = 0U; index < (uint16_t)sizeof(page_buffer); index++)
  {
    page_buffer[index] = 0U;
  }

  /* Reinitialize OTA transfer state, retry counters, completion flags, and write address. */
  page_currently_written = 0U;
  tx_length = 0U;
  tx_pending = 0U;
  page_size = 0U;
  seq_num = 0U;
  write_address = APP_WITH_OTA_SERVICE_ADDRESS;
  retry_size = 0U;
  retry_start = 0U;
  retry_getdata = 0U;
  retry_datareq = 0U;
  bootloadingCompleted = 0U;
  bootloadingCompleted_end = 0U;
  dbg_conn_retry_since_ms = dbg_last_print_ms;

  /* Full context reset returns to connection state and drops negotiated size information. */
  if (keep_size == 0U)
  {
    app_size = 0U;
    seq_num_max = 0U;
    ota_state_machine_g = OTA_CONNECTION;
  }

  /* Only full reset paths flush IRQ-deferred events and radio scheduler state. */
  if (full_reset != 0U)
  {
    /* Reset deferred radio event flags and local radio scheduler indicators. */
    RxOkEventPending = FALSE;
    TxDoneEventPending = FALSE;
    RxErrorEventPending = FALSE;
    RxErrorIrqStatus = 0U;
    RxReceivedByteCount = 0U;
    RadioTxBusy = 0U;
    RadioRxArmed = 0U;
    dbg_rx_ok_count = 0U;
    dbg_tx_done_count = 0U;
    dbg_rx_err_count = 0U;
    dbg_rx_crc_err_count = 0U;
    dbg_rx_other_err_count = 0U;
    dbg_last_print_ms = HAL_GetTick();
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

    case OTA_SIZE:
      return OTA_SizeCallback(packet, length);

    case OTA_START:
      return OTA_StartCallback(packet, length);

    case OTA_GETDATA:
      return OTA_GetDataCallback(packet, length);

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
  }

  /* Configure TX mode, payload length, and data buffer settings. */
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  HAL_MRSubG_PktBasicSetPayloadLength(length);
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)RadioTxPacketBuffer);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(MAX_PACKET_LENGTH);

  /* Clear TX-done IRQ flag and start TX command. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
  __HAL_MRSUBG_STROBE_CMD(CMD_TX);

  /* Update scheduler flags: TX is busy and RX is no longer armed. */
  RadioTxBusy = 1U;
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

  /* If TX is in progress, defer any new operation. */
  if (RadioTxBusy != 0U)
  {
    return;
  }

  /* Switch from RX to TX in two steps to avoid command rejection races. */
  if ((tx_pending != 0U) && (RadioRxArmed != 0U))
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
    if (LL_MRSubG_GetRadioFSMState() != STATE_IDLE)
    {
       __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
    
       /* wait for Sabort process completed, then clear the related flag */
       while(LL_MRSubG_GetRadioFSMState() != STATE_IDLE)
       {
       }
    }
    
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
    RadioRxArmed = 0U;
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
