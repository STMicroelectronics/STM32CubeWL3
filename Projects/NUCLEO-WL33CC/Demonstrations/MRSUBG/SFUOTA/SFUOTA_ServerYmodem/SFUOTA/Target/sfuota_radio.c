/**
  ******************************************************************************
  * @file    sfuota_radio.c
  * @author  GPAM Wireless Application Team
  * @brief   MRSubG PHY adaptation layer for the WL33 SFUOTA applications.
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
#include "sfuota_radio.h"

/* Private define -----------------------------------------------------------*/
#define RFSEQ_IRQ_ERROR_MASK   (MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F     | \
                                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F   | \
                                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_AHB_ACCESS_ERROR_F | \
                                MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_COMMAND_REJECTED_F)  /*!< Radio IRQ error sources reported to the protocol layer. */

#define SFUOTA_TX_STUCK_TIMEOUT_MS (300U)  /*!< Watchdog timeout to recover from missed TX_DONE event. */
#define SFUOTA_ABORT_TIMEOUT_MS    (10U)   /*!< Watchdog timeout to recover from missed SABORT_DONE event. */

/* Private variables --------------------------------------------------------*/
/* Word alignment required: the MRSubG data buffer pointer ignores the two LSBs. */
static __attribute__((aligned(4))) uint8_t RadioRxPacketBuffer[SFUOTA_RADIO_MAX_PACKET];  /*!< Radio RX packet buffer. */
static __attribute__((aligned(4))) uint8_t RadioTxPacketBuffer[SFUOTA_RADIO_MAX_PACKET];  /*!< Radio TX packet buffer. */

static volatile uint8_t RxOkEventPending = FALSE;     /*!< RX OK event flag. */
static volatile uint8_t TxDoneEventPending = FALSE;   /*!< TX done event flag. */
static volatile uint8_t RxErrorEventPending = FALSE;  /*!< RX error event flag. */
static volatile uint32_t RxErrorIrqStatus = 0U;       /*!< RX error IRQ status. */
static uint8_t RadioTxBusy = 0U;                      /*!< Radio TX busy flag. */
static uint32_t RadioTxStartMs = 0U;                  /*!< Tick when current TX started, used to recover from missed TX_DONE. */
static uint8_t RadioRxArmed = 0U;                     /*!< Radio RX armed flag. */
static uint8_t RadioAbortInProgress = 0U;             /*!< RX abort in progress before switching to TX. */
static uint32_t RadioAbortStartMs = 0U;               /*!< Tick when the current abort was strobed. */

/* Private functions prototypes ----------------------------------------------*/
static void SFUOTA_Radio_StartReception(void);
static uint8_t SFUOTA_Radio_AbortPending(void);

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Enables the RFSEQ IRQ sources used by SFUOTA processing.
  * @note   Reception is not armed here: the caller arms it through SFUOTA_Radio_EnsureReceiving().
  * @param  None.
  * @retval None.
  */
void SFUOTA_Radio_Init(void)
{
  /* Enable the IRQ sources used by SFUOTA. */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_AHB_ACCESS_ERROR_E |
                                    MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_COMMAND_REJECTED_E);

  /* Clear pending IRQ flags before initialization. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F |
                                    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F |
                                    MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F |
                                    RFSEQ_IRQ_ERROR_MASK);
}

/**
  * @brief  Drops deferred radio events and clears the local scheduling state.
  * @param  None.
  * @retval None.
  */
void SFUOTA_Radio_Reset(void)
{
  RxOkEventPending = FALSE;
  TxDoneEventPending = FALSE;
  RxErrorEventPending = FALSE;
  RxErrorIrqStatus = 0U;
  RadioTxBusy = 0U;
  RadioTxStartMs = 0U;
  RadioRxArmed = 0U;

  /* Clearing the software flags alone would leave them out of step with a receiver
     still running, and the next command would come back as COMMAND_REJECTED. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
  RadioAbortInProgress = 1U;
  RadioAbortStartMs = HAL_GetTick();
}

/**
  * @brief  Latches SFUOTA-related MRSubG IRQ events for deferred processing.
  *
  * Keeps ISR work minimal by clearing hardware IRQ flags and storing software
  * event indicators consumed later in task context.
  *
  * @param  None.
  * @retval None.
  */
void SFUOTA_Radio_IRQHandler(void)
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
      SFUOTA_Radio_StartReception();
    }
  }

  /* Latch RX completion and clear the corresponding IRQ flag. */
  if ((irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F) != 0U)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F);
    RxOkEventPending = TRUE;
  }

  /* Latch RX errors. The raw status is decoded later, in GetEvents(). */
  if ((irq_status & RFSEQ_IRQ_ERROR_MASK) != 0U)
  {
    RxErrorIrqStatus = (irq_status & RFSEQ_IRQ_ERROR_MASK);
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(RxErrorIrqStatus);
    RxErrorEventPending = TRUE;
  }
}

/**
  * @brief  Collects and clears the radio events latched since the previous call.
  * @param  events Destination event structure.
  * @retval None.
  */
void SFUOTA_Radio_GetEvents(sfuota_radio_event_t *events)
{
  uint32_t now_ms = HAL_GetTick();

  if (events == NULL)
  {
    return;
  }

  events->rx_ok = FALSE;
  events->tx_done = FALSE;
  events->rx_error = FALSE;
  events->rx_error_flags = 0U;
  events->rx_data = NULL;
  events->rx_length = 0U;

  if (RxOkEventPending == TRUE)
  {
    RxOkEventPending = FALSE;
    RadioRxArmed = 0U;
    events->rx_ok = TRUE;
    events->rx_data = RadioRxPacketBuffer;
    events->rx_length = __HAL_MRSUBG_GET_DATABUFFER_COUNT();
  }

  if (TxDoneEventPending == TRUE)
  {
    TxDoneEventPending = FALSE;
    RadioTxBusy = 0U;
    RadioTxStartMs = 0U;
    events->tx_done = TRUE;
  }

  /* Recover from a missed TX_DONE IRQ that would otherwise keep TX busy forever. */
  if ((RadioTxBusy != 0U) && (RadioTxStartMs != 0U) && ((now_ms - RadioTxStartMs) >= SFUOTA_TX_STUCK_TIMEOUT_MS))
  {
    RadioTxBusy = 0U;
    RadioTxStartMs = 0U;
    if (RadioRxArmed == 0U)
    {
      SFUOTA_Radio_StartReception();
    }
  }

  if (RxErrorEventPending == TRUE)
  {
    RxErrorEventPending = FALSE;
    RadioRxArmed = 0U;
    events->rx_error = TRUE;

    /* Translate the radio status bits into hardware-independent error causes. */
    if ((RxErrorIrqStatus & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F) != 0U)
    {
      events->rx_error_flags |= SFUOTA_RADIO_ERR_CRC;
    }
    if ((RxErrorIrqStatus & (~MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F)) != 0U)
    {
      events->rx_error_flags |= SFUOTA_RADIO_ERR_OTHER;
    }

    RxErrorIrqStatus = 0U;
  }
}

/**
  * @brief  Checks radio availability and aborts reception when a transmission is requested.
  * @param  None.
  * @retval SFUOTA_RADIO_READY when @ref SFUOTA_Radio_Transmit can be called.
  * @retval SFUOTA_RADIO_BUSY when the caller must retry later.
  */
sfuota_radio_status_t SFUOTA_Radio_PrepareTx(void)
{
  /* If TX is in progress, defer any new operation. */
  if (RadioTxBusy != 0U)
  {
    return SFUOTA_RADIO_BUSY;
  }

  /* Wait for RX abort completion before issuing TX command. */
  if (SFUOTA_Radio_AbortPending() != 0U)
  {
    return SFUOTA_RADIO_BUSY;
  }

  /* Switch from RX to TX in two steps to avoid command rejection races. */
  if (RadioRxArmed != 0U)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
    __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
    RadioRxArmed = 0U;
    RadioAbortInProgress = 1U;
    RadioAbortStartMs = HAL_GetTick();
    return SFUOTA_RADIO_BUSY;
  }

  return SFUOTA_RADIO_READY;
}

/**
  * @brief  Configures and starts radio transmission of one SFUOTA packet.
  * @param  packet Pointer to packet payload to transmit.
  * @param  length Packet length in bytes.
  * @retval None.
  */
void SFUOTA_Radio_Transmit(const uint8_t *packet, uint8_t length)
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

  /* Point DATABUFFER0 at the TX buffer. */
  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  HAL_MRSubG_PktBasicSetPayloadLength(length);
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)RadioTxPacketBuffer);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(SFUOTA_RADIO_MAX_PACKET);

  /* Clear TX-done IRQ flag and start TX command. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
  __HAL_MRSUBG_STROBE_CMD(CMD_TX);

  /* Update scheduler flags: TX is busy and RX is no longer armed. */
  RadioTxBusy = 1U;
  RadioTxStartMs = HAL_GetTick();
  RadioRxArmed = 0U;
}

/**
  * @brief  Arms reception when no transmission or abort is pending.
  * @param  None.
  * @retval None.
  */
void SFUOTA_Radio_EnsureReceiving(void)
{
  if (RadioTxBusy != 0U)
  {
    return;
  }

  if (SFUOTA_Radio_AbortPending() != 0U)
  {
    return;
  }

  if (RadioRxArmed != 0U)
  {
    return;
  }

  SFUOTA_Radio_StartReception();
}

/**
  * @brief  Reports whether a transmission is still ongoing.
  * @param  None.
  * @retval Non-zero while the radio is transmitting.
  */
uint8_t SFUOTA_Radio_IsTxBusy(void)
{
  return RadioTxBusy;
}

/**
  * @brief  Reports whether reception is currently armed.
  * @param  None.
  * @retval Non-zero while the receiver is armed.
  */
uint8_t SFUOTA_Radio_IsRxArmed(void)
{
  return RadioRxArmed;
}

/* Private functions --------------------------------------------------------*/
/**
  * @brief  Reports whether a receive abort is still running, clearing it once completed.
  * @param  None.
  * @retval Non-zero while the abort has not completed yet.
  */
static uint8_t SFUOTA_Radio_AbortPending(void)
{
  uint32_t irq_status;

  if (RadioAbortInProgress == 0U)
  {
    return 0U;
  }

  irq_status = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();
  if ((irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F) == 0U)
  {
    /* An already idle radio never raises SABORT_DONE: give up after a while
       instead of blocking every further command. */
    if ((HAL_GetTick() - RadioAbortStartMs) < SFUOTA_ABORT_TIMEOUT_MS)
    {
      return 1U;
    }
  }
  else
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);
  }

  RadioAbortInProgress = 0U;
  return 0U;
}

/**
  * @brief  Configures and starts radio reception for SFUOTA packets.
  * @param  None.
  * @retval None.
  */
static void SFUOTA_Radio_StartReception(void)
{
  /* Point DATABUFFER0 back at the RX buffer: a TX may have moved it. */
  __HAL_MRSUBG_SET_RX_MODE(RX_NORMAL);
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)RadioRxPacketBuffer);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(SFUOTA_RADIO_MAX_PACKET);
  HAL_MRSubG_PktBasicSetPayloadLength(SFUOTA_RADIO_MAX_PACKET - 1U);

  /* Clear pending RX/error IRQ flags and start reception. */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F | RFSEQ_IRQ_ERROR_MASK);
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  /* Track local RX armed state for scheduler decisions. */
  RadioRxArmed = 1U;
}
