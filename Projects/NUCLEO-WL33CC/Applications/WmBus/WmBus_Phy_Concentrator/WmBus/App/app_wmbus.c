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
/* define of maximum buffer size: T-mode 3o6 frame format A */
#define MAX_WMBUS_PHY_PACKET 435
/* USER CODE BEGIN PD */
#define CRC_POLYNOM 0x3D65
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
uint8_t wMBus_mode = 0;
uint8_t wMBus_format = 0;
uint8_t wMBus_direction = 0;
uint8_t wMBus_RxBuffer[MAX_WMBUS_PHY_PACKET];
uint16_t wMBus_RxBuffer_length;
int32_t wMBus_RssiDbm = 0; /* Rx level of Rx wM-Bus packet */
uint32_t wMBus_RxSync;

/* USER CODE BEGIN PV */
uint8_t wMBus_Valid_Packet_cnt = 0;
uint8_t xCrcResult = TRUE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemApp_Init(void);
static void wMBus_init(void);
/* USER CODE BEGIN PFP */
#ifndef WMBUS_CRC_IN_HAL
uint16_t wMBus_Phy_CRC_calc(uint16_t crcReg, uint8_t crcData);
uint8_t wMBus_Phy_CRC_check(uint8_t *pStart, uint8_t *pStop);
#endif
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
  /* USER CODE BEGIN MX_wMBus_Process_1 */

  /* USER CODE END MX_wMBus_Process_1 */

  wMBus_Phy_prepare_Rx();
  /* Send the RX command */
  wMBus_Phy_start_continuousRx();
#ifndef WMBUS_NO_BLOCKING_HAL
#ifndef WMBUS_CRC_IN_HAL
  /* wait for Rx packet IRQ */
  wMBus_RxBuffer_length = wMBus_Phy_wait_Rx_completed(wMBus_RxBuffer, &wMBus_RssiDbm, &wMBus_RxSync);

  /* USER CODE BEGIN MX_wMBus_Process_2 */
  printf("wM-Bus RAW buffer (with CRCs) received\r\n");
  printf("[ ");
  for (uint16_t i = 0; i < wMBus_RxBuffer_length; i++)
  {
    printf("%x ", wMBus_RxBuffer[i]);
  }
  printf("]\r\n");

  /* CRC verification */
  xCrcResult = TRUE;
  xCrcResult = wMBus_Phy_CRC_check(&wMBus_RxBuffer[0], &wMBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2]);

  for (uint16_t i = 0; i < (wMBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) / WMBUS_BLOCK_SIZE_FORMAT_A; i++)
  {
    xCrcResult &= wMBus_Phy_CRC_check(&wMBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * i], &wMBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * (i + 1) - 2]);
  }

  if ((wMBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A != 0)
    xCrcResult &= wMBus_Phy_CRC_check(&wMBus_RxBuffer[wMBus_RxBuffer_length - ((wMBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A)], &wMBus_RxBuffer[wMBus_RxBuffer_length - 2]);

  if (xCrcResult)
  {
    wMBus_Valid_Packet_cnt++;
    printf("Valid Packet Received\r\n");
  }

  printf("Valid packet count: %d\r\n", wMBus_Valid_Packet_cnt);

  printf("Rx RSSI level: %d\r\n", wMBus_RssiDbm);
  /* USER CODE END MX_wMBus_Process_2 */
#else
  wMBus_RxBuffer_length = wMBus_Phy_wait_Rx_CRC_mngt(wMBus_RxBuffer, &wMBus_RssiDbm, wMBus_format, &wMBus_RxSync, &wMBus_RssiDbm);

  /* USER CODE BEGIN MX_wMBus_Process_3 */
  printf("wM-Bus RAW buffer (with CRCs) received\r\n");

  printf("wMBus_RxBuffer_length: %d\r\n", wMBus_RxBuffer_length);

  if (wMBus_RxBuffer_length != 0)
  {
    printf("wM-Bus buffer - CRCs checked - received\r\n");
    printf("[ ");
    for (uint16_t i = 0; i < wMBus_RxBuffer_length; i++)
    {
      printf("%x ", wMBus_RxBuffer[i]);
    }
    printf("]\r\n");

    printf("Valid Packet Received\r\n");

    wMBus_Valid_Packet_cnt++;

    printf("Valid packet count: %d\r\n", wMBus_Valid_Packet_cnt);

    printf("Rx RSSI level: %d\r\n", wMBus_RssiDbm);
  }
  /* USER CODE END MX_wMBus_Process_3 */
#endif
#else /* WMBUS_NO_BLOCKING_HAL */
  /* register LL wM-Bus buffer & size into HAL for copy - and verification */
  /* indicate LL buffer size in wMBus_RxBuffer_length */
  wMBus_RxBuffer_length = MAX_WMBUS_PHY_PACKET;
  wMBus_Phy_register_LL_buffer(wMBus_RxBuffer, &wMBus_RxBuffer_length, &wMBus_format, &wMBus_RxSync, &wMBus_RssiDbm);

#ifndef WMBUS_CRC_IN_HAL
  /* wait for WMBUS_RX_COMPLETED_WITH_RAW_BUFFER event flag set */
  while (!wMBus_Phy_check_radio_events(WMBUS_RX_COMPLETED_WITH_RAW_BUFFER));

  /* USER CODE BEGIN MX_wMBus_Process_4 */
  printf("wM-Bus RAW buffer (with CRCs) received\r\n");
  printf("[ ");
  for (uint16_t i = 0; i < wMBus_RxBuffer_length; i++)
  {
    printf("%x ", wMBus_RxBuffer[i]);
  }
  printf("]\r\n");

  /* CRC verification */
  xCrcResult = TRUE;
  xCrcResult = wMBus_Phy_CRC_check(&wMBus_RxBuffer[0], &wMBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2]);

  for (uint16_t i = 0; i < (wMBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) / WMBUS_BLOCK_SIZE_FORMAT_A; i++)
  {
    xCrcResult &= wMBus_Phy_CRC_check(&wMBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * i], &wMBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * (i + 1) - 2]);
  }

  if ((wMBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A != 0)
    xCrcResult &= wMBus_Phy_CRC_check(&wMBus_RxBuffer[wMBus_RxBuffer_length - ((wMBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A)], &wMBus_RxBuffer[wMBus_RxBuffer_length - 2]);

  if (xCrcResult)
  {
    wMBus_Valid_Packet_cnt++;
    printf("Valid Packet Received\r\n");

    printf("Rx RSSI level: %d\r\n", wMBus_RssiDbm);
  }

  printf("Valid packet count: %d\r\n", wMBus_Valid_Packet_cnt);
  /* USER CODE END MX_wMBus_Process_4 */
#else /* #ifndef WMBUS_CRC_IN_HAL */

  /* USER CODE BEGIN MX_wMBus_Process_5 */
#ifdef WFI_ENABLE
  Enter_WFI();
#endif
  /* USER CODE END MX_wMBus_Process_5 */

  /* wait for WMBUS_RX_COMPLETED_WITH_VALID_CRC event flag set */
  while (!wMBus_Phy_check_radio_events(WMBUS_RX_COMPLETED_WITH_VALID_CRC))
  {
    /* USER CODE BEGIN MX_wMBus_Process_6 */
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
    /* USER CODE END MX_wMBus_Process_6 */
  }

  /* USER CODE BEGIN MX_wMBus_Process_7 */
  /* here it means valid packet has been received */
  printf("wMBus_RxBuffer_length: %d\r\n", wMBus_RxBuffer_length);

  printf("wM-Bus buffer - CRCs checked - received\r\n");
  printf("[ ");
  for (uint16_t i = 0; i < wMBus_RxBuffer_length; i++)
  {
    printf("%x ", wMBus_RxBuffer[i]);
  }
  printf("]\r\n");

  printf("Valid Packet Received\r\n");

  printf("Rx RSSI level: %ld\r\n", wMBus_RssiDbm);

  wMBus_Valid_Packet_cnt++;

  printf("Valid packet count: %d\r\n", wMBus_Valid_Packet_cnt);
  /* USER CODE END MX_wMBus_Process_7 */
#endif
#endif
  /* USER CODE BEGIN MX_wMBus_Process_8 */
  printf("\r\n");
  /* USER CODE END MX_wMBus_Process_8 */
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
  wMBus_mode = C_MODE;
  wMBus_format = WMBUS_FORMAT_A;
  wMBus_direction = METER_TO_OTHER;
  wMBus_Phy_init(wMBus_mode, wMBus_direction, wMBus_format);
#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
  wMBus_Phy_set_active_power_mode(WMBUS_LPM);
#endif
  /* USER CODE BEGIN wMBus_init_2 */
  printf("STM32WL3 wM-Bus Phy Demo - Concentrator.\r\n");
  /* USER CODE END wMBus_init_2 */
}

/* USER CODE BEGIN PrFD */
#ifndef WMBUS_CRC_IN_HAL
/**
 * @brief  Calculates the 16-bit CRC.
 * @note   This function is defined only if WMBUS_CRC_IN_HAL is not defined
 * @param  crcReg Current or initial value of the CRC calculation
 * @param  crcData Data to perform the CRC-16 operation on.
 * @retval crcReg Updated CRC value
 */
uint16_t wMBus_Phy_CRC_calc(uint16_t crcReg, uint8_t crcData)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    /* If upper most bit is 1 */
    if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
      crcReg = (crcReg << 1) ^ CRC_POLYNOM;
    else
      crcReg = (crcReg << 1);

    crcData <<= 1;
  }

  return crcReg;
}

/**
 * @brief  Check CRC of the data between the specified start and stop pointers.
 * @note   This function is defined only if WMBUS_CRC_IN_HAL is not defined
 * @param  pStart Pointer to the start of the buffer.
 * @param  pStop Pointer to the end of the buffer.
 * @retval uint8_t CRC check result.
 */
uint8_t wMBus_Phy_CRC_check(uint8_t *pStart, uint8_t *pStop)
{
  uint16_t seed = 0x0000;

  while (pStart != pStop)
  {
    seed = wMBus_Phy_CRC_calc(seed, *pStart);
    pStart++;
  }
  seed = ~seed;
  if ((pStop[0] == (uint8_t)(seed >> 8)) && (pStop[1] == (uint8_t)(seed)))
  {
    return TRUE;
  }
  return FALSE;
}
#endif

#ifdef WFI_ENABLE
void Enter_WFI()
{
  UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
  UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

  UTIL_LPM_EnterLowPower();
}
#endif
/* USER CODE END PrFD */
