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

#define MAX_WMBUS_PHY_PACKET 435 /* maximum buffer size: T-mode 3o6 frame format A */
/* USER CODE BEGIN PD */
#define CRC_POLYNOM 0x3D65
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static uint8_t wMBus_RxBuffer[MAX_WMBUS_PHY_PACKET];
static uint16_t wMBus_RxBuffer_length;
static int32_t wMBus_RssiDbm = 0; /* Rx level of Rx wM-Bus packet */
static uint32_t wMBus_RxSync;

/* USER CODE BEGIN PV */
static uint8_t wMBus_Valid_Packet_cnt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemApp_Init(void);
static void wMBus_init(void);
/* USER CODE BEGIN PFP */
#ifdef WFI_ENABLE
void Enter_WFI(void);
#endif
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
  uint8_t _wMBus_format = WMBUS_FORMAT;
  /* USER CODE BEGIN MX_wMBus_Process_1 */

  /* USER CODE END MX_wMBus_Process_1 */

  wMBus_Phy_prepare_Rx();
  /* Send the RX command */
  wMBus_Phy_start_continuousRx();

  /* register LL wM-Bus buffer & size into HAL for copy - and verification */
  /* indicate LL buffer size in wMBus_RxBuffer_length */
  wMBus_RxBuffer_length = MAX_WMBUS_PHY_PACKET;
  wMBus_Phy_register_LL_buffer(wMBus_RxBuffer, &wMBus_RxBuffer_length, &_wMBus_format, &wMBus_RxSync, &wMBus_RssiDbm);

  /* USER CODE BEGIN MX_wMBus_Process_2 */
#ifdef WFI_ENABLE
  Enter_WFI();
#endif
  /* USER CODE END MX_wMBus_Process_2 */

  /* wait for WMBUS_RX_COMPLETED_WITH_VALID_CRC event flag set */
  while (!wMBus_Phy_check_radio_events(WMBUS_RX_COMPLETED_WITH_VALID_CRC))
  {
    /* USER CODE BEGIN MX_wMBus_Process_3 */
    /* while no valid packet is received - we can check other event */
    if (wMBus_Phy_check_radio_events(WMBUS_RX_COMPLETED_WITH_CRC_ERROR) == 1)
    {
      printf("CRC error\r\n");
    }
    if (wMBus_Phy_check_radio_events(WMBUS_FIRST_CRC_ERROR) == 1)
    {
      printf("CRC error on 1st Block\r\n");
    }
    if (wMBus_Phy_check_radio_events(WMBUS_RX_VALID_HEADER_ERROR) == 1)
    {
      printf("Packet Header error\r\n");
    }
    if (wMBus_Phy_check_radio_events(WMBUS_RX_OVERFLOW_ERROR) == 1)
    {
      printf("Packet payload overflow error\r\n");
    }

#ifdef WFI_ENABLE
    Enter_WFI();
#endif
    /* USER CODE END MX_wMBus_Process_3 */
  }

  /* USER CODE BEGIN MX_wMBus_Process_4 */
  /* here it means valid packet has been received */
  printf("wMBus_RxBuffer_length: %d\r\n", wMBus_RxBuffer_length);

  printf("wM-Bus buffer - CRCs checked - received\r\n");
  printf("[ ");
  for (uint16_t i = 0; i < wMBus_RxBuffer_length; i++)
  {
    printf("%02X ", wMBus_RxBuffer[i]);
  }
  printf("]\r\n");

  printf("Valid Packet Received\r\n");

  printf("Rx RSSI level: %ld\r\n", wMBus_RssiDbm);

  wMBus_Valid_Packet_cnt++;

  printf("Valid packet count: %d\r\n", wMBus_Valid_Packet_cnt);
  printf("\r\n");
  /* USER CODE END MX_wMBus_Process_4 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

static void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */

  COM_InitTypeDef COM_Init = {0};

  COM_Init.BaudRate = 460800;
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
#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
  wMBus_Phy_set_active_power_mode(WMBUS_LPM);
#endif
  /* USER CODE BEGIN wMBus_init_2 */
  printf("STM32WL3 wM-Bus Phy Demo - Concentrator.\r\n");
  /* USER CODE END wMBus_init_2 */
}

/* USER CODE BEGIN PrFD */

#ifdef WFI_ENABLE
void Enter_WFI()
{
  UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
  UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

  UTIL_LPM_EnterLowPower();
}
#endif
/* USER CODE END PrFD */
