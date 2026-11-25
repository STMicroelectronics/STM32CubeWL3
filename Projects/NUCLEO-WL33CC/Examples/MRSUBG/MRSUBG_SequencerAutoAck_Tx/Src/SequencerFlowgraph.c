
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
volatile uint8_t databuf0[18] = {0x53, 0x54, 0x4d, 0x69, 0x63, 0x72, 0x6f, 0x65, 0x6c, 0x65, 0x63, 0x74, 0x72, 0x6f, 0x6e, 0x69, 0x63, 0x73};
volatile uint8_t databuf1[18] = {0x53, 0x54, 0x4d, 0x69, 0x63, 0x72, 0x6f, 0x65, 0x6c, 0x65, 0x63, 0x74, 0x72, 0x6f, 0x6e, 0x69, 0x63, 0x73};

static MRSubG_Sequencer_ActionConfiguration_t WaitForACK;
static MRSubG_Sequencer_ActionConfiguration_t TransmitMessage;

typedef struct {
  const char *name;
  uint32_t address;
} SeqActionTableEntry;

/* The table contiaining all SeqActions with their addresses and names, only used for debugging purposes */
SeqActionTableEntry SeqActionTable[] = {
  { .name = "WaitForACK", .address = (uint32_t)&WaitForACK },
  { .name = "TransmitMessage", .address = (uint32_t)&TransmitMessage },
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
   * Control flow settings for WaitForACK
   * If an ACK message is received, transmit next data message after 100ms.
   * Otherwise, RX_TIMEOUT occurs after 4s, restart with TransmitMessage in that case.
   */
  WaitForACK.NextAction1Ptr = &TransmitMessage;
  WaitForACK.NextAction1Ctrl = SEQ_FLAG_CLEAREVENTS;
  WaitForACK.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F;
  WaitForACK.NextAction1Interval = HAL_MRSubG_Sequencer_Milliseconds(100);

  WaitForACK.NextAction2Ptr = &TransmitMessage;
  WaitForACK.NextAction2Ctrl = SEQ_FLAG_CLEAREVENTS;
  WaitForACK.NextAction2Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F | MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F;
  WaitForACK.NextAction2Interval = HAL_MRSubG_Sequencer_Microseconds(0);

  WaitForACK.ActionTimeout = 0;

  /*
   * Control flow settings for TransmitMessage
   * Transmit message with databuffer contents and, when done, transition to WaitForACK state.
   */
  TransmitMessage.NextAction1Ptr = &WaitForACK;
  TransmitMessage.NextAction1Ctrl = SEQ_FLAG_CLEAREVENTS;
  TransmitMessage.NextAction1Mask = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F;
  TransmitMessage.NextAction1Interval = HAL_MRSubG_Sequencer_Microseconds(0);

  TransmitMessage.NextAction2Ptr = NULL;
  TransmitMessage.NextAction2Ctrl = 0;
  TransmitMessage.NextAction2Mask = SEQ_MASK_NEVER_MATCH;
  TransmitMessage.NextAction2Interval = HAL_MRSubG_Sequencer_Microseconds(0);

  TransmitMessage.ActionTimeout = 0;

  /* Basic dynamic configuration for WaitForACK */
  __HAL_MRSUBG_SET_PKT_LEN(1);

  HAL_MRSubG_SetFrequencyBase(868000000);
  HAL_MRSubG_SetDatarate(38400);
  HAL_MRSubG_SetFrequencyDev(20000);
  HAL_MRSubG_SetChannelBW(100000);


  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD0_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD0_CONFIG_BT_SEL, 0);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD0_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD0_CONFIG_MOD_TYPE, MOD_2FSK & 0x0f);

  LL_MRSubG_SetAlmostEmptyThresholdTx(10);
  LL_MRSubG_SetAlmostFullThresholdRx(100);

  __HAL_MRSUBG_SET_RX_TIMEOUT(HAL_MRSubG_Sequencer_Seconds(4));

  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_CS_TIMEOUT_MASK);
  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_PQI_TIMEOUT_MASK);
  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_SQI_TIMEOUT_MASK);
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->RX_TIMER, MR_SUBG_GLOB_DYNAMIC_RX_TIMER_RX_OR_NAND_SELECT);

  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_CS_TERM_EN);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_RX_TIMEOUT, 0);

  MR_SUBG_GLOB_DYNAMIC->RFSEQ_IRQ_ENABLE = MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_TIMEOUT_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_CRC_ERROR_E;

  /* Always perform calibration */
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->VCO_CAL_CONFIG, MR_SUBG_GLOB_DYNAMIC_VCO_CAL_CONFIG_VCO_CALIB_REQ);

  /* Store dynamic configuration to WaitForACK */
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&WaitForACK, CMD_RX);

  /* Basic dynamic configuration for TransmitMessage */
  __HAL_MRSUBG_SET_PKT_LEN(1);

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

  CLEAR_BIT(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_CS_TERM_EN);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->FAST_RX_TIMER, MR_SUBG_GLOB_DYNAMIC_FAST_RX_TIMER_FAST_RX_TIMEOUT, 0);

  MR_SUBG_GLOB_DYNAMIC->RFSEQ_IRQ_ENABLE = MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E;

  /* Always perform calibration */
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->VCO_CAL_CONFIG, MR_SUBG_GLOB_DYNAMIC_VCO_CAL_CONFIG_VCO_CALIB_REQ);

  /* Store dynamic configuration to TransmitMessage */
  HAL_MRSubG_Sequencer_ApplyDynamicConfig(&TransmitMessage, CMD_TX);

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
  HAL_MRSubG_SetRSSIThreshold(-139);

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

//  MR_SUBG_GLOB_STATIC->CRC_INIT = 0;
//  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->PCKT_CTRL, MR_SUBG_GLOB_STATIC_PCKT_CTRL_WHIT_INIT, 0x00);

  GlobalSequencerConfiguration.Flags = 0;
  GlobalSequencerConfiguration.ActionConfigPtr = &TransmitMessage;
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

__weak void MRSUBG_RX_TIMEOUT_IRQHandler(void);
__weak void MRSUBG_RX_TIMEOUT_IRQHandler(void) {
  printf("IRQ for RX_TIMEOUT flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_TX_DONE_IRQHandler(void);
__weak void MRSUBG_TX_DONE_IRQHandler(void) {
  printf("IRQ for TX_DONE flag triggered in state %s\r\n", getCurrentAction());
}

__weak void MRSUBG_RX_OK_IRQHandler(void);
__weak void MRSUBG_RX_OK_IRQHandler(void) {
  printf("IRQ for RX_OK flag triggered in state %s\r\n", getCurrentAction());
}

/**
 * @brief Interrupt handler for RFSEQ_IRQ_STATUS events.
 *
 * @param None.
 */
void HAL_MRSubG_IRQ_Callback(void);
void HAL_MRSubG_IRQ_Callback(void) {
  uint32_t irq_status = MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS;
  MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = 0xffffffff;

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F)
    MRSUBG_RX_CRC_ERROR_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F)
    MRSUBG_RX_TIMEOUT_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F)
    MRSUBG_TX_DONE_IRQHandler();

  if (irq_status & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F)
    MRSUBG_RX_OK_IRQHandler();
}