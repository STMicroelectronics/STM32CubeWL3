/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_wmbus.c
 * @author  MCD Application Team
 * @brief   Application of the WmBus Phy Middleware
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
#include "app_wmbus.h"

#include "stm32_lpm.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
static void SystemApp_Init(void);
static void wmbphy_Init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void MX_wmbphy_Init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format) {
    /* USER CODE BEGIN MX_wmbphy_Init_1 */

    /* USER CODE END MX_wmbphy_Init_1 */
    SystemApp_Init();
    /* USER CODE BEGIN MX_wmbphy_Init_2 */

    /* USER CODE END MX_wmbphy_Init_2 */
    wmbphy_Init(WmBus_mode, WmBus_Direction, WmBus_Format);
    /* USER CODE BEGIN MX_wmbphy_Init_3 */

    /* USER CODE END MX_wmbphy_Init_3 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

static void SystemApp_Init(void) {
    /* USER CODE BEGIN SystemApp_Init_1 */

    /* Low Power Manager Init */
    UTIL_LPM_Init();

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

static void wmbphy_Init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format) {
    /* USER CODE BEGIN wmbphy_Init_1 */

    /* USER CODE END wmbphy_Init_1 */
    wmbphy_init(WmBus_mode, WmBus_Direction, WmBus_Format);
    /* USER CODE BEGIN wmbphy_Init_2 */
    printf("STM32WL3 WmBus Phy Demo - Meter.\r\n");
    /* USER CODE END wmbphy_Init_2 */
}

void MX_wmbphy_Process() {
    /* USER CODE BEGIN MX_wmbphy_Process_1 */

    /* USER CODE END MX_wmbphy_Process_1 */

    /* USER CODE BEGIN MX_wmbphy_Process_2 */

    /* USER CODE END MX_wmbphy_Process_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
