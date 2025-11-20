/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    binary_iqsampling_spi.c
  * @author  GPM WBL Application Team
  * @brief   This file provides firmware functions for raw I/Q sampling via SPI
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

#include "binary_iqsampling_spi.h"
#include "stm32wl3x_ll_spi.h"
#include "stm32wl3x_ll_gpio.h"
#include "stm32wl3x_ll_bus.h"
#include "stm32wl3x_ll_dma.h"
#include "stm32wl3x_ll_dmamux.h"

/*
 * SPI-Mode specific variables
 */
__IO uint8_t _spi_tx_complete = 1;

void BinaryIQSampling_SPI_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  BINARYSAMPLING_LL_SPI_Slave_EnableClock();
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA);  
  
  /* SCK */
  GPIO_InitStruct.Pin = BINARYSAMPLING_GPIO_PIN_SPI_SLAVE_SCK;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  GPIO_InitStruct.Alternate = BINARYSAMPLING_GPIO_AF_SPI_SLAVE_SCK;
  LL_GPIO_Init(BINARYSAMPLING_GPIO_PORT_SLAVE_SCK, &GPIO_InitStruct);
  
  /* MISO */
  GPIO_InitStruct.Pin = BINARYSAMPLING_GPIO_PIN_SPI_SLAVE_MISO;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  GPIO_InitStruct.Alternate = BINARYSAMPLING_GPIO_AF_SPI_SLAVE_MISO;
  LL_GPIO_Init(BINARYSAMPLING_GPIO_PORT_SLAVE_MISO, &GPIO_InitStruct);

  /* NSS */
  GPIO_InitStruct.Pin = BINARYSAMPLING_GPIO_PIN_SPI_SLAVE_NSS;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  GPIO_InitStruct.Alternate = BINARYSAMPLING_GPIO_AF_SPI_SLAVE_NSS;
  LL_GPIO_Init(BINARYSAMPLING_GPIO_PORT_SLAVE_NSS, &GPIO_InitStruct);

  /* SPI_SLAVE DMA Init */
  /* SPI_SLAVE_TX Init */
  /* Configure the DMA1_Channel3 functional parameters */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, BINARYSAMPLING_LL_DMAMUX_REQ_SPI_SLAVE_TX);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_HIGH);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);
  
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, BINARYSAMPLING_LL_DMAMUX_REQ_SPI_SLAVE_TX);

  /* Enable DMA interrupts complete/error */
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);

  /* Configure SPI_SLAVE DMA transfer interrupts */
  /* Enable DMA TX Interrupt */
  LL_SPI_EnableDMAReq_TX(BINARYSAMPLING_SPI_SLAVE);

  /* SPI_SLAVE parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_SLAVE;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_HARD_OUTPUT;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;

  LL_SPI_Init(BINARYSAMPLING_SPI_SLAVE, &SPI_InitStruct);
  LL_SPI_SetStandard(BINARYSAMPLING_SPI_SLAVE, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_EnableNSSPulseMgt(BINARYSAMPLING_SPI_SLAVE);

  /* DMA interrupt init */
  /* DMA_IRQn interrupt configuration */
  NVIC_SetPriority(DMA_IRQn, IRQ_HIGH_PRIORITY);
  NVIC_EnableIRQ(DMA_IRQn);
}

void BinaryIQSampling_SPI_Transmit(uint8_t* buffer, uint32_t length)
{
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, (uint32_t)buffer, LL_SPI_DMA_GetRegAddr(BINARYSAMPLING_SPI_SLAVE), LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, length);

  _spi_tx_complete = 0;

  LL_SPI_Enable(BINARYSAMPLING_SPI_SLAVE);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}

uint8_t BinaryIQSampling_SPI_TXIsComplete(void)
{
  return _spi_tx_complete && !LL_SPI_IsActiveFlag_BSY(BINARYSAMPLING_SPI_SLAVE);
}

void BinaryIQSampling_SPI_Finalize(void)
{
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
}

/**
 * @brief De-Initialize SPI hardware
 */
void BinaryIQSampling_SPI_DeInit(void)
{
  //LL_SPI_DeInit(BINARYSAMPLING_SPI_SLAVE);
  LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_3);
  LL_DMA_DisableIT_TE(DMA1, LL_DMA_CHANNEL_3);
  LL_SPI_DisableDMAReq_TX(BINARYSAMPLING_SPI_SLAVE);
  NVIC_DisableIRQ(DMA_IRQn);
}

/**
  * @brief  Function called in case of error detected in SPI IT Handler
  * @param  None
  * @retval None
  */
static void SPI_SLAVE_TransferError_Callback(void)
{
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
  while(1) {};
}

/**
* @brief DMA interrupt handler for SPI transfer
*/
void DMA_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC3(DMA1)) {
    LL_DMA_ClearFlag_GI3(DMA1);
    _spi_tx_complete = 1;
  } else if (LL_DMA_IsActiveFlag_TE3(DMA1)) {
    SPI_SLAVE_TransferError_Callback();
  }
}