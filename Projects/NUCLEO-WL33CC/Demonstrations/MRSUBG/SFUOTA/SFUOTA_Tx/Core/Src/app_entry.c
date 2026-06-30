/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  GPAM Wireless Application Team
  * @brief   Entry point of the application
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#if (CFG_LPM_SUPPORTED == 1)
#include "stm32_lpm.h"
#include "stm32_lpm_if.h"
#endif /* CFG_LPM_SUPPORTED */

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
#define OTA_SWITCH_TO_OTA_RESET_MANAGER (0xB0U)
#define MAX_NUM_PACKET         (100U)            /* Number of packets used for the test */
#define TX_PAYLOAD_LEN         (7U)
#define MRSUBG_TX_CHANNEL_HZ   (868000000U)
#define TX_INTER_PACKET_DELAY_MS (100U)

#define MAX_PACKET_LENGTH      (255U)
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a, b)                        (((a) < (b)) ? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t TxPacketBuffer[MAX_PACKET_LENGTH];
static uint8_t TxSendPending;
static uint16_t TxPacketCount = 0;
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/

/* USER CODE BEGIN PFP */
void OTA_Jump_To_Reset_Manager(void);
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

uint32_t MX_APPE_Init(void *p_param)
{

  UNUSED(p_param);

  /* USER CODE BEGIN APPE_Init_1 */
  BSP_LED_Init(LD1);
  BSP_LED_Init(LD2);
  BSP_LED_Init(LD3);
  BSP_LED_On(LD1);

  COM_InitTypeDef COM_Init = {0};

  COM_Init.BaudRate = 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);

  /* Initialize the button B2 */
  BSP_PB_Init(B2, BUTTON_MODE_GPIO);

  /* Build packet */
  TxPacketBuffer[0] = 0x02;
  TxPacketBuffer[1] = 5;   /* Length position is fixed */
  TxPacketBuffer[2] = 0x01;
  TxPacketBuffer[3] = 0x02;
  TxPacketBuffer[4] = 0x03;
  TxPacketBuffer[5] = 0x04;
  TxPacketBuffer[6] = 0;

  __HAL_MRSUBG_SET_TX_MODE(TX_NORMAL);
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)TxPacketBuffer);
  __HAL_MRSUBG_SET_DATABUFFER_SIZE(MAX_PACKET_LENGTH);
  /* USER CODE END APPE_Init_1 */

#if (CFG_LPM_SUPPORTED == 1)
  /* Low Power Manager Init */
  UTIL_LPM_Init();
#endif /* CFG_LPM_SUPPORTED */

  /* USER CODE BEGIN APPE_Init_2 */
  TxSendPending = TRUE;
  printf("STM32WL3 SFUOTA Tx Demo.\r\n");
  printf("Transmitting packets on %u Hz\r\n", MRSUBG_TX_CHANNEL_HZ);
  /* USER CODE END APPE_Init_2 */

  return 0;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
#if (CFG_LPM_SUPPORTED == 1)
static PowerSaveLevels App_PowerSaveLevel_Check(void)
{
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;

  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */
  if (TxSendPending == TRUE)
  {
    output_level = POWER_SAVE_LEVEL_DISABLED;
  }

  /* USER CODE END App_PowerSaveLevel_Check_1 */

  return output_level;
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck(void)
{
  return POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;
}
#endif

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

void OTA_Jump_To_Reset_Manager(void)
{
  RAM_VR.bootloader_vr = OTA_SWITCH_TO_OTA_RESET_MANAGER;

  NVIC_SystemReset();
}

/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void MX_APPE_Process(void)
{
  /* USER CODE BEGIN MX_APPE_Process_1 */

  /* USER CODE END MX_APPE_Process_1 */

  /* USER CODE BEGIN MX_APPE_Process_2 */
  if (TxSendPending == TRUE)
  {
    TxSendPending = FALSE;
    HAL_MRSubG_PktBasicSetPayloadLength(TX_PAYLOAD_LEN);
    __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)TxPacketBuffer);
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);

    while ((__HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS() & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F) == 0);

    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);
    BSP_LED_Toggle(LD1);
    HAL_Delay(TX_INTER_PACKET_DELAY_MS);
    TxSendPending = TRUE;

    TxPacketBuffer[6] = (uint8_t)(++TxPacketCount);

    if (TxPacketCount == MAX_NUM_PACKET)
    {
      printf("Frequency = %u Hz, Packet count = %d\r\n", MRSUBG_TX_CHANNEL_HZ, TxPacketCount);
      TxPacketCount = 0;
      TxPacketBuffer[6] = 0;
    }
  }

  if (BSP_PB_GetState(B2) == SET)
  {
    OTA_Jump_To_Reset_Manager();
  }
  /* USER CODE END MX_APPE_Process_2 */
}

void MX_APPE_Idle(void)
{
  /* USER CODE BEGIN MX_APPE_Idle_1 */

  /* USER CODE END MX_APPE_Idle_1 */

#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_powerSave_level, vtimer_powerSave_level, final_level;

  app_powerSave_level = App_PowerSaveLevel_Check();

  if (app_powerSave_level != POWER_SAVE_LEVEL_DISABLED)
  {
    vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
    final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);

    switch (final_level)
    {
    case POWER_SAVE_LEVEL_DISABLED:
      /* Not Power Save device is busy */
      return;
      break;
    case POWER_SAVE_LEVEL_SLEEP:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_SLEEP_MODE);
      break;
    case POWER_SAVE_LEVEL_DEEPSTOP_TIMER:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_DEEPSTOP_LS_MODE);
      break;
    case POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER:
      UTIL_LPM_SetMaxMode(1 << CFG_LPM_APP, UTIL_LPM_DEEPSTOP_NOLS_MODE);
      break;
    }

    UTIL_LPM_Enter(0);
  }
#endif /* CFG_LPM_SUPPORTED */

  /* USER CODE BEGIN MX_APPE_Idle_2 */

  /* USER CODE END MX_APPE_Idle_2 */
}

