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
#ifdef SINGLE_TIMER_OPT
void HAL_MRSUBG_TIMER_StartConfigureTimer(uint32_t time);
#endif
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#ifdef NO_DEEPSTOP
#define WAKEUP_NO_DEEPSTOP_TIMEOUT 500 // 500 us
#else
#define WAKEUP_TIMEOUT 10000 // 10s
#ifdef MULTIPLE_RADIO_TIMER
#define WAKEUP_TIMEOUT2 6000 // 6s
#define WAKEUP_TIMEOUT3 2000 // 2s
#endif
#endif

#define DEBUG_GPIO_LINE

#ifdef SINGLE_TIMER_OPT
#define INTERPOLATED_FREQ (16 * 32768)
#define TIMEOUT_500us 500
#define TIMEOUT_167ms 167
#define TIMEOUT_10s 10
#define TIMEOUT_NO_DEEPSTOP (TIMEOUT_500us * INTERPOLATED_FREQ) / 1000000
#define TIMEOUT_WITH_DEEPSTOP (uint32_t)(TIMEOUT_10s * INTERPOLATED_FREQ)
#define TIMER_MAX_VALUE 0xFFFFFFFF
#define LOW_POWER_THR_STU 60
#define LOW_POWER_THR (76 + 786) // 786 is HS startup time (1500 us) expressed in interpolated time
#endif

#ifndef WMBUS_CRC_IN_HAL
#define SND_IR_LENGTH 19
#define SND_NR_LENGTH 58
#else
#define SND_IR_LENGTH 15
#define SND_NR_LENGTH 50
#endif
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
VTIMER_HandleType timerHandle;
uint8_t wMBus_Transmission_VTimer_Callback = 0;
#ifdef MULTIPLE_RADIO_TIMER
VTIMER_HandleType timerHandle2;
VTIMER_HandleType timerHandle3;
uint8_t VTimer_Callback2_flag = 0;
uint8_t VTimer_Callback3_flag = 0;
#endif

uint8_t wMBus_mode = 0;
uint8_t wMBus_format = 0;
uint8_t wMBus_direction = 0;

#ifndef WMBUS_CRC_IN_HAL
uint8_t wMBus_SND_IR[SND_IR_LENGTH] = { 0x0E, 0x46, 0x8D, 0x4E, 0x11, 0x45, 0x63, 0x1F, 0x10, 0x04, 0xE9, 0x1F, 0x28, 0x33, 0xE0, 0x88, 0x91, 0x67, 0xB5 };
uint8_t wMBus_SND_NR[SND_NR_LENGTH] = { 0x31, 0x44, 0xB4, 0x4C, 0x12, 0x34, 0x45, 0x67, 0x50, 0x07, 0x8D, 0xCA, 0x7B, 0x27, 0xD8, 0xC9, 0x6D, 0x0C, 0x32, 0xF1, 0x4E, 0x33, 0x16, 0xD0, 0xFF, 0x62, 0x98, 0x7D, 0x39, 0xA1, 0x85, 0xAB, 0xE1, 0x2B, 0x6D, 0x6F, 0x70, 0x1D, 0xA3, 0xD7, 0x20, 0x26, 0x34, 0x5C, 0xFB, 0x26, 0x5C, 0x1F, 0x8F, 0x71, 0x6A, 0x95, 0xB1, 0x04, 0x00, 0x00, 0xCF, 0xA5 };
#else
/* buffer without CRC -> to be computed in HAL drivers */
uint8_t wMBus_SND_IR[SND_IR_LENGTH] = { 0x0E, 0x46, 0x8D, 0x4E, 0x11, 0x45, 0x63, 0x1F, 0x10, 0x04, /* 0xE9, 0x1F */ 0x28, 0x33, 0xE0, 0x88, 0x91 /*, 0x67, 0xB5 */ };
uint8_t wMBus_SND_NR[SND_NR_LENGTH] = { 0x31, 0x44, 0xB4, 0x4C, 0x12, 0x34, 0x45, 0x67, 0x50, 0x07,                                     /* 0x8D, 0xCA, */
                                        0x7B, 0x27, 0xD8, 0xC9, 0x6D, 0x0C, 0x32, 0xF1, 0x4E, 0x33, 0x16, 0xD0, 0xFF, 0x62, 0x98, 0x7D, /* 0x39, 0xA1, */
                                        0x85, 0xAB, 0xE1, 0x2B, 0x6D, 0x6F, 0x70, 0x1D, 0xA3, 0xD7, 0x20, 0x26, 0x34, 0x5C, 0xFB, 0x26, /* 0x5C, 0x1F, */
                                        0x8F, 0x71, 0x6A, 0x95, 0xB1, 0x04, 0x00, 0x00,                                                 /* 0xCF, 0xA5 */ };
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

#ifdef SINGLE_TIMER_OPT
void HAL_MRSUBG_TIMER_StartConfigureTimer(uint32_t time)
{
  uint32_t ABSOLUTE_TIME = LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC);

  if (time == TIMEOUT_500us)
  {
    LL_MRSUBG_TIMER_SetCPUWakeupTime(MR_SUBG_GLOB_RETAINED, ((ABSOLUTE_TIME + TIMEOUT_NO_DEEPSTOP) & TIMER_MAX_VALUE));
  }
  else
  {
    LL_MRSUBG_TIMER_SetCPUWakeupTime(MR_SUBG_GLOB_RETAINED, ((ABSOLUTE_TIME + TIMEOUT_WITH_DEEPSTOP) & TIMER_MAX_VALUE));
  }

  LL_MRSUBG_TIMER_EnableCPUWakeupTimer(MR_SUBG_GLOB_RETAINED);
}
#endif

void MX_wMBus_RadioTimerConfig(void)
{
  uint8_t ret_val = SUCCESS;

#ifdef DEBUG_GPIO_LINE
  GPIOB->BSRR = 1 << 4;
#endif

#ifdef SINGLE_TIMER_OPT
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
  if (ret_val != SUCCESS)
  {
    /* USER CODE BEGIN MX_wMBus_RadioTimerConfig_1 */
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    /* USER CODE END MX_wMBus_RadioTimerConfig_1 */
    while (1);
  }
#endif
#endif
}

void TimeoutCallback(void *timerHandle)
{
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
  if (ret_val != SUCCESS)
  {
    /* USER CODE BEGIN TimeoutCallback_1 */
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    /* USER CODE END TimeoutCallback_1 */
    while (1);
  }
  /* set wM-Bus transmission flag */
  wMBus_Transmission_VTimer_Callback = 1;

  /* USER CODE BEGIN TimeoutCallback_2 */
#ifdef PRINT_DEBUG
  printf("Virtual Timer Timeout\r\n");
#endif
  /* USER CODE END TimeoutCallback_2 */
}

#ifdef MULTIPLE_RADIO_TIMER
void TimeoutCallback2(void *timerHandle)
{
  uint8_t ret_val;

  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT2);

  if (ret_val != SUCCESS)
  {
    /* USER CODE BEGIN TimeoutCallback2_1 */
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    /* USER CODE END TimeoutCallback2_1 */
    while (1);
  }
  /* set Radio Timer application flag */
  VTimer_Callback2_flag = 1;

  /* USER CODE BEGIN TimeoutCallback2_2 */
#ifdef PRINT_DEBUG
  printf("Virtual Timer2 Timeout\r\n");
#endif
  /* USER CODE END TimeoutCallback2_2 */
}

void TimeoutCallback3(void *timerHandle)
{
  uint8_t ret_val;

  ret_val = HAL_MRSUBG_TIMER_StartVirtualTimerMs(timerHandle, WAKEUP_TIMEOUT3);

  if (ret_val != SUCCESS)
  {
    /* USER CODE BEGIN TimeoutCallback3_1 */
    printf("HAL_VTIMER_StartTimerMs() error 0x%02x\r\n", ret_val);
    /* USER CODE END TimeoutCallback3_1 */
    while (1);
  }
  /* set Radio Timer application flag */
  VTimer_Callback3_flag = 1;

  /* USER CODE BEGIN TimeoutCallback3_2 */
#ifdef PRINT_DEBUG
  printf("Virtual Timer3 Timeout\r\n");
#endif
  /* USER CODE END TimeoutCallback3_2 */
}
#endif

void MX_wMBus_Process()
{
#ifndef SINGLE_TIMER_OPT
  HAL_MRSUBG_TIMER_Tick();
#endif
  /* USER CODE BEGIN MX_wMBus_Process_1 */

  /* USER CODE END MX_wMBus_Process_1 */

  /* Wakeup source configuration */
  uint32_t wakeupInternal = HAL_PWREx_GetClearInternalWakeUpLine();

  if (wakeupInternal & PWR_WAKEUP_SUBGHOST)
  {
    /* USER CODE BEGIN MX_wMBus_Process_2 */
#ifdef PRINT_DEBUG
    BSP_LED_On(LD2);
    printf("Wakeup on Radio Timer!\r\n");
    BSP_LED_Off(LD2);
#endif
    /* USER CODE END MX_wMBus_Process_2 */
  }

  /* check if Wake-up for Wm-Bus transmission */
  if (wMBus_Transmission_VTimer_Callback == 1)
  {
    wMBus_Transmission_VTimer_Callback = 0;

#ifndef WMBUS_CRC_IN_HAL
    wMBus_Phy_prepare_Tx(wMBus_SND_IR, SND_IR_LENGTH);
#else
    wMBus_Phy_prepare_Tx_CRC_mngt(wMBus_SND_NR, SND_NR_LENGTH, wMBus_format);
#endif

    /* Send the TX command */
    wMBus_Phy_start_transmission();
#ifndef WMBUS_NO_BLOCKING_HAL
    /* wait for Tx done IRQ */
    wMBus_Phy_wait_Tx_completed();
#else
    /* wait for WMBUS_TX_COMPLETED event flag set */
    while (!wMBus_Phy_check_radio_events(WMBUS_TX_COMPLETED));
#endif
    /* USER CODE BEGIN MX_wMBus_Process_3 */
#ifdef PRINT_DEBUG
    printf("Wm-Bus transmission completed\r\n");
#endif
    /* USER CODE END MX_wMBus_Process_3 */
  }
}

#if (CFG_LPM_SUPPORTED == 1)
static PowerSaveLevels App_PowerSaveLevel_Check(void)
{
#ifdef NO_DEEPSTOP
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DISABLED;
#else
  PowerSaveLevels output_level = POWER_SAVE_LEVEL_DEEPSTOP_NOTIMER;
#ifdef SINGLE_TIMER_OPT
  if (LL_MRSUBG_TIMER_GetCPUWakeupTime(MR_SUBG_GLOB_RETAINED) < (LL_MRSUBG_TIMER_GetAbsoluteTime(MR_SUBG_GLOB_MISC) + LOW_POWER_THR))
  {
    output_level = POWER_SAVE_LEVEL_SLEEP;
  }
#endif
#endif

  /* USER CODE BEGIN App_PowerSaveLevel_Check_1 */

  /* USER CODE END App_PowerSaveLevel_Check_1 */

  return output_level;
}

__weak PowerSaveLevels HAL_MRSUBG_TIMER_PowerSaveLevelCheck()
{
  return POWER_SAVE_LEVEL_DEEPSTOP_TIMER;
}
#endif

void MX_wMBus_Idle()
{
  /* USER CODE BEGIN MX_wMBus_Idle_1 */
#if (CFG_LPM_SUPPORTED == 1)
  PowerSaveLevels app_powerSave_level, final_level;
#ifndef SINGLE_TIMER_OPT
  PowerSaveLevels vtimer_powerSave_level;
#endif

  app_powerSave_level = App_PowerSaveLevel_Check();

  if (app_powerSave_level != POWER_SAVE_LEVEL_DISABLED)
  {
#ifdef SINGLE_TIMER_OPT
    final_level = app_powerSave_level;
#else
    vtimer_powerSave_level = HAL_MRSUBG_TIMER_PowerSaveLevelCheck();
    final_level = (PowerSaveLevels)MIN(vtimer_powerSave_level, app_powerSave_level);
#endif

    switch (final_level)
    {
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

  /* USER CODE END MX_wMBus_Idle_1 */

  /* USER CODE BEGIN MX_wMBus_Idle_2 */

  /* USER CODE END MX_wMBus_Idle_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

static void SystemApp_Init(void)
{
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

static void wMBus_init()
{
  /* USER CODE BEGIN wMBus_init_1 */

  /* USER CODE END wMBus_init_1 */
  wMBus_mode = C_MODE;
  wMBus_format = WMBUS_FORMAT_A;
  wMBus_direction = METER_TO_OTHER;
  wMBus_Phy_init(wMBus_mode, wMBus_direction, wMBus_format);
  /* Start the VTimer  */
  MX_wMBus_RadioTimerConfig();
  /* USER CODE BEGIN wMBus_init_2 */
  printf("STM32WL3 wM-Bus Phy Demo - Meter Radio Timer.\r\n");
  printf("Wakeup every xxx seconds.\r\n");
  /* USER CODE END wMBus_init_2 */
}

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
