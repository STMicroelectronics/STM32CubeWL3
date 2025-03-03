/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    main.c
 * @author  MCD Application Team
 * @brief   This code implements a WmBus Phy Meter.
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef WMBUS_CRC_IN_HAL
#define SND_IR_LENGTH 19
#define SND_NR_LENGTH 58
#else
#define SND_IR_LENGTH 15
#define SND_NR_LENGTH 50
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
uint8_t WmBus_mode = 0;
uint8_t WmBus_format = 0;
uint8_t WmBus_direction = 0;

#ifndef WMBUS_CRC_IN_HAL
uint8_t Wm_Bus_SND_IR[SND_IR_LENGTH] = {0x0E, 0x46, 0x8D, 0x4E, 0x11, 0x45, 0x63, 0x1F, 0x10, 0x04, 0xE9, 0x1F, 0x28, 0x33, 0xE0, 0x88, 0x91, 0x67, 0xB5};
uint8_t Wm_Bus_SND_NR[SND_NR_LENGTH] = {0x31, 0x44, 0xB4, 0x4C, 0x12, 0x34, 0x45, 0x67, 0x50, 0x07, 0x8D, 0xCA, 0x7B, 0x27, 0xD8, 0xC9, 0x6D, 0x0C, 0x32, 0xF1, 0x4E, 0x33, 0x16, 0xD0, 0xFF, 0x62, 0x98, 0x7D, 0x39, 0xA1, 0x85, 0xAB, 0xE1, 0x2B, 0x6D, 0x6F, 0x70, 0x1D, 0xA3, 0xD7, 0x20, 0x26, 0x34, 0x5C, 0xFB, 0x26, 0x5C, 0x1F, 0x8F, 0x71, 0x6A, 0x95, 0xB1, 0x04, 0x00, 0x00, 0xCF, 0xA5};
#else
/* buffer without CRC -> to be computed in HAL drivers */
uint8_t Wm_Bus_SND_IR[SND_IR_LENGTH] = {0x0E, 0x46, 0x8D, 0x4E, 0x11, 0x45, 0x63, 0x1F, 0x10, 0x04, /* 0xE9, 0x1F */ 0x28, 0x33, 0xE0, 0x88, 0x91 /*, 0x67, 0xB5 */};
uint8_t Wm_Bus_SND_NR[SND_NR_LENGTH] = {0x31, 0x44, 0xB4, 0x4C, 0x12, 0x34, 0x45, 0x67, 0x50, 0x07,                                     /* 0x8D, 0xCA,*/
                                        0x7B, 0x27, 0xD8, 0xC9, 0x6D, 0x0C, 0x32, 0xF1, 0x4E, 0x33, 0x16, 0xD0, 0xFF, 0x62, 0x98, 0x7D, /* 0x39, 0xA1, */
                                        0x85, 0xAB, 0xE1, 0x2B, 0x6D, 0x6F, 0x70, 0x1D, 0xA3, 0xD7, 0x20, 0x26, 0x34, 0x5C, 0xFB, 0x26, /* 0x5C, 0x1F, */
                                        0x8F, 0x71, 0x6A, 0x95, 0xB1, 0x04, 0x00, 0x00,
                                        /* 0xCF, 0xA5 */};
#endif

/* USER CODE BEGIN PV */
#ifdef SENSE_RSSI
int32_t rssi_dbm;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* Configure the peripherals common clocks */
    PeriphCommonClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    WmBus_mode = C_MODE;
    WmBus_format = WMBUS_FORMAT_A;
    WmBus_direction = METER_TO_OTHER;
    MX_wmbphy_Init(WmBus_mode, WmBus_direction, WmBus_format);

    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
#ifdef SENSE_RSSI
        rssi_dbm = wmbphy_sense_rssi(1000);
        printf("Rx RSSI level: %ld dBm\r\n", rssi_dbm);
        HAL_Delay(1);
#endif
        /* USER CODE END 3 */

#ifndef WMBUS_CRC_IN_HAL
        wmbphy_prepare_Tx(Wm_Bus_SND_IR, SND_IR_LENGTH);
#else
        wmbphy_prepare_Tx_CRC_mngt(Wm_Bus_SND_NR, SND_NR_LENGTH, WmBus_format);
#endif

        /* Start continuous TX */
        wmbphy_start_transmission();

#ifndef WMBUS_NO_BLOCKING_HAL
        /* wait for Tx done IRQ */
        wmbphy_wait_Tx_completed();
#else
        /* wait for WMBUS_TX_COMPLETED event flag set */
        while (!wmbphy_check_radio_events(WMBUS_TX_COMPLETED));
#endif

        /* USER CODE BEGIN 4 */
        printf("Wm-Bus transmission completed\r\n");
        /* USER CODE END 4 */
        HAL_Delay(1000);

        /* USER CODE BEGIN 5 */
    }
    /* USER CODE END 5 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Configure the SYSCLKSource and SYSCLKDivider
     */
#ifndef WMBUS_ACTIVE_POWER_MODE_ENABLED
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_RC64MPLL;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_RC64MPLL_DIV1;
#else /* case 48MHz with direct HSE @ 16MHz system clock */

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_DIRECT_HSE;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_DIRECT_HSE_SWITCH_DIV3;

    uint8_t gmc = 10;

    MODIFY_REG_FIELD(RCC->RFSWHSECR, RCC_RFSWHSECR_GMC, gmc);
#endif

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WAIT_STATES_1) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
    PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLK_DIV4;
    PeriphClkInitStruct.KRMRateMultiplier = 4;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
    /* Stop HSI clock */
    MODIFY_REG_FIELD(RCC->CFGR, RCC_CFGR_STOPHSI, 0x01);
#endif
}

/* USER CODE BEGIN 6 */

/* USER CODE END 6 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while (1) {
    }

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN assert_failed */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\n", file, line) */
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */
    }
    /* USER CODE END assert_failed */
}
#endif /* USE_FULL_ASSERT */

/******************* (C) COPYRIGHT 2025 STMicroelectronics *****END OF FILE****/
