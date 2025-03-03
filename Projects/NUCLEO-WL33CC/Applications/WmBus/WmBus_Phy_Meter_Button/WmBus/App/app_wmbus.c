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
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
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

void MX_wmbphy_Process(void) {
    /* USER CODE BEGIN MX_wmbphy_Process_1 */

    /* USER CODE END MX_wmbphy_Process_1 */

#ifdef DEEPSTOP_ENABLE
    /* Wakeup source configuration */
    HAL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PORTA, PWR_WAKEUP_PIN11, PWR_WUP_FALLEDG);

    uint32_t wakeupPin = HAL_PWR_GetClearWakeupSource(LL_PWR_WAKEUP_PORTA);

    if (wakeupPin & B2_PIN) {
        /* USER CODE BEGIN MX_wmbphy_Process_2 */
#ifdef PRINT_DEBUG
        BSP_LED_On(LD2);
        printf("Wakeup on Button B2!\r\n");
        BSP_LED_Off(LD2);
#endif
        /* USER CODE END MX_wmbphy_Process_2 */
    }
#endif

    /* USER CODE BEGIN MX_wmbphy_Process_3 */

    /* USER CODE END MX_wmbphy_Process_3 */
}

#if (CFG_LPM_SUPPORTED == 1)
static PowerSaveLevels App_PowerSaveLevel_Check(void) {
    PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;

    /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */

    /* USER CODE END App_PowerSaveLevel_Check_1 */

    return output_level;
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck() {
    return POWER_SAVE_LEVEL_DEEPSTOP_TIMER;
}
#endif

void MX_wmbphy_Idle() {
    /* USER CODE BEGIN MX_wmbphy_Idle_1 */
#if (CFG_LPM_SUPPORTED == 1)
    PowerSaveLevels app_powerSave_level, vtimer_powerSave_level, final_level;

    app_powerSave_level = App_PowerSaveLevel_Check();

    if (app_powerSave_level != POWER_SAVE_LEVEL_DISABLED) {
        vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
        final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);

        switch (final_level) {
            case POWER_SAVE_LEVEL_DISABLED:
                /* Not Power Save device is busy */
                return;
                break;
            case POWER_SAVE_LEVEL_SLEEP:
                UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
                UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
                break;
            case POWER_SAVE_LEVEL_DEEPSTOP_TIMER:
                UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_ENABLE);
                UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
                break;
            case POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER:
                UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_ENABLE);
                UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_ENABLE);
                break;
        }

        UTIL_LPM_EnterLowPower();
    }
#endif /* CFG_LPM_SUPPORTED */

    /* USER CODE END MX_wmbphy_Idle_1 */

    /* USER CODE BEGIN MX_wmbphy_Idle_2 */

    /* USER CODE END MX_wmbphy_Idle_2 */
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

static void wmbphy_Init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format) {
    /* USER CODE BEGIN wmbphy_Init_1 */

    /* USER CODE END wmbphy_Init_1 */
    wmbphy_init(WmBus_mode, WmBus_Direction, WmBus_Format);
    /* USER CODE BEGIN wmbphy_Init_2 */
    printf("STM32WL3 WmBus Phy Demo - Meter Button.\r\n");
    printf("Wakeup pushing Button 2\r\n");
    /* USER CODE END wmbphy_Init_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
