/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  GPM WBL Application Team
  * @brief   Entry point of the application
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

#include "main.h"
#include "app_entry.h"
#include "stm32_lpm.h"
#include "stm32_lpm_if.h"

#define MSG_SIZE 20U
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static __attribute__((aligned(4))) uint8_t payload_memory[MSG_SIZE];

#if (CFG_LPM_SUPPORTED == 1)
static PowerSaveLevels App_PowerSaveLevel_Check(void)
{
  return POWER_SAVE_LEVEL_SLEEP;
}
#endif

uint32_t MX_APPE_Init(void *p_param)
{
  COM_InitTypeDef com_init = {0};
  uint32_t irq_enable = MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E;

  UNUSED(p_param);

  BSP_LED_Init(LD2);

  com_init.BaudRate = 115200;
  com_init.HwFlowCtl = COM_HWCONTROL_NONE;
  com_init.WordLength = COM_WORDLENGTH_8B;
  com_init.Parity = COM_PARITY_NONE;
  com_init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &com_init);

  UTIL_LPM_Init();

  HAL_NVIC_SetPriority(MRSUBG_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MRSUBG_IRQn);
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(irq_enable);

  __HAL_MRSUBG_SET_RX_MODE(RX_NORMAL);
  HAL_MRSubG_PktBasicSetPayloadLength(MSG_SIZE);
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&payload_memory);
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  return 0;
}

void MX_APPE_Process(void)
{
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck(void)
{
  return POWER_SAVE_LEVEL_SLEEP;
}

void MX_APPE_Idle(void)
{
#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_power_save_level;
  PowerSaveLevels timer_power_save_level;
  PowerSaveLevels final_level;

  app_power_save_level = App_PowerSaveLevel_Check();
  timer_power_save_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
  final_level = (PowerSaveLevels)MIN(timer_power_save_level, app_power_save_level);

  if (final_level == POWER_SAVE_LEVEL_SLEEP)
  {
    UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_SLEEP_MODE);
    UTIL_LPM_Enter(0);
  }
#endif
}

void RX_IRQHandler(void)
{
  uint32_t irq;

  irq = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();
  if ((irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F) != 0U)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F);

    BSP_LED_On(LD2);

    printf("RX - Data received: [ ");
    for (uint8_t index = 0U; index < MSG_SIZE; index++)
    {
      printf("%d ", payload_memory[index]);
    }
    printf("]\r\n");

    BSP_LED_Off(LD2);
    __HAL_MRSUBG_STROBE_CMD(CMD_RX);
  }
}
