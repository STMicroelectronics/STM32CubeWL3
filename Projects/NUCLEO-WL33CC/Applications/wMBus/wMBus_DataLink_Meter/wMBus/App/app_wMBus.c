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
#define WMBUS_DEVICE_TYPE METER           /**< wM-Bus device type (see @ref wMBus_Phy_meter_modes_t) */
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static uint8_t *rxBuff;
static uint16_t rxBuffLength;

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

  wMBus_DataLink_C_field_t c_field = DL_SND_NR;
  wMBus_DataLink_CI_field_t ci_field = CI_EN13757_3_SHORT_TPL;
  static uint8_t acc = 0x00;
  uint8_t data[20] = {0x00, 0x00, 0x10, 0x05, 0xD8, 0x0A, 0xB5, 0x1C, 0xB4, 0xE3, 0xD8, 0x4D, 0xB8, 0x6D, 0x27, 0x45, 0x71, 0xC7, 0xEE, 0x04};
  data[0] = acc++;
  uint8_t length = 20;
  wMBus_DataLink_send_frame(c_field, ci_field, data, length, &rxBuff, &rxBuffLength);

  /* USER CODE BEGIN MX_wMBus_Process_2 */
  printf("wM-Bus transmission completed\r\n");

  /* Wakeup source configuration */
  HAL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PORTA, PWR_WAKEUP_PIN11, PWR_WUP_FALLEDG);
  uint32_t wakeupPin = HAL_PWR_GetClearWakeupSource(LL_PWR_WAKEUP_PORTA);

  if (wakeupPin & B2_PIN)
  {
    BSP_LED_On(LD2);
    printf("SND-IR!\r\n");
    c_field = DL_SND_IR;
    ci_field = CI_EN13757_3_SHORT_TPL;
    wMBus_DataLink_send_frame(c_field, ci_field, data, length, &rxBuff, &rxBuffLength);
    printf("wM-Bus transmission completed\r\n");
    BSP_LED_Off(LD2);
    HAL_Delay(1000);
  }

  if (new_Rx_frame == 1)
  {
    printf("Received data length: %d\r\n", rxBuffLength);
    printf("Received data: ");
    for (int i = 0; i < rxBuffLength; i++)
    {
      printf("0x%02X ", rxBuff[i]);
    }
    printf("\r\n");
  }
  new_Rx_frame = 0;
  HAL_Delay(5000);
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

  BSP_LED_Init(LD2);
  /* Init SW2 User Button */
  BSP_PB_Init(B2, BUTTON_MODE_GPIO);

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
  uint8_t a_field[6] = {0x21, 0x01, 0x08, 0x10, 0x01, 0x02};
  uint8_t m_field[2] = {0x85, 0x5D};
  wMBus_DataLink_init(m_field, a_field, WMBUS_DEVICE_TYPE, WMBUS_MODE, WMBUS_FORMAT, WMBUS_DIRECTION);
  /* USER CODE BEGIN wMBus_init_2 */
  printf("STM32WL3 wM-Bus DataLink Demo - Meter.\r\n");
  /* USER CODE END wMBus_init_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
