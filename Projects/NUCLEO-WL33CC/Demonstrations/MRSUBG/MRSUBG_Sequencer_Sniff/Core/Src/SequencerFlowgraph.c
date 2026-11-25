
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    SequencerFlowgraph.c
  * @author  GPM WBL Application Team
  * @brief   This code implements a MRSUBG sequencer Auto ACK scenario
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/** @} */
/** \cond DOXYGEN_SHOULD_SKIP_THIS
 */
#include <stddef.h>
#include <stdio.h>

#include "SequencerFlowgraph.h"

const char* getCurrentAction(void);
static MRSubG_Sequencer_GlobalConfiguration_t GlobalSequencerConfiguration;

/* Allocate databuffers */
__ALIGNED(4) volatile uint8_t databuf0[15] = {0x53, 0x54, 0x4d, 0x69, 0x63, 0x72, 0x6f, 0x65, 0x6c, 0x65, 0x63, 0x74, 0x72, 0x6f, 0x6e};
__ALIGNED(4) volatile uint8_t databuf1[15] = {0x53, 0x54, 0x4d, 0x69, 0x63, 0x72, 0x6f, 0x65, 0x6c, 0x65, 0x63, 0x74, 0x72, 0x6f, 0x6e};
static MRSubG_Sequencer_ActionConfiguration_t ReceiveFrame;
static MRSubG_Sequencer_ActionConfiguration_t WaitForMessage;

typedef struct {
  const char *name;
  uint32_t address;
} SeqActionTableEntry;

/* The table contiaining all SeqActions with their addresses and names, only used for debugging purposes */
SeqActionTableEntry SeqActionTable[] = {
  { .name = "ReceiveFrame", .address = (uint32_t)&ReceiveFrame },
  { .name = "WaitForMessage", .address = (uint32_t)&WaitForMessage },
};

const char* getCurrentAction(void)
{
  uint32_t actionAddress = (uint32_t)GlobalSequencerConfiguration.ActionConfigPtr;

  for (uint32_t i = 0; i < sizeof(SeqActionTable) / sizeof(SeqActionTable[0]); ++i) {
    if (SeqActionTable[i].address == actionAddress)
      return SeqActionTable[i].name;
  }

  return "(action not found)";
}

void SequencerConfig(void)
{
  /*
   * Control flow settings for ReceiveFrame
   * If a signal was previously detected (CS flag raised in WaitForMessage), restart RX timer from the beginning to give receiver more time to capture message.
   * If message is successfully received or CRC error occurs, the flowgraph terminates (unconnected NextAction1 output).
   * If CS was erroneous and actually no message is on the air, restart with WaitForMessage after a 100ms interval.
   */
  ReceiveFrame.NextAction1Ptr = &WaitForMessage;
  ReceiveFrame.NextAction1Ctrl = SEQ_FLAG_CLEAREVENTS;
  ReceiveFrame.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F;
  ReceiveFrame.NextAction1Interval = HAL_MRSubG_Sequencer_Milliseconds(12);

  ReceiveFrame.NextAction2Ptr = &WaitForMessage;
  ReceiveFrame.NextAction2Ctrl = SEQ_FLAG_SLEEPENABLE | SEQ_FLAG_CLEAREVENTS;
  ReceiveFrame.NextAction2Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F | MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F | MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_FAST_RX_TERM_F | MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F;
  ReceiveFrame.NextAction2Interval = HAL_MRSubG_Sequencer_Milliseconds(12);

  ReceiveFrame.ActionTimeout = 0;

  /*
   * Control flow settings for WaitForMessage
   * If some signal is received (CS flag raised), transition to ReceiveFrame state.
   * Otherwise, if no signal is received within the RX timer interval (12ms), restart listening after a 10ms delay.
   */
  WaitForMessage.NextAction1Ptr = &ReceiveFrame;
  WaitForMessage.NextAction1Ctrl = 0;
  WaitForMessage.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_CS_F;
  WaitForMessage.NextAction1Interval = HAL_MRSubG_Sequencer_Milliseconds(0);

  WaitForMessage.NextAction2Ptr = &WaitForMessage;
  WaitForMessage.NextAction2Ctrl = SEQ_FLAG_SLEEPENABLE | SEQ_FLAG_CLEAREVENTS;
  WaitForMessage.NextAction2Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_FAST_RX_TERM_F;
  WaitForMessage.NextAction2Interval = HAL_MRSubG_Sequencer_Milliseconds(10);

  WaitForMessage.ActionTimeout = 0;

  /* Basic dynamic configuration for ReceiveFrame */
  __HAL_MRSUBG_SET_PKT_LEN(15);

  HAL_MRSubG_SetFrequencyBase(868000000);
  HAL_MRSubG_SetDatarate(38400);
  HAL_MRSubG_SetFrequencyDev(20000);
  HAL_MRSubG_SetChannelBW(100000);


  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD0_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD0_CONFIG_BT_SEL, 0);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD0_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD0_CONFIG_MOD_TYPE, MOD_2FSK & 0x0f);

  LL_MRSubG_SetAlmostEmptyThresholdTx(10);
  LL_MRSubG_SetAlmostFullThresholdRx(100);

  __HAL_MRSUBG_SET_RX_TIMEOUT(HAL_MRSubG_Sequencer_Milliseconds(25));

  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_CS_TIMEOUT_MASK);
  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_PQI_TIMEOUT_MASK);
  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_SQI_TIMEOUT_MASK);
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_OR_NAND_SELECT);

  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_CS_TERM_EN);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_RX_TIMEOUT, 0);

  MR_SUBG_GLOB_DYNAMIC->RFSEQ_IRQ_ENABLE = MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_TIMEOUT_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_FAST_RX_TERM_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SEQ_E;

  /* Always perform calibration */
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->VCO_CAL_CONFIG, MR_SUBG_GLOB_DYNAMIC_VCO_CAL_CONFIG_VCO_CALIB_REQ);

  /* Store dynamic configuration to ReceiveFrame */
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&ReceiveFrame, CMD_RELOAD_RX_TIMER);

  /* Basic dynamic configuration for WaitForMessage */
  __HAL_MRSUBG_SET_PKT_LEN(8);

  HAL_MRSubG_SetFrequencyBase(868000000);
  HAL_MRSubG_SetDatarate(38400);
  HAL_MRSubG_SetFrequencyDev(20000);
  HAL_MRSubG_SetChannelBW(100000);


  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD0_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD0_CONFIG_BT_SEL, 0);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD0_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD0_CONFIG_MOD_TYPE, MOD_2FSK & 0x0f);

  LL_MRSubG_SetAlmostEmptyThresholdTx(10);
  LL_MRSubG_SetAlmostFullThresholdRx(100);

  __HAL_MRSUBG_SET_RX_TIMEOUT(0);

  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_CS_TIMEOUT_MASK);
  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_PQI_TIMEOUT_MASK);
  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_SQI_TIMEOUT_MASK);
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_OR_NAND_SELECT);

  SET_BIT(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_CS_TERM_EN);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_RX_TIMEOUT, 80);

  MR_SUBG_GLOB_DYNAMIC->RFSEQ_IRQ_ENABLE = MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_TIMEOUT_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_FAST_RX_TERM_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_HW_ANA_FAILURE_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SEQ_E;

  /* Always perform calibration */
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->VCO_CAL_CONFIG, MR_SUBG_GLOB_DYNAMIC_VCO_CAL_CONFIG_VCO_CALIB_REQ);

  /* Store dynamic configuration to WaitForMessage */
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&WaitForMessage, CMD_RX);

  /* Assign allocated buffers to databuffer pointer registers */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&databuf0[0]);
  __HAL_MRSUBG_SET_DATABUFFER1_POINTER((uint32_t)&databuf1[0]);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(sizeof(databuf0));

  /* Enable MRSUBG_IRQ interrupt and configure interrupt flags */
  HAL_NVIC_EnableIRQ(MRSUBG_IRQn);

  /* Set the SOC_WAKEUP_OFFSET */
  __HAL_MRSUBG_SET_WAKEUP_OFFSET(6);

  /* Setup design values for default registers */
  MODIFY_REG_FIELD(MR_SUBG_RADIO->AFC1_CONFIG, MR_SUBG_RADIO_AFC1_CONFIG_AFC_FAST_PERIOD, 0x00);

  MODIFY_REG_FIELD(MR_SUBG_RADIO->CLKREC_CTRL0, MR_SUBG_RADIO_CLKREC_CTRL0_PSTFLT_LEN, 0x01);
  MODIFY_REG_FIELD(MR_SUBG_RADIO->CLKREC_CTRL0, MR_SUBG_RADIO_CLKREC_CTRL0_CLKREC_P_GAIN_FAST, 0x03);
  MODIFY_REG_FIELD(MR_SUBG_RADIO->CLKREC_CTRL0, MR_SUBG_RADIO_CLKREC_CTRL0_CLKREC_I_GAIN_FAST, 0x08);

  MODIFY_REG_FIELD(MR_SUBG_RADIO->CLKREC_CTRL1, MR_SUBG_RADIO_CLKREC_CTRL1_CLKREC_ALGO_SEL, 0x00);
  MODIFY_REG_FIELD(MR_SUBG_RADIO->CLKREC_CTRL1, MR_SUBG_RADIO_CLKREC_CTRL1_CLKREC_P_GAIN_SLOW, 0x05);
  MODIFY_REG_FIELD(MR_SUBG_RADIO->CLKREC_CTRL1, MR_SUBG_RADIO_CLKREC_CTRL1_CLKREC_I_GAIN_SLOW, 0x0C);

  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->DSSS_CTRL, MR_SUBG_GLOB_STATIC_DSSS_CTRL_ACQ_HITS, 0x03);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->DSSS_CTRL, MR_SUBG_GLOB_STATIC_DSSS_CTRL_ACQ_WINDOW, 0x04);

  /* Enable calibration */
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->VCO_CAL_CONFIG, MR_SUBG_GLOB_DYNAMIC_VCO_CAL_CONFIG_VCO_CALIB_REQ);

  /* Avoid AGC glitches */
  WRITE_REG(MR_SUBG_RADIO->RF_FSM7_TIMEOUT, 0x0F);

  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  __HAL_MRSUBG_SET_RX_MODE(RX_NORMAL);
  HAL_MRSubG_SetPALeveldBm(7, 10, PA_DRV_TX_HP);
  HAL_MRSubG_SetRSSIThreshold(-80);

  #define PREAMBLE_LENGTH                 PREAMBLE_BYTE(4)
  #define POSTAMBLE_LENGTH                PREAMBLE_BYTE(0)
  #define SYNC_LENGTH                     SYNC_BYTE(4)
  #define SYNC_WORD                       0x88888888
  #define VARIABLE_LENGTH                 VARIABLE
  #define EXTENDED_LENGTH_FIELD           DISABLE
  #define PREAMBLE_SEQ                    PRE_SEQ_0101
  #define POSTAMBLE_SEQ                   POST_SEQ_0101
  #define CRC_MODE                        PKT_CRC_MODE_8BITS
  #define EN_ADDRESS                      DISABLE
  #define CODING                          CODING_NONE
  #define EN_WHITENING                    ENABLE
  #define LEN_WIDTH                       BYTE_LEN_1
  #define SYNC_PRESENT                    ENABLE

  #define PREAMBLE_BYTE(v)                (4*v)
  #define SYNC_BYTE(v)                    ((8*v)-1)

  MRSubG_PcktBasicFields_t xBasicInit={
    PREAMBLE_LENGTH,
    POSTAMBLE_LENGTH,
    SYNC_LENGTH,
    SYNC_WORD,
    VARIABLE_LENGTH,
    PREAMBLE_SEQ,
    POSTAMBLE_SEQ,
    CRC_MODE,
    CODING,
    EN_WHITENING,
    LEN_WIDTH,
    SYNC_PRESENT
  };

  HAL_MRSubG_PacketBasicInit(&xBasicInit);

  // MR_SUBG_GLOB_STATIC->CRC_INIT = 0;
  // MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->PCKT_CTRL, MR_SUBG_GLOB_STATIC_PCKT_CTRL_WHIT_INIT, 0x00);

  GlobalSequencerConfiguration.Flags = 0;
  GlobalSequencerConfiguration.ActionConfigPtr = &WaitForMessage;
  HAL_MRSubG_Sequencer_ApplyStaticConfig(&GlobalSequencerConfiguration);
  __HAL_MRSUBG_SEQ_SET_GLOBAL_CONFIG(&GlobalSequencerConfiguration);
  MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = 0xffffffff;
}

/*
 * The following flag-specific interrupt handlers are declared as weak symbols
 * so that they can be overwritten from the user application.
 *
 * The reported current SeqAction may not be the SeqAction that caused the interrupt:
 * If the interrupt flag also causes a state transition, reported state will be *after* the transition.
 */
__weak void MRSUBG_RX_CRC_ERROR_IRQHandler(void);
__weak void MRSUBG_RX_CRC_ERROR_IRQHandler(void) {
  printf("IRQ for RX_CRC_ERROR flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_HW_ANA_FAILURE_IRQHandler(void);
__weak void MRSUBG_HW_ANA_FAILURE_IRQHandler(void) {
  printf("IRQ for HW_ANA_FAILURE flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_RX_OK_IRQHandler(void);
__weak void MRSUBG_RX_OK_IRQHandler(void) {
  printf("IRQ for RX_OK flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_FAST_RX_TERM_IRQHandler(void);
__weak void MRSUBG_FAST_RX_TERM_IRQHandler(void) {
  printf("IRQ for FAST_RX_TERM flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_SEQ_IRQHandler(void);
__weak void MRSUBG_SEQ_IRQHandler(void) {
  printf("IRQ for SEQ flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_RX_TIMEOUT_IRQHandler(void);
__weak void MRSUBG_RX_TIMEOUT_IRQHandler(void) {
  printf("IRQ for RX_TIMEOUT flag triggered in state %s\r\n", getCurrentAction());
}

/**
 * @brief Interrupt handler for RFSEQ_IRQ_STATUS events.
 *
 * @param None.
 */
void HAL_MRSubG_IRQ_Callback(void);
void HAL_MRSubG_IRQ_Callback(void) {
  uint32_t irq_status = MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS;
  uint32_t status_detail = MR_SUBG_GLOB_STATUS->RFSEQ_STATUS_DETAIL;
  MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = 0xffffffff;

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F)
    MRSUBG_RX_CRC_ERROR_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F)
    MRSUBG_HW_ANA_FAILURE_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F)
    MRSUBG_RX_OK_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_FAST_RX_TERM_F)
    MRSUBG_FAST_RX_TERM_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F)
    MRSUBG_RX_TIMEOUT_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SEQ_F) {
    MRSUBG_SEQ_IRQHandler();

    printf("The sequencer has terminated ");
    if (status_detail & MR_SUBG_GLOB_STATUS_RFSEQ_STATUS_DETAIL_SEQ_COMPLETE_F)
      printf("normally due to NextActionPtr = NULL.");
    else if (status_detail & MR_SUBG_GLOB_STATUS_RFSEQ_STATUS_DETAIL_SEQ_ACTIONTIMEOUT_F)
      printf("due to an ActionTimeout.");
    else
      printf("for unknown reasons.");

    printf("\r\n");

    if (status_detail & MR_SUBG_GLOB_STATUS_RFSEQ_STATUS_DETAIL_SEQ_ACTIONTIMEOUT_F) {
      printf("Re-triggering sequencer!\r\n");
      __HAL_MRSUBG_SEQ_TRIGGER();
    }
  }
}