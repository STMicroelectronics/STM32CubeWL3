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
#define WMBUS_MODE        C_MODE          /**< wM-Bus mode (see @ref wMBus_Phy_submode_t) */
#define WMBUS_FORMAT      WMBUS_FORMAT_A  /**< wM-Bus format (see @ref wMBus_Phy_frame_format_t) */
#define WMBUS_DIRECTION   METER_TO_OTHER  /**< wM-Bus direction (see @ref wMBus_Phy_direction_t) */
#define WMBUS_DEVICE_TYPE OTHER           /**< wM-Bus device type (see @ref wMBus_Phy_meter_modes_t) */
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static wMBus_DataLink_status_t status;
#ifdef PROCESS_FRAME
static wMBus_DataLink_frame_t receivedFrame;
#else
static uint8_t *rxBuff;
static uint16_t rxBuffLength;
#endif

/* USER CODE BEGIN PV */

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

  /* USER CODE END MX_wMBus_Process_1 */
  UNUSED(status);

#ifdef PROCESS_FRAME
  status = wMBus_DataLink_receive_frame(&receivedFrame);
#else
  status = wMBus_DataLink_receive_frame(&rxBuff, &rxBuffLength);
#endif

  /* USER CODE BEGIN MX_wMBus_Process_2 */
#ifdef PROCESS_FRAME
  if (status == LINK_STATUS_SUCCESS)
  {
    printf("Received frame with valid CRC\n");
    printf("L-field: 0x%02X\n", receivedFrame.l_field);
    printf("C-field: 0x%02X\n", receivedFrame.c_field);
    printf("CI-field: 0x%02X\n", receivedFrame.ci_field);
    printf("M-field: 0x%02X %02X\n", receivedFrame.m_field[0], receivedFrame.m_field[1]);
    printf("A-field: ");
    for (int i = 0; i < 6; i++)
    {
      printf("0x%02X ", receivedFrame.a_field[i]);
    }
    printf("\nData-field size: %d\n", receivedFrame.data_field.size);
    printf("Data-field: ");
    for (int i = 0; i < receivedFrame.data_field.size; i++)
    {
      printf("0x%02X ", receivedFrame.data_field.payload[i]);
    }
    printf("\n");
  }
  else
  {
    printf("Failed to receive frame, status: %d\n", status);
  }
#else
  if (status == LINK_STATUS_SUCCESS)
  {
    printf("Received frame with valid CRC\n");
    printf("Received data length: %d\n", rxBuffLength);
    printf("Received data: ");
    for (int i = 0; i < rxBuffLength; i++)
    {
      printf("0x%02X ", rxBuff[i]);
    }
    printf("\n");
  }
  else
  {
    printf("Failed to receive frame, status: %d\n", status);
  }
#endif
  /* USER CODE END MX_wMBus_Process_2 */
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
  uint8_t a_field[6] = {0x22, 0x01, 0x08, 0x10, 0x01, 0x02};
  uint8_t m_field[2] = {0x85, 0x5D};
  wMBus_DataLink_init(m_field, a_field, WMBUS_DEVICE_TYPE, WMBUS_MODE, WMBUS_FORMAT, WMBUS_DIRECTION);
  wMBus_DataLink_startRx();
  /* USER CODE BEGIN wMBus_init_2 */
  printf("STM32WL3 wM-Bus DataLink Demo - Concentrator.\r\n");
  /* USER CODE END wMBus_init_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
