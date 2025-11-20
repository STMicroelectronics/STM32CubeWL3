/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_wMBus.c
  * @author  MCD Application Team
  * @brief   Application of the wMBus Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "app_wMBus.h"

#include "stm32_lpm.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* External functions ---------------------------------------------------------*/
/* USER CODE BEGIN EF */

/* USER CODE END EF */
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define WMBUS_MODE      C_MODE            /**< wM-Bus mode (see @ref wMBus_Phy_submode_t) */
#define WMBUS_FORMAT    WMBUS_FORMAT_A    /**< wM-Bus format (see @ref wMBus_Phy_frame_format_t) */
#define WMBUS_DIRECTION METER_TO_OTHER    /**< wM-Bus direction (see @ref wMBus_Phy_direction_t) */
#define SND_NR_LENGTH 50
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static uint8_t wMBus_SND_NR[SND_NR_LENGTH] = { 0x31, 0x44, 0xB4, 0x4C, 0x12, 0x34, 0x45, 0x67, 0x50, 0x07,                                     /* 0x8D, 0xCA, */
                                               0x7B, 0x27, 0xD8, 0xC9, 0x6D, 0x0C, 0x32, 0xF1, 0x4E, 0x33, 0x16, 0xD0, 0xFF, 0x62, 0x98, 0x7D, /* 0x39, 0xA1, */
                                               0x85, 0xAB, 0xE1, 0x2B, 0x6D, 0x6F, 0x70, 0x1D, 0xA3, 0xD7, 0x20, 0x26, 0x34, 0x5C, 0xFB, 0x26, /* 0x5C, 0x1F, */
                                               0x8F, 0x71, 0x6A, 0x95, 0xB1, 0x04, 0x00, 0x00,                                                 /* 0xCF, 0xA5  */ };
/* USER CODE BEGIN PV */
#ifdef SENSE_RSSI
static int32_t rssi_dbm;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemApp_Init(void);
static void wMBus_init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void MX_wMBus_Init()
{
  /* USER CODE BEGIN MX_wMBus_Init_1 */

  /* USER CODE END MX_wMBus_Init_1 */
  SystemApp_Init();
  /* USER CODE BEGIN MX_wMBus_Init_2 */

  /* USER CODE END MX_wMBus_Init_2 */
  wMBus_init();
  /* USER CODE BEGIN MX_wMBus_Init_3 */

  /* USER CODE END MX_wMBus_Init_3 */
}

void MX_wMBus_Process()
{
  /* USER CODE BEGIN MX_wMBus_Process_1 */
#ifdef SENSE_RSSI
  rssi_dbm = wMBus_Phy_sense_rssi(1000);
  printf("Rx RSSI level: %ld dBm\r\n", rssi_dbm);
  HAL_Delay(1);
#endif
  /* USER CODE END MX_wMBus_Process_1 */

  wMBus_Phy_prepare_Tx_CRC_mngt(wMBus_SND_NR, SND_NR_LENGTH, WMBUS_FORMAT);

  /* Send the TX command */
  wMBus_Phy_start_transmission();

  /* wait for WMBUS_TX_COMPLETED event flag set */
  while (!wMBus_Phy_check_radio_events(WMBUS_TX_COMPLETED));

  /* USER CODE BEGIN MX_wMBus_Process_2 */
  printf("wM-Bus transmission completed\r\n");
  HAL_Delay(1000);
  /* USER CODE END MX_wMBus_Process_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

static void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */

  COM_InitTypeDef COM_Init = {0};

  COM_Init.BaudRate = 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  /* USER CODE END SystemApp_Init_1 */

  /* USER CODE BEGIN SystemApp_Init_2 */

  /* USER CODE END SystemApp_Init_2 */
}

static void wMBus_init()
{
  /* USER CODE BEGIN wMBus_init_1 */

  /* USER CODE END wMBus_init_1 */
  wMBus_Phy_init(WMBUS_MODE, WMBUS_DIRECTION, WMBUS_FORMAT);
  /* USER CODE BEGIN wMBus_init_2 */
  printf("STM32WL3 wM-Bus Phy Demo - Meter.\r\n");
  /* USER CODE END wMBus_init_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
