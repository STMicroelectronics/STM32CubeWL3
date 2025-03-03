/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    main.c
 * @author  MCD Application Team
 * @brief   This code implements a WmBus Phy Concentrator.
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
/* define of maximum buffer size: T-mode 3o6 frame format A */
#define MAX_WMBUS_PHY_PACKET 435
/* USER CODE BEGIN PD */
#define CRC_POLYNOM 0x3D65
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
uint8_t WmBus_mode = 0;
uint8_t WmBus_format = 0;
uint8_t WmBus_direction = 0;

uint8_t WmBus_RxBuffer[MAX_WMBUS_PHY_PACKET];
uint16_t WmBus_RxBuffer_length;
int32_t WmBus_RssiDbm = 0; /* Rx level of Rx Wmbus packet */
uint32_t WmBus_RxSync;

/* USER CODE BEGIN PV */
uint8_t WmBus_Valid_Packet_cnt = 0;
uint8_t xCrcResult = TRUE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
#ifndef WMBUS_CRC_IN_HAL
uint16_t wmbphy_CRC_calc(uint16_t crcReg, uint8_t crcData);
uint8_t wmbphy_CRC_check(uint8_t *pStart, uint8_t *pStop);
#endif

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */
#ifndef WMBUS_CRC_IN_HAL
/**
 * @brief  Calculates the 16-bit CRC.
 * @note   This function is defined only if WMBUS_CRC_IN_HAL is not defined
 * @param  crcReg Current or initial value of the CRC calculation
 * @param  crcData Data to perform the CRC-16 operation on.
 * @retval crcReg Updated CRC value
 */
uint16_t wmbphy_CRC_calc(uint16_t crcReg, uint8_t crcData) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
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
uint8_t wmbphy_CRC_check(uint8_t *pStart, uint8_t *pStop) {
    uint16_t seed = 0x0000;

    while (pStart != pStop) {
        seed = wmbphy_CRC_calc(seed, *pStart);
        pStart++;
    }
    seed = ~seed;
    if ((pStop[0] == (uint8_t)(seed >> 8)) && (pStop[1] == (uint8_t)(seed))) {
        return TRUE;
    }
    return FALSE;
}
#endif
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

    wmbphy_prepare_Rx();

#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
    wmbphy_set_active_power_mode(WMBUS_LPM);
#endif

    /* Start continuous RX */
    wmbphy_start_continuousRx();

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */
#ifndef WMBUS_NO_BLOCKING_HAL
#ifndef WMBUS_CRC_IN_HAL
        /* wait for Rx packet IRQ */
        WmBus_RxBuffer_length = wmbphy_wait_Rx_completed(WmBus_RxBuffer, &WmBus_RssiDbm, &WmBus_RxSync);

        /* USER CODE BEGIN 3 */
        printf("WmBus RAW buffer (with CRCs) received\r\n");
        printf("[ ");
        for (uint16_t i = 0; i < WmBus_RxBuffer_length; i++) {
            printf("%x ", WmBus_RxBuffer[i]);
        }
        printf("]\r\n");

        /* CRC verification */
        xCrcResult = TRUE;
        xCrcResult = wmbphy_CRC_check(&WmBus_RxBuffer[0], &WmBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2]);

        for (uint16_t i = 0; i < (WmBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) / WMBUS_BLOCK_SIZE_FORMAT_A; i++) {
            xCrcResult &= wmbphy_CRC_check(&WmBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * i], &WmBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * (i + 1) - 2]);
        }

        if ((WmBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A != 0)
            xCrcResult &= wmbphy_CRC_check(&WmBus_RxBuffer[WmBus_RxBuffer_length - ((WmBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A)], &WmBus_RxBuffer[WmBus_RxBuffer_length - 2]);

        if (xCrcResult) {
            WmBus_Valid_Packet_cnt++;
            printf("Valid Packet Received\r\n");
        }

        printf("Valid packet count: %d\r\n", WmBus_Valid_Packet_cnt);

        printf("Rx RSSI level: %d\r\n", WmBus_RssiDbm);
        /* USER CODE END 3 */
#else
        WmBus_RxBuffer_length = wmbphy_wait_Rx_CRC_mngt(WmBus_RxBuffer, &WmBus_RssiDbm, WmBus_format, &WmBus_RxSync, &WmBus_RssiDbm);

        /* USER CODE BEGIN 4 */
        printf("WmBus RAW buffer (with CRCs) received\r\n");

        printf("WmBus_RxBuffer_length: %d\r\n", WmBus_RxBuffer_length);

        if (WmBus_RxBuffer_length != 0) {
            printf("WmBus buffer - CRCs checked - received\r\n");
            printf("[ ");
            for (uint16_t i = 0; i < WmBus_RxBuffer_length; i++) {
                printf("%x ", WmBus_RxBuffer[i]);
            }
            printf("]\r\n");

            printf("Valid Packet Received\r\n");

            WmBus_Valid_Packet_cnt++;

            printf("Valid packet count: %d\r\n", WmBus_Valid_Packet_cnt);

            printf("Rx RSSI level: %d\r\n", WmBus_RssiDbm);
        }
        /* USER CODE END 4 */
#endif
#else /* WMBUS_NO_BLOCKING_HAL */
        /* register LL WmBus buffer & size into HAL for copy - and verification */
        /* indicate LL buffer size in WmBus_RxBuffer_length */
        WmBus_RxBuffer_length = MAX_WMBUS_PHY_PACKET;
        wmbphy_register_LL_buffer(WmBus_RxBuffer, &WmBus_RxBuffer_length, &WmBus_format, &WmBus_RxSync, &WmBus_RssiDbm);

#ifndef WMBUS_CRC_IN_HAL
        /* wait for WMBUS_RX_COMPLETED_WITH_RAW_BUFFER event flag set */
        while (!wmbphy_check_radio_events(WMBUS_RX_COMPLETED_WITH_RAW_BUFFER));

        /* USER CODE BEGIN 5 */
        printf("WmBus RAW buffer (with CRCs) received\r\n");
        printf("[ ");
        for (uint16_t i = 0; i < WmBus_RxBuffer_length; i++) {
            printf("%x ", WmBus_RxBuffer[i]);
        }
        printf("]\r\n");

        /* CRC verification */
        xCrcResult = TRUE;
        xCrcResult = wmbphy_CRC_check(&WmBus_RxBuffer[0], &WmBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2]);

        for (uint16_t i = 0; i < (WmBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) / WMBUS_BLOCK_SIZE_FORMAT_A; i++) {
            xCrcResult &= wmbphy_CRC_check(&WmBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * i], &WmBus_RxBuffer[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + WMBUS_BLOCK_SIZE_FORMAT_A * (i + 1) - 2]);
        }

        if ((WmBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A != 0)
            xCrcResult &= wmbphy_CRC_check(&WmBus_RxBuffer[WmBus_RxBuffer_length - ((WmBus_RxBuffer_length - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A)], &WmBus_RxBuffer[WmBus_RxBuffer_length - 2]);

        if (xCrcResult) {
            WmBus_Valid_Packet_cnt++;
            printf("Valid Packet Received\r\n");

            printf("Rx RSSI level: %d\r\n", WmBus_RssiDbm);
        }

        printf("Valid packet count: %d\r\n", WmBus_Valid_Packet_cnt);
        /* USER CODE END 5 */
#else /* #ifndef WMBUS_CRC_IN_HAL */
        MX_wmbphy_Process();

        /* wait for WMBUS_RX_COMPLETED_WITH_VALID_CRC event flag set */
        while (!wmbphy_check_radio_events(WMBUS_RX_COMPLETED_WITH_VALID_CRC)) {
            /* USER CODE BEGIN 6 */
            /* while no valid packet is received - we can check other event */
            if (wmbphy_check_radio_events(WMBUS_RX_COMPLETED_WITH_CRC_ERROR) == 1) {
                printf("CRC error\r\n");
            }
            if (wmbphy_check_radio_events(WMBUS_FIRST_CRC_ERROR) == 1) {
                printf("CRC error on 1st Block\r\n");
            }
            if (wmbphy_check_radio_events(WMBUS_RX_VALID_HEADER_ERROR) == 1) {
                printf("Packet Header error\r\n");
            }
            if (wmbphy_check_radio_events(WMBUS_RX_OVERFLOW_ERROR) == 1) {
                printf("Packet payload overflow error\r\n");
            }
            /* USER CODE END 6 */
            MX_wmbphy_Process();
        }

        /* USER CODE BEGIN 7 */
        /* here it means valid packet has been received */
        printf("WmBus_RxBuffer_length: %d\r\n", WmBus_RxBuffer_length);

        printf("WmBus buffer - CRCs checked - received\r\n");
        printf("[ ");
        for (uint16_t i = 0; i < WmBus_RxBuffer_length; i++) {
            printf("%x ", WmBus_RxBuffer[i]);
        }
        printf("]\r\n");

        printf("Valid Packet Received\r\n");

        printf("Rx RSSI level: %ld\r\n", WmBus_RssiDbm);

        WmBus_Valid_Packet_cnt++;

        printf("Valid packet count: %d\r\n", WmBus_Valid_Packet_cnt);
        /* USER CODE END 7 */
#endif
#endif

        /* USER CODE BEGIN 8 */
        printf("\r\n");
    }
    /* USER CODE END 8 */
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

/* USER CODE BEGIN 9 */

/* USER CODE END 9 */

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
