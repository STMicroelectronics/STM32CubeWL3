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
#ifdef NO_DEEPSTOP
#define WAKEUP_NO_DEEPSTOP_TIMEOUT 500  // 500 us
#else
#define WAKEUP_TIMEOUT 10000  // 10s
#ifdef MULTIPLE_RADIO_TIMER
#define WAKEUP_TIMEOUT2 6000  // 6s
#define WAKEUP_TIMEOUT3 2000  // 2s
#endif
#endif

#define DEBUG_GPIO_LINE

#ifdef SPECIAL_CODE
#define INTERPOLATED_FREQ (16 * 32768)
#define TIMEOUT_500us 500
#define TIMEOUT_167ms 167
#define TIMEOUT_10s 10
#define TIMEOUT_NO_DEEPSTOP (TIMEOUT_500us * INTERPOLATED_FREQ) / 1000000
#define TIMEOUT_WITH_DEEPSTOP (uint32_t)(TIMEOUT_10s * INTERPOLATED_FREQ)
#define TIMER_MAX_VALUE 0xFFFFFFFF
#define LOW_POWER_THR_STU 60
#define LOW_POWER_THR (76 + 786)  // 786 is HS startup time (1500 us) expressed in interpolated time
#endif
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
VTIMER_HandleType timerHandle;
#ifdef MULTIPLE_RADIO_TIMER
VTIMER_HandleType timerHandle2;
VTIMER_HandleType timerHandle3;
#endif
#ifdef RADIO_TIMER_ENABLED
uint8_t WmBus_Transmission_VTimer_Callback = 0;
#ifdef MULTIPLE_RADIO_TIMER
uint8_t VTimer_Callback2_flag = 0;
uint8_t VTimer_Callback3_flag = 0;
#endif
#endif
/* USER CODE BEGIN PV */

/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
static void SystemApp_Init(void);
static void wmbphy_Init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format);
#ifdef SPECIAL_CODE
void HAL_MRSUBG_TIMER_StartConfigureTimer(uint32_t time);
#endif
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

#ifdef SPECIAL_CODE
void HAL_MRSUBG_TIMER_StartConfigureTimer(uint32_t time) {
    uint32_t ABSOLUTE_TIME = LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC);

    if (time == TIMEOUT_500us) {
        LL_MRSUBG_TIMER_SetCPUWakeupTime(MR_SUBG_GLOB_RETAINED, ((ABSOLUTE_TIME + TIMEOUT_NO_DEEPSTOP) & TIMER_MAX_VALUE));
    } else {
        LL_MRSUBG_TIMER_SetCPUWakeupTime(MR_SUBG_GLOB_RETAINED, ((ABSOLUTE_TIME + TIMEOUT_WITH_DEEPSTOP) & TIMER_MAX_VALUE));
    }

    LL_MRSUBG_TIMER_EnableCPUWakeupTimer(MR_SUBG_GLOB_RETAINED);
}
#endif

void MX_wmbphy_RadioTimerConfig(void) {
    uint8_t ret_val = SUCCESS;

#ifdef DEBUG_GPIO_LINE
    GPIOB->BSRR = 1 << 4;
#endif

#ifdef SPECIAL_CODE
#ifdef NO_DEEPSTOP
    HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_500us);
#else
    HAL_MRSUBG_TIMER_StartConfigureTimer(TIMEOUT_10s);

#endif
#else
#ifdef NO_DEEPSTOP
    /* Start the VTimer 500us */
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(&timerHandle, WAKEUP_NO_DEEPSTOP_TIMEOUT);
#else
    /* Start the radio timer with WAKEUP_TIMEOUT */
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle, WAKEUP_TIMEOUT);
#ifdef MULTIPLE_RADIO_TIMER
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle2, WAKEUP_TIMEOUT2);
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(&timerHandle3, WAKEUP_TIMEOUT3);
#endif
#endif
#endif

    if (ret_val != SUCCESS) {
        /* USER CODE BEGIN MX_wmbphy_RadioTimerConfig_1 */
        printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
        /* USER CODE END MX_wmbphy_RadioTimerConfig_1 */
        while (1);
    }
}

void TimeoutCallback(void *timerHandle) {
    uint8_t ret_val;

    /* Add app code to execute @ Stop timeout */
#ifdef DEBUG_GPIO_LINE
    GPIOB->BRR = 1 << 4;
#endif

#ifdef NO_DEEPSTOP
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimer(timerHandle, WAKEUP_NO_DEEPSTOP_TIMEOUT);
#else
    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT);
#endif

#ifdef DEBUG_GPIO_LINE
    GPIOB->BSRR = 1 << 4;
#endif
    if (ret_val != SUCCESS) {
        /* USER CODE BEGIN TimeoutCallback_1 */
        printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
        /* USER CODE END TimeoutCallback_1 */
        while (1);
    }
#ifdef RADIO_TIMER_ENABLED
    /* set WmBus transmission flag */
    WmBus_Transmission_VTimer_Callback = 1;
#endif

    /* USER CODE BEGIN TimeoutCallback_2 */
#ifdef PRINT_DEBUG
    printf("Virtual Timer Timeout\r\n");
#endif
    /* USER CODE END TimeoutCallback_2 */
}

#ifdef MULTIPLE_RADIO_TIMER
void TimeoutCallback2(void *timerHandle) {
    uint8_t ret_val;

    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT2);

    if (ret_val != SUCCESS) {
        /* USER CODE BEGIN TimeoutCallback2_1 */
        printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
        /* USER CODE END TimeoutCallback2_1 */
        while (1);
    }
#ifdef RADIO_TIMER_ENABLED
    /* set Radio Timer application flag */
    VTimer_Callback2_flag = 1;
#endif

    /* USER CODE BEGIN TimeoutCallback2_2 */
#ifdef PRINT_DEBUG
    printf("Virtual Timer2 Timeout\r\n");
#endif
    /* USER CODE END TimeoutCallback2_2 */
}

void TimeoutCallback3(void *timerHandle) {
    uint8_t ret_val;

    ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT3);

    if (ret_val != SUCCESS) {
        /* USER CODE BEGIN TimeoutCallback3_1 */
        printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
        /* USER CODE END TimeoutCallback3_1 */
        while (1);
    }
#ifdef RADIO_TIMER_ENABLED
    /* set Radio Timer application flag */
    VTimer_Callback3_flag = 1;
#endif

    /* USER CODE BEGIN TimeoutCallback3_2 */
#ifdef PRINT_DEBUG
    printf("Virtual Timer3 Timeout\r\n");
#endif
    /* USER CODE END TimeoutCallback3_2 */
}

#endif

void MX_wmbphy_Process(void) {
#ifndef SPECIAL_CODE
    HAL_MRSUBG_TIMER_Tick();
#endif
    /* USER CODE BEGIN MX_wmbphy_Process_1 */

    /* USER CODE END MX_wmbphy_Process_1 */

#ifdef DEEPSTOP_ENABLE
    /* Wakeup source configuration */
    uint32_t wakeupInternal = HAL_PWREx_GetClearInternalWakeUpLine();

    if (wakeupInternal & PWR_WAKEUP_SUBGHOST) {
        /* USER CODE BEGIN MX_wmbphy_Process_2 */
#ifdef PRINT_DEBUG
        BSP_LED_On(LD2);
        printf("Wakeup on Radio Timer!\r\n");
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
#ifdef NO_DEEPSTOP
    PowerSaveLevels output_level = POWER_SAVE_LEVEL_DISABLED;
#else
    PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;
#ifdef SPECIAL_CODE
    if (LL_MRSUBG_TIMER_GetCPUWakeupTime(MR_SUBG_GLOB_RETAINED) < (LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC) + LOW_POWER_THR)) {
        output_level = POWER_SAVE_LEVEL_SLEEP;
    }
#endif
#endif

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
    PowerSaveLevels app_powerSave_level, final_level;
#ifndef SPECIAL_CODE
    PowerSaveLevels vtimer_powerSave_level;
#endif

    app_powerSave_level = App_PowerSaveLevel_Check();

    if (app_powerSave_level != POWER_SAVE_LEVEL_DISABLED) {
#ifdef SPECIAL_CODE
        final_level = app_powerSave_level;
#else
        vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
        final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);
#endif

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
    printf("STM32WL3 WmBus Phy Demo - Meter Radio Timer.\n\r");
    printf("Wakeup every xxx seconds\n\r");
    /* USER CODE END wmbphy_Init_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
