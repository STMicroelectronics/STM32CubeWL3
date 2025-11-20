/**
  ******************************************************************************
  * @file    stm32wl3_wMBus_Phy_radio.c
  * @author  MCD Application Team
  * @brief   wM-Bus Phy Radio module driver.
  *          This file provides firmware functions to manage the Radio module
  *          of the wM-Bus Phy driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024-2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal.h"
#include "stm32wl3_wMBus_Phy_radio.h"
#include "stm32wl3_wMBus_Phy_pwr.h"

/** @addtogroup STM32WL3_wM-Bus
  * @{
  */

/** @addtogroup Phy
  * @{
  */

/** @addtogroup Radio
  * @{
  */

/** @defgroup Radio_Private_Constants Radio Private Constants
  * @{
  */
#define WMBUS_FIRST_BLOCK_SIZE_FORMAT_A 12 /**< First block size for format A */
#define WMBUS_FIRST_BLOCK_SIZE_FORMAT_B 10 /**< First block size for format B */

#define WMBUS_SECOND_BLOCK_MAX_SIZE_FORMAT_B 118 /**< Maximum size of the second block for format B, including CRC */

#ifdef WMBUS_FIRST_CRC_CHECK                                // only valid for frame format A
#define WMBUS_FIRST_RXBUFFER_THRESHOLD_TMODE_FFA 12         /**< First RX buffer threshold for T-mode, frame format A */
#define WMBUS_FIRST_RXBUFFER_THRESHOLD_CMODE_FFA 12         /**< First RX buffer threshold for C-mode, frame format A */
#define WMBUS_FIRST_RXBUFFER_THRESHOLD_CMODE_FF_DYNAMCIC 13 /**< First RX buffer threshold for C-mode, dynamic frame format */
#define WMBUS_FIRST_RXBUFFER_THRESHOLD_SMODE_FFA 12         /**< First RX buffer threshold for S-mode, frame format A */
#define WMBUS_FIRST_RXBUFFER_THRESHOLD_NMODE_FFA 12         /**< First RX buffer threshold for N-mode, frame format A */
#define WMBUS_FIRST_RXBUFFER_THRESHOLD_CTMODE_FF_DYNAMIC 18 /**< First RX buffer threshold for CT mode, dynamic frame format */
#if !defined(T1C1_ACTIVATED) && !defined(T1C1_ACTIVATED_C1_MODE)
#define WMBUS_FIRST_RXBUFFER_THRESHOLD WMBUS_FIRST_RXBUFFER_THRESHOLD_CMODE_FF_DYNAMCIC /**< Maximum value for the number of RX bytes to receive, for C-mode dynamic frame format */
#else
#define WMBUS_FIRST_RXBUFFER_THRESHOLD WMBUS_FIRST_RXBUFFER_THRESHOLD_CTMODE_FF_DYNAMIC /**< Maximum value for the number of RX bytes to receive, for CT-mode dynamic frame format */
#endif
#endif

#ifndef CLKREC_PLL_MODE
#define RSSI_THRESHOLD -111 /**< RSSI threshold if CLKREC_PLL_MODE is not defined */
#else
#define RSSI_THRESHOLD -105 /**< RSSI threshold if CLKREC_PLL_MODE is defined */
#endif

#define CRC_POLYNOM 0x3D65 /**< CRC polynomial for wM-Bus Phy Radio */

#ifdef WMBUS_DEBUG
#define WMBUS_LOG_BUFFER_SIZE 100 /**< Size of the debug log buffer */
#endif

#define T_S_C_N2G_FAC_VALUE 2 /**< FAC value (N) for T, S, C modes: N = 2, 3 or 5 */
#define N2A_N2F_FAC_VALUE 5   /**< FAC value (N) for T, S, C modes: N = 5, 7 or 13 */
/**
  * @}
  */

/** @defgroup Radio_Private_Macros Radio Private Macros
  * @{
  */
#define CEILING(X) (X - (uint16_t)(X) > 0 ? (uint16_t)(X + 1) : (uint16_t)(X)) /**< Ceiling function */
/**
  * @}
  */

/** @defgroup Radio_Private_Variables Radio Private Variables
  * @{
  */
/**
  * @brief Define frequencies depending on mode and direction.
  */
const uint32_t wMBus_Phy_Frequency[WMBUS_MODE_NUMBER][2] =
{
  {868950000, 868300000}, /**< T-mode: M2O = 868950000 Hz, O2M = 868300000 Hz */
  {868300000, 868300000}, /**< S-mode: M2O = 868300000 Hz, O2M = 868300000 Hz */
  {868300000, 868300000}, /**< S1M-mode: M2O = 868300000 Hz, O2M = 868300000 Hz */
  {868950000, 869525000}, /**< C-mode: M2O = 868950000 Hz, O2M = 869525000 Hz */
  {169406250, 169406250}, /**< N-mode - N1a, N2a: M2O = 169406250 Hz, O2M = 169406250 Hz */
  {169418750, 169418750}, /**< N-mode - N1b, N2b: M2O = 169418750 Hz, O2M = 169418750 Hz */
  {169431250, 169431250}, /**< N-mode - N1c, N2c: M2O = 169431250 Hz, O2M = 169431250 Hz */
  {169443750, 169443750}, /**< N-mode - N1d, N2d: M2O = 169443750 Hz, O2M = 169443750 Hz */
  {169456250, 169456250}, /**< N-mode - N1e, N2e: M2O = 169456250 Hz, O2M = 169456250 Hz */
  {169468750, 169468750}, /**< N-mode - N1f, N2f: M2O = 169468750 Hz, O2M = 169468750 Hz */
  {169437500, 169437500}  /**< N-mode - N2g: M2O = 169437500 Hz, O2M = 169437500 Hz */
};

/**
  * @brief Define data rates depending on mode and direction.
  */
const uint32_t wMBus_Phy_Datarate[WMBUS_MODE_NUMBER][2] =
{
  {100000, 32768}, /**< T-mode: M2O = 100000 bps, O2M = 32768 bps */
  {32768, 32768},  /**< S-mode: M2O = 32768 bps, O2M = 32768 bps */
  {32768, 32768},  /**< S1M-mode: M2O = 32768 bps, O2M = 32768 bps */
  {100000, 50000}, /**< C-mode: M2O = 100000 bps, O2M = 50000 bps */
  {4800, 4800},    /**< N-mode - N1a, N2a: M2O = 4800 bps, O2M = 4800 bps */
  {4800, 4800},    /**< N-mode - N1b, N2b: M2O = 4800 bps, O2M = 4800 bps */
  {2400, 2400},    /**< N-mode - N1c, N2c: M2O = 2400 bps, O2M = 2400 bps */
  {2400, 2400},    /**< N-mode - N1d, N2d: M2O = 2400 bps, O2M = 2400 bps */
  {4800, 4800},    /**< N-mode - N1e, N2e: M2O = 4800 bps, O2M = 4800 bps */
  {4800, 4800},    /**< N-mode - N1f, N2f: M2O = 4800 bps, O2M = 4800 bps */
  {9600, 9600}     /**< N-mode - N2g 4-GFSK symbol rate: M2O = 4800 bps, O2M = 4800 bps */
};

/**
  * @brief Define frequency deviation depending on mode and direction.
  */
const uint32_t wMBus_Phy_FreqDev[WMBUS_MODE_NUMBER][2] =
{
  {50000, 50000}, /**< T-mode: M2O = 50 kHz, O2M = 50 kHz */
  {50000, 50000}, /**< S-mode: M2O = 50 kHz, O2M = 50 kHz */
  {50000, 50000}, /**< S1M-mode: M2O = 50 kHz, O2M = 50 kHz */
  {45000, 25000}, /**< C-mode: M2O = 45 kHz, O2M = 25 kHz */
  {2400, 2400},   /**< N-mode - N1a, N2a: M2O = 2.4 kHz, O2M = 2.4 kHz */
  {2400, 2400},   /**< N-mode - N1b, N2b: M2O = 2.4 kHz, O2M = 2.4 kHz */
  {2400, 2400},   /**< N-mode - N1c, N2c: M2O = 2.4 kHz, O2M = 2.4 kHz */
  {2400, 2400},   /**< N-mode - N1d, N2d: M2O = 2.4 kHz, O2M = 2.4 kHz */
  {2400, 2400},   /**< N-mode - N1e, N2e: M2O = 2.4 kHz, O2M = 2.4 kHz */
  {2400, 2400},   /**< N-mode - N1f, N2f: M2O = 2.4 kHz, O2M = 2.4 kHz */
  {7200, 7200}    /**< N-mode - N2g: M2O = 7.2 kHz, O2M = 7.2 kHz */
};

/**
  * @brief Define modulation type depending on mode and direction.
  */
const MRSubGModSelect wMBus_Phy_Modulation[WMBUS_MODE_NUMBER][2] =
{
  {MOD_2FSK, MOD_2FSK},       /**< T-mode: M2O = 2-FSK, O2M = 2-FSK */
  {MOD_2FSK, MOD_2FSK},       /**< S-mode: M2O = 2-FSK, O2M = 2-FSK */
  {MOD_2FSK, MOD_2FSK},       /**< S1M-mode: M2O = 2-FSK, O2M = 2-FSK */
  {MOD_2FSK, MOD_2GFSK05},    /**< C-mode: M2O = 2-FSK, O2M = 2GFSK BT = 0.5 */
  {MOD_2GFSK05, MOD_2GFSK05}, /**< N-mode - N1a, N2a: M2O = 2GFSK BT = 0.5, O2M = 2GFSK BT = 0.5 */
  {MOD_2GFSK05, MOD_2GFSK05}, /**< N-mode - N1b, N2b: M2O = 2GFSK BT = 0.5, O2M = 2GFSK BT = 0.5 */
  {MOD_2GFSK05, MOD_2GFSK05}, /**< N-mode - N1c, N2c: M2O = 2GFSK BT = 0.5, O2M = 2GFSK BT = 0.5 */
  {MOD_2GFSK05, MOD_2GFSK05}, /**< N-mode - N1d, N2d: M2O = 2GFSK BT = 0.5, O2M = 2GFSK BT = 0.5 */
  {MOD_2GFSK05, MOD_2GFSK05}, /**< N-mode - N1e, N2e: M2O = 2GFSK BT = 0.5, O2M = 2GFSK BT = 0.5 */
  {MOD_2GFSK05, MOD_2GFSK05}, /**< N-mode - N1f, N2f: M2O = 2GFSK BT = 0.5, O2M = 2GFSK BT = 0.5 */
  {MOD_4GFSK05, MOD_4GFSK05}  /**< N-mode - N2g: M2O = 4GFSK BT = 0.5, O2M = 4GFSK BT = 0.5 */
};

/**
  * @brief Define channel bandwidth depending on mode and direction.
  */
const uint32_t wMBus_Phy_ChannelBandwidth[WMBUS_MODE_NUMBER][2] =
{
#ifndef WMBUS_T_MODE_PERFORMANCE_FREQ_OFFSET
  {250000, 200000}, /**< T-mode if WMBUS_T_MODE_PERFORMANCE_FREQ_OFFSET is not defined: M2O = 250 kHz, O2M = 200 kHz */
#else
#ifndef CLKREC_PLL_MODE
  {270000, 270000}, /**< T-mode if WMBUS_T_MODE_PERFORMANCE_FREQ_OFFSET is defined and CLKREC_PLL_MODE is not defined: M2O = 270 kHz, O2M = 270 kHz */
#else
  {300000, 300000}, /**< T-mode if WMBUS_T_MODE_PERFORMANCE_FREQ_OFFSET is defined and CLKREC_PLL_MODE is defined: M2O = 300 kHz, O2M = 300 kHz */
#endif
#endif
  {150000, 150000}, /**< S-mode: M2O = 150 kHz, O2M = 150 kHz */
  {150000, 150000}, /**< S1M-mode: M2O = 150 kHz, O2M = 150 kHz */
  {200000, 125000}, /**< C-mode: M2O = 200 kHz, O2M = 125 kHz */
  {10000, 10000},   /**< N-mode - N1a, N2a: M2O = 10 kHz, O2M = 10 kHz */
  {10000, 10000},   /**< N-mode - N1b, N2b: M2O = 10 kHz, O2M = 10 kHz */
  {10000, 10000},   /**< N-mode - N1c, N2c: M2O = 10 kHz, O2M = 10 kHz */
  {10000, 10000},   /**< N-mode - N1d, N2d: M2O = 10 kHz, O2M = 10 kHz */
  {10000, 10000},   /**< N-mode - N1e, N2e: M2O = 10 kHz, O2M = 10 kHz */
  {10000, 10000},   /**< N-mode - N1f, N2f: M2O = 10 kHz, O2M = 10 kHz */
  {40000, 40000}    /**< N-mode - N2g: M2O = 40 kHz, O2M = 40 kHz */
};

/**
  * @brief Define the number of times the standard preamble sequence shall be repeated depending on mode and direction.
  */
const uint16_t wMBus_Phy_PreambleLength[WMBUS_MODE_NUMBER][2] =
{
  {12, 15},  /**< T-mode: M2O = 12, O2M = 15 */
  {276, 15}, /**< S-mode: M2O = 276, O2M = 15 */
  {12, 15},  /**< S1M-mode: M2O = 12, O2M = 15 */
  {16, 16},  /**< C-mode: M2O = 16, O2M = 16 */
  {8, 8},    /**< N-mode - N1a, N2a: M2O = 8, O2M = 8 */
  {8, 8},    /**< N-mode - N1b, N2b: M2O = 8, O2M = 8 */
  {8, 8},    /**< N-mode - N1c, N2c: M2O = 8, O2M = 8 */
  {8, 8},    /**< N-mode - N1d, N2d: M2O = 8, O2M = 8 */
  {8, 8},    /**< N-mode - N1e, N2e: M2O = 8, O2M = 8 */
  {8, 8},    /**< N-mode - N1f, N2f: M2O = 8, O2M = 8 */
  {8, 8}     /**< N-mode - N2g 4-GFSK preamble pattern: 8x(0111): M2O = 8, O2M = 8 */
};

/**
  * @brief Define sync length depending on mode and direction.
  * on gateway side better to align to 24-bit Sync pattern for Meter to Other direction
  */
const uint8_t wMBus_Phy_SyncLength[WMBUS_MODE_NUMBER][2] =
{
  {24, 18}, /**< T-mode: M2O = 24, O2M = 18 */
  {24, 18}, /**< S-mode: M2O = 24, O2M = 18 */
  {24, 18}, /**< S1M-mode: M2O = 24, O2M = 18 */
  {32, 32}, /**< C-mode: M2O = 32, O2M = 32 */
  {16, 16}, /**< N-mode - N1a, N2a: M2O = 16, O2M = 16 */
  {16, 16}, /**< N-mode - N1b, N2b: M2O = 16, O2M = 16 */
  {16, 16}, /**< N-mode - N1c, N2c: M2O = 16, O2M = 16 */
  {16, 16}, /**< N-mode - N1d, N2d: M2O = 16, O2M = 16 */
  {16, 16}, /**< N-mode - N1e, N2e: M2O = 16, O2M = 16 */
  {16, 16}, /**< N-mode - N1f, N2f: M2O = 16, O2M = 16 */
  {32, 32}  /**< N-mode - N2g specific setting with FORCE_2FSK_SYNC_MODE bit enabled: M2O = 32, O2M = 32 */
};

/**
  * @brief Define sync pattern depending on mode and direction.
  */
const uint32_t wMBus_Phy_SyncPattern[WMBUS_MODE_NUMBER][2] =
{
  {0x55543D00, 0x1DA58000}, /**< T-mode: M2O = 0x55543D00, O2M = 0x1DA58000 */
  {0x54769600, 0x1DA58000}, /**< S-mode: M2O = 0x54769600, O2M = 0x1DA58000 */
  {0x54769600, 0x1DA58000}, /**< S1M-mode: M2O = 0x54769600, O2M = 0x1DA58000 */
  {0x543D54CD, 0x543D54CD}, /**< C-mode: M2O = 0x543D54CD, O2M = 0x543D54CD */
  {0xF68D0000, 0xF68D0000}, /**< N-mode - N1a, N2a: M2O = 0xF68D0000, O2M = 0xF68D0000 */
  {0xF68D0000, 0xF68D0000}, /**< N-mode - N1b, N2b: M2O = 0xF68D0000, O2M = 0xF68D0000 */
  {0xF68D0000, 0xF68D0000}, /**< N-mode - N1c, N2c: M2O = 0xF68D0000, O2M = 0xF68D0000 */
  {0xF68D0000, 0xF68D0000}, /**< N-mode - N1d, N2d: M2O = 0xF68D0000, O2M = 0xF68D0000 */
  {0xF68D0000, 0xF68D0000}, /**< N-mode - N1e, N2e: M2O = 0xF68D0000, O2M = 0xF68D0000 */
  {0xF68D0000, 0xF68D0000}, /**< N-mode - N1f, N2f: M2O = 0xF68D0000, O2M = 0xF68D0000 */
  {0xFF7DD5F7, 0xFF7DD5F7}  /**< N-mode - N2g: M2O = 0xFF7DD5F7, O2M = 0xFF7DD5F7 */
};

/**
  * @brief Define channel coding depending on mode and direction.
  */
const uint8_t wMBus_Phy_ChannelCoding[WMBUS_MODE_NUMBER][2] =
{
  {CODING_3o6, CODING_MANCHESTER},        /**< T-mode: M2O = 3 out of 6, O2M = Manchester */
  {CODING_MANCHESTER, CODING_MANCHESTER}, /**< S-mode: M2O = Manchester, O2M = Manchester */
  {CODING_MANCHESTER, CODING_MANCHESTER}, /**< S1M-mode: M2O = Manchester, O2M = Manchester */
  {CODING_NONE, CODING_NONE},             /**< C-mode: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N1a, N2a: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N1b, N2b: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N1c, N2c: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N1d, N2d: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N1e, N2e: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N1f, N2f: M2O = None, O2M = None */
  {CODING_NONE, CODING_NONE},             /**< N-mode - N2g: M2O = None, O2M = None */
};

#ifdef WMBUS_RX_PERFORMANCE_ENABLED
/**
  * @brief Define optimized Rx RSSI thresholds depending on mode and direction.
  *        Register values assuming 0x00 => -160dBm with 1/2dB steps
  *        Example: decimal register value = 100 to obtain -110dbm
  * @note  This const is defined only if WMBUS_RX_PERFORMANCE_ENABLED is defined.
  */
const uint16_t wMBus_Phy_Rx_RssiThreshold[WMBUS_MODE_NUMBER][2] =
{
  {102, 98}, /**< T-mode: M2O = -109dBm, O2M = -111dBm */
  {98, 98},  /**< S-mode: M2O = -111dBm, O2M = -111dBm */
  {98, 98},  /**< S1M-mode: M2O = -111dBm, O2M = -111dBm */
  {98, 98},  /**< C-mode: M2O = -111dBm, O2M = -111dBm */
  {60, 60},  /**< N-mode - N1a, N2a: M2O = -130dBm, O2M = -130dBm */
  {60, 60},  /**< N-mode - N1b, N2b: M2O = -130dBm, O2M = -130dBm */
  {60, 60},  /**< N-mode - N1c, N2c: M2O = -130dBm, O2M = -130dBm */
  {60, 60},  /**< N-mode - N1d, N2d: M2O = -130dBm, O2M = -130dBm */
  {60, 60},  /**< N-mode - N1e, N2e: M2O = -130dBm, O2M = -130dBm */
  {60, 60},  /**< N-mode - N1f, N2f: M2O = -130dBm, O2M = -130dBm */
  {100, 100} /**< N-mode - N2g: M2O = -110dBm, O2M = -110dBm */
};

/**
  * @brief Define optimized Rx Clock Recovery values depending on mode and direction.
  *        Pair of values for CLK_REC0 and CLK_REC1 registers {CLK_REC0, CLK_REC1}.
  */
const uint8_t wMBus_Phy_Rx_ClkRec[WMBUS_MODE_NUMBER][2][2] =
{
  {{0x98, 0x18}, {0x98, 0x18}}, /**< T-mode, optimized values */
  {{0x98, 0x18}, {0x98, 0x18}}, /**< S-mode, optimized values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< S1M-mode, default values */
  {{0x98, 0x18}, {0x98, 0x18}}, /**< C-mode, optimized values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< N-mode - N1a, N2a, default values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< N-mode - N1b, N2b, default values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< N-mode - N1c, N2c, default values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< N-mode - N1d, N2d, default values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< N-mode - N1e, N2e, default values */
  {{0xB8, 0x5C}, {0xB8, 0x5C}}, /**< N-mode - N1f, N2f, default values */
  {{0x38, 0x5C}, {0x38, 0x5C}}  /**< N-mode - N2g, default values */
};
#endif

/**
  * @brief Define {Min, Max} Response delay (tRO) in us depending on mode.
  */
const uint32_t wMBus_Phy_responseDelay[WMBUS_MODE_NUMBER][2] =
{
  {2000, 3000},    /**< T-mode: Min = 2000us=2ms, Max = 3000us=3ms */
  {3000, 50000},   /**< S-mode: Min = 3000us=3ms, Max = 50000us=50ms */
  {3000, 50000},   /**< S1M-mode: Min = 3000us=3ms, Max = 50000us=50ms */
  {99500, 100500}, /**< C-mode: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}, /**< N-mode - N1a, N2a: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}, /**< N-mode - N1b, N2b: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}, /**< N-mode - N1c, N2c: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}, /**< N-mode - N1d, N2d: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}, /**< N-mode - N1e, N2e: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}, /**< N-mode - N1f, N2f: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
  {99500, 100500}  /**< N-mode - N2g: Min = 99500us=99.5ms, Max = 100500us=100.5ms */
};

/**
  * @brief Define FAC transmission delay (tTxD) in us depending on mode.
  */
const uint32_t wMBus_Phy_FACTransmissionDelay[WMBUS_MODE_NUMBER] = 
{
  T_S_C_N2G_FAC_VALUE * 1000, /**< T-mode: N*1000us, N=2, 3 or 5 */
  T_S_C_N2G_FAC_VALUE * 1000, /**< S-mode: N*1000us, N=2, 3 or 5 */
  0,                          /**< S1M-mode: not exists */
  T_S_C_N2G_FAC_VALUE * 1000, /**< C-mode: N*1000us, N=2, 3 or 5 */
  N2A_N2F_FAC_VALUE * 1000,   /**< N-mode - N2a: N*1000us, N=5, 7 or 13 */
  N2A_N2F_FAC_VALUE * 1000,   /**< N-mode - N2b: N*1000us, N=5, 7 or 13 */
  N2A_N2F_FAC_VALUE * 1000,   /**< N-mode - N2c: N*1000us, N=5, 7 or 13 */
  N2A_N2F_FAC_VALUE * 1000,   /**< N-mode - N2d: N*1000us, N=5, 7 or 13 */
  N2A_N2F_FAC_VALUE * 1000,   /**< N-mode - N2e: N*1000us, N=5, 7 or 13 */
  N2A_N2F_FAC_VALUE * 1000,   /**< N-mode - N2f: N*1000us, N=5, 7 or 13 */
  T_S_C_N2G_FAC_VALUE * 1000  /**< N-mode - N2g: N*1000us, N=2, 3 or 5 */
};

SMRSubGConfig_t wMBus_Phy_RadioInitStruct = {0};

MRSubG_PcktBasicFields_t wMBus_Phy_PacketSettingsStruct = {0};

volatile FlagStatus xRxDoneFlag = RESET;
volatile FlagStatus xSabortCompletedFlag = RESET;

static uint8_t wMBus_Auto_Rx = 0;                                                        /**< Variable to relaunch Rx automatically: 0= not automatically relaunched, 1 = automatically relaunched */
static uint8_t vectcRadioBuff[WMBUS_RADIO_BUFFER_SIZE + WMBUS_RADIO_BUFFER_SIZE_MARGIN]; /**< Radio buffer directly connected to Radio IP */
static uint8_t vectcRadioBuff_copy[WMBUS_RADIO_BUFFER_SIZE_MAX_NRZ];                     /**< Radio buffer copy - used for vectcRadioBuff[] buffer manipulation - especially if T1C1_ACTIVATED */

volatile uint8_t xRxState = SM_RX_NEW_PACKET;

static uint16_t RssiOnSync = 0; /**< Rssi level measured once Sync is done */
static uint16_t RssiRxDone = 0; /**< RssiOnSync to be copied into RssiRxDone at the end of reception */

static uint32_t RxSync_Ongoing = 0; /**< Save Sync pattern during Rx */
static uint32_t RxSync_RxDone = 0;  /**< RxSync_Ongoing to be copied into RxSync_RxDone at the end of reception */

static uint8_t cLField; /**< L-field value */

static uint16_t nPcktLength = 0; /**< Packet length variable*/
#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE)
static uint16_t nPcktLengthRaw = 0; /**< Packet length variable for T1C1 mode */
#endif

#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE) || defined(AUTOMATIC_FORMAT_DETECTION_C_MODE)
static uint8_t wMBus_mode_global = 0;   /*< Global copy of wM-Bus configuration - mode */
#endif
static uint8_t wMBus_Format_global = 0; /*< Global copy of wM-Bus configuration - frame format */

static uint32_t wMBus_Radio_Event_bitmap = 0; /**< Bitmap of wM-Bus Radio events */

static uint8_t *global_buffer_ptr;        /**< Global buffer pointer */
static uint16_t *global_buffer_size_ptr;  /**< Global buffer size pointer */
static uint16_t global_buffer_allocation; /**< Global buffer allocation */
static uint8_t *global_buffer_format_ptr; /**< Global buffer format pointer */
static int32_t *global_RssiDbm_ptr;       /**< Global RSSI pointer */
static uint32_t *global_RxSync_ptr;       /**< Global Rx Sync pointer */

#ifdef WMBUS_DEBUG
static uint8_t wMBus_debug_log_index = 0;                        /**< Index of the debug log buffer */
static wMBus_Phy_debug_t wMBus_debug_log[WMBUS_LOG_BUFFER_SIZE]; /**< Debug log buffer */
#endif

wMBus_Phy_tRO_t wMBus_tRO = {0}; /**< tRO structure */
uint32_t wMBus_tTxD = 0;     /**< tTxD: FAC Transmission delay */

/**
  * @}
  */

/** @addtogroup Radio_Private_Functions
  * @{
  */
#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
__weak int adjustFrequencyBaseFor169(void);
__weak void revertFrequencyBaseFor169(void);
#endif
__weak void FEM_Operation(FEM_OperationType operation);
#endif
/**
  * @brief  Initializes the low-level hardware resources for the wM-Bus Phy radio.
  * @retval None
  */
static void wMBus_Phy_MRSubG_MspInit(void);
/**
  * @}
  */

/** @defgroup Radio_Exported_Functions Radio Exported Functions
  * @{
  */

wMBus_Phy_tRO_t wMBus_Phy_getTiming()
{
  return wMBus_tRO;
}

uint32_t wMBus_Phy_getTxD()
{
  return wMBus_tTxD;
}

void wMBus_Phy_init(uint8_t wMBus_mode, uint8_t wMBus_Direction, uint8_t wMBus_Format)
{
  /* Configures the radio parameters */
  wMBus_Phy_RadioInitStruct.lFrequencyBase = wMBus_Phy_Frequency[wMBus_mode][wMBus_Direction];
  wMBus_Phy_RadioInitStruct.xModulationSelect = wMBus_Phy_Modulation[wMBus_mode][wMBus_Direction];
  wMBus_Phy_RadioInitStruct.lDatarate = wMBus_Phy_Datarate[wMBus_mode][wMBus_Direction];
  wMBus_Phy_RadioInitStruct.lFreqDev = wMBus_Phy_FreqDev[wMBus_mode][wMBus_Direction];
  wMBus_Phy_RadioInitStruct.lBandwidth = wMBus_Phy_ChannelBandwidth[wMBus_mode][wMBus_Direction];
  wMBus_Phy_RadioInitStruct.dsssExp = 0;
  wMBus_Phy_RadioInitStruct.PADrvMode = PA_DRV_TX_HP;
  /* +10dBm Tx output power */
#ifdef STM32WL33XA
  wMBus_Phy_RadioInitStruct.outputPower = -8;
#else
  wMBus_Phy_RadioInitStruct.outputPower = 10;
#endif

  wMBus_Phy_MRSubG_MspInit();
  HAL_MRSubG_Init(&wMBus_Phy_RadioInitStruct);

  /* Configure Timing parameters */
  wMBus_tRO.tRO_min = wMBus_Phy_responseDelay[wMBus_mode][0];
  wMBus_tRO.tRO_max = wMBus_Phy_responseDelay[wMBus_mode][1];
  wMBus_tTxD = wMBus_Phy_FACTransmissionDelay[wMBus_mode];

  /* Configures the packet parameters */
  wMBus_Phy_PacketSettingsStruct.PreambleLength = wMBus_Phy_PreambleLength[wMBus_mode][wMBus_Direction];
  wMBus_Phy_PacketSettingsStruct.PostambleLength = PREAMBLE_BYTE(0);
  /* set Tx and Rx Sync settings - by default same but can be different in some special cases */
  wMBus_Phy_PacketSettingsStruct.SyncLength = wMBus_Phy_SyncLength[wMBus_mode][wMBus_Direction] - 1;
  wMBus_Phy_PacketSettingsStruct.SyncWord = wMBus_Phy_SyncPattern[wMBus_mode][wMBus_Direction];

  if (wMBus_mode == N_MODE_Ng)
  {
    /* specific configuration for Sync pattern : need to force 2-FSK SYnc pattern : set FORCE_2FSK_SYNC_MODE */
    LL_MRSubG_Force2FSKSync(ENABLE);
  }

  /* special Sync settings if needed */
  /* case C-mode & Format B */
  if ((wMBus_Format == WMBUS_FORMAT_B) && (wMBus_mode == C_MODE))
  {
    wMBus_Phy_PacketSettingsStruct.SyncWord = WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B;
  }

  wMBus_Phy_PacketSettingsStruct.FixVarLength = FIXED;
  wMBus_Phy_PacketSettingsStruct.PreambleSequence = PRE_SEQ_0101;
  wMBus_Phy_PacketSettingsStruct.PostambleSequence = POST_SEQ_0101;
  wMBus_Phy_PacketSettingsStruct.CrcMode = PKT_NO_CRC;
  wMBus_Phy_PacketSettingsStruct.Coding = (MRSubG_PcktCoding)wMBus_Phy_ChannelCoding[wMBus_mode][wMBus_Direction];
  /* set Manchester type1 if needed */
  if (wMBus_Phy_PacketSettingsStruct.Coding == CODING_MANCHESTER)
  {
    LL_MRSubG_PacketHandlerManchesterType(MANCHESTER_TYPE1);
  }
  wMBus_Phy_PacketSettingsStruct.DataWhitening = DISABLE;
  wMBus_Phy_PacketSettingsStruct.LengthWidth = BYTE_LEN_1;
  wMBus_Phy_PacketSettingsStruct.SyncPresent = ENABLE;

  /* in that case - ONLY FOR GATEWAY - special settings to be used */
#ifdef T1C1_ACTIVATED
  if ((wMBus_mode == C_MODE) || (wMBus_mode == T_MODE))
  {
    /* only Rx Sync settings to change */
    /* use different Sync pattern to cope with T1 AND C1 */
    wMBus_Phy_PacketSettingsStruct.SyncWord = WMBUS_SYNC_PATTERN_T1C1_MODE_M2O_GATEWAY;
    wMBus_Phy_PacketSettingsStruct.SyncLength = WMBUS_SYNC_LENGTH_T1C1_MODE_M2O_GATEWAY - 1;

    /* force NRZ decoding */
    wMBus_Phy_PacketSettingsStruct.Coding = CODING_NONE;
  }
#endif

  /* specific for 169MHz wM-Bus */
#ifdef STM32WL33XA
  /* if N2g mode, specific configuration for 4-GFSK05 */
  if (wMBus_mode == N_MODE_Ng)
  {
    /* specific configuration for preamble pattern : set LL_MRSubG_SetPreambleSeq : 4-GFSK 0111 pattern */
    LL_MRSubG_SetPreambleSeq(0);

    /* specific configuration for Sync pattern : need to force 2-FSK SYnc pattern : set FORCE_2FSK_SYNC_MODE */
    LL_MRSubG_Force2FSKSync(ENABLE);
  }
#endif

  HAL_MRSubG_PacketBasicInit(&wMBus_Phy_PacketSettingsStruct);

  /* set maximum packet length to avoid corrupting Radio buffer*/
  HAL_MRSubG_PktBasicSetPayloadLength(WMBUS_RADIO_BUFFER_SIZE);

  /* configure Output Power */
  HAL_MRSubG_SetPALeveldBm(7, wMBus_Phy_RadioInitStruct.outputPower, wMBus_Phy_RadioInitStruct.PADrvMode);

  /* check back */
  wMBus_Phy_RadioInitStruct.outputPower = HAL_MRSubG_GetPALeveldBm();

#ifdef WMBUS_RX_PERFORMANCE_ENABLED
  /* activate Rx performance settings - may depend on wM-Bus mode */
  wMBus_Phy_Rx_performance_settings(wMBus_mode, wMBus_Direction);
#endif

  HAL_NVIC_EnableIRQ(MRSUBG_IRQn);

  HAL_NVIC_ClearPendingIRQ(MRSUBG_IRQn);

  /* clear IRQ flags */
  __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(0xffffffff);

  /* Set the pointer to the data buffer */
  __HAL_MRSUBG_SET_DATABUFFER0_POINTER((uint32_t)&vectcRadioBuff);

  /* memorize frame format */
  wMBus_Format_global = wMBus_Format;
  /* memorize wM-Bus mode */
#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE) || defined(AUTOMATIC_FORMAT_DETECTION_C_MODE)
  wMBus_mode_global = wMBus_mode;
#endif
}

void wMBus_Phy_prepare_Tx_CRC_mngt(uint8_t *wMBus_TxBuffer, uint16_t wMBusTxBuffer_size, uint8_t FrameFormat)
{
  /* IRQ Config */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_TX_DONE_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SABORT_DONE_E);

  /* should align to Phy layer buffer which is 32-bit aligned : vectcRadioBuff */
  uint16_t i = 0;
  uint16_t radio_buffer_index = 0;
  uint8_t dataBytesRemaining;
  uint8_t blockDataSize;
  uint8_t *local_wMBus_Buffer_ptr = wMBus_TxBuffer;

  /* wMBusTxBuffer_size is buffer size without CRCs */
  dataBytesRemaining = wMBusTxBuffer_size;

  if (FrameFormat == WMBUS_FORMAT_A)
  {
    /* copy 1st block data into Radio buffer */
    for (i = 0; i < (WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2); i++)
    {
      vectcRadioBuff[i] = *(local_wMBus_Buffer_ptr);
      local_wMBus_Buffer_ptr++;
    }

    /* manage CRC of 1st block : After adding 2 byte CRC */
    wMBus_Phy_CRC_append(&vectcRadioBuff[0], &vectcRadioBuff[i]);
    /* update vectcRadioBuff index */
    i = i + 2;

    /* update remaining bytes */
    dataBytesRemaining = dataBytesRemaining - (WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2);

    /* Copy 0-16 bytes of data in the second block : determine nb of bytes to copy */
    if (dataBytesRemaining <= (WMBUS_BLOCK_SIZE_FORMAT_A - 2))
    {
      blockDataSize = dataBytesRemaining;
    }
    else
    {
      blockDataSize = (WMBUS_BLOCK_SIZE_FORMAT_A - 2);
    }

    /* copy 2nd block data into Radio buffer */
    for (i = WMBUS_FIRST_BLOCK_SIZE_FORMAT_A; i < (WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + blockDataSize); i++)
    {
      vectcRadioBuff[i] = *(local_wMBus_Buffer_ptr);
      local_wMBus_Buffer_ptr++;
    }

    /* manage CRC of 2nd block : After adding 2 byte CRC */
    wMBus_Phy_CRC_append(&vectcRadioBuff[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A], &vectcRadioBuff[i]);
    /* update vectcRadioBuff index */
    i = i + 2;

    /* update remaining bytes */
    dataBytesRemaining = dataBytesRemaining - blockDataSize;

    /* Assemble optional blocks */
    while (dataBytesRemaining)
    {
      /* ease index management */
      radio_buffer_index = i;

      /* Copy 0-16 bytes of data into the next block */
      if (dataBytesRemaining <= (WMBUS_BLOCK_SIZE_FORMAT_A - 2))
      {
        blockDataSize = dataBytesRemaining;
      }
      else
      {
        blockDataSize = (WMBUS_BLOCK_SIZE_FORMAT_A - 2);
      }

      /* Data-field (Data) - Application payload */
      /* copy nth block data into Radio buffer */
      for (i = radio_buffer_index; i < radio_buffer_index + blockDataSize; i++)
      {
        vectcRadioBuff[i] = *(local_wMBus_Buffer_ptr);
        local_wMBus_Buffer_ptr++;
      }

      /* manage CRC of 2nd block : After adding 2 byte CRC */
      wMBus_Phy_CRC_append(&vectcRadioBuff[radio_buffer_index], &vectcRadioBuff[i]);

      /* update vectcRadioBuff index */
      i = i + 2;

      dataBytesRemaining -= blockDataSize;
    }
  }
  else if (FrameFormat == WMBUS_FORMAT_B)
  {
    /* adapt L-field according to B format : without CRC for the time being...*/
    /* depends on the size of wM-Bus packet : either +2 CRC bytes or +4 */
    if (wMBusTxBuffer_size <= WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + WMBUS_SECOND_BLOCK_SIZE_FORMAT_B - 2)
    {
      vectcRadioBuff[0] = wMBusTxBuffer_size - 1 + 2;
    }
    else
    {
      vectcRadioBuff[0] = wMBusTxBuffer_size - 1 + 4;
    }

    /* skit first byte which is supposed to be L-field */
    local_wMBus_Buffer_ptr++;

    /* copy 1st block data into Radio buffer */
    for (i = 1; i < (WMBUS_FIRST_BLOCK_SIZE_FORMAT_B); i++)
    {
      vectcRadioBuff[i] = *(local_wMBus_Buffer_ptr);
      local_wMBus_Buffer_ptr++;
    }

    /* no CRC in 1st block - Format B */

    /* update remaining bytes */
    dataBytesRemaining = dataBytesRemaining - (WMBUS_FIRST_BLOCK_SIZE_FORMAT_B);

    /* Copy 0-116 bytes of data in the second block : determine nb of bytes to copy */
    if (dataBytesRemaining <= (WMBUS_SECOND_BLOCK_SIZE_FORMAT_B - 2))
    {
      blockDataSize = dataBytesRemaining;
    }
    else
    {
      blockDataSize = (WMBUS_SECOND_BLOCK_SIZE_FORMAT_B - 2);
    }

    /* copy 2nd block data into Radio buffer */
    for (i = WMBUS_FIRST_BLOCK_SIZE_FORMAT_B; i < (WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + blockDataSize); i++)
    {
      vectcRadioBuff[i] = *(local_wMBus_Buffer_ptr);
      local_wMBus_Buffer_ptr++;
    }

    /* manage CRC of 2nd block : After adding 2 byte CRC */
    wMBus_Phy_CRC_append(&vectcRadioBuff[0], &vectcRadioBuff[i]);
    /* update vectcRadioBuff index */
    i = i + 2;

    /* update remaining bytes */
    dataBytesRemaining = dataBytesRemaining - blockDataSize;

    /* Assemble optional blocks */
    while (dataBytesRemaining)
    {
      /* ease index management */
      radio_buffer_index = i;

      /* Copy 0-126 bytes of data into the next block */
      if (dataBytesRemaining <= (WMBUS_OPTIONAL_BLOCK_SIZE_FORMAT_B - 2))
      {
        blockDataSize = dataBytesRemaining;
      }
      else
      {
        blockDataSize = (WMBUS_OPTIONAL_BLOCK_SIZE_FORMAT_B - 2);
      }

      /* Data-field (Data) - Application payload */
      /* copy nth block data into Radio buffer */
      for (i = radio_buffer_index; i < radio_buffer_index + blockDataSize; i++)
      {
        vectcRadioBuff[i] = *(local_wMBus_Buffer_ptr);
        local_wMBus_Buffer_ptr++;
      }

      /* manage CRC of 2nd block : After adding 2 byte CRC */
      wMBus_Phy_CRC_append(&vectcRadioBuff[radio_buffer_index], &vectcRadioBuff[i]);

      /* update vectcRadioBuff index */
      i = i + 2;

      dataBytesRemaining -= blockDataSize;
    }
  }
  /* specify Tx mode Normal */
  LL_MRSubG_SetTXMode(TX_NORMAL);

  /* ease index management */
  radio_buffer_index = i;

  /* do not forget to specify packet length as using packet handler */
  HAL_MRSubG_PktBasicSetPayloadLength(radio_buffer_index);
}

void wMBus_Phy_prepare_Rx(void)
{
  /* IRQ Config */
  uint32_t tmpReg = 0x00;
  tmpReg |= MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_OK_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_ALMOST_FULL_0_E | MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SABORT_DONE_E;

#ifdef PREAMBLE_AND_SYNC_IRQ_ENABLE
  tmpReg |= MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SYNC_VALID_E;

  /* by default set to 8 bits preamble verification */
  __HAL_MRSUBG_SET_PQI_THRESHOLD(0x02);

  tmpReg |= MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_PREAMBLE_VALID_E;
#endif
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(tmpReg);

  /* specify Rx mode Normal */
  LL_MRSubG_SetRXMode(RX_NORMAL);

  /* do not forget to specify maximum Rx packet length as using packet handler */
  HAL_MRSubG_PktBasicSetPayloadLength(WMBUS_RADIO_BUFFER_SIZE);

#ifndef WMBUS_FIRST_CRC_CHECK
  /* initialize Rx buffer threshold to xx bytes - size of max of 1st block size :format A */
  LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_BLOCK_SIZE_FORMAT_A);
#else
  /* initialize Rx buffer threshold to xx bytes - size of max number of bytes to include CRC - format A */
  LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
#endif

#if defined(T1C1_ACTIVATED_C1_MODE)
  if (wMBus_mode_global == C_MODE)
  {
    /* only Rx Sync settings to change */
    /* use different Sync pattern to cope with T1 AND C1 */
    LL_MRSubG_SetSyncWord(WMBUS_SYNC_PATTERN_T1C1_MODE_M2O_GATEWAY);
    LL_MRSubG_SetSyncLength(WMBUS_SYNC_LENGTH_T1C1_MODE_M2O_GATEWAY - 1);

    /* force NRZ decoding */
    LL_MRSubG_PacketHandlerCoding(CODING_NONE);
  }
#endif
#if defined(T1C1_ACTIVATED)
  if ((wMBus_mode_global == C_MODE) || (wMBus_mode_global == T_MODE))
  {
    /* only Rx Sync settings to change */
    /* use different Sync pattern to cope with T1 AND C1 */
    LL_MRSubG_SetSyncWord(WMBUS_SYNC_PATTERN_T1C1_MODE_M2O_GATEWAY);
    LL_MRSubG_SetSyncLength(WMBUS_SYNC_LENGTH_T1C1_MODE_M2O_GATEWAY - 1);

    /* force NRZ decoding */
    LL_MRSubG_PacketHandlerCoding(CODING_NONE);
  }
#endif

#ifdef AUTOMATIC_FORMAT_DETECTION_C_MODE
  /* check if C-mode Rx is activated and enable dynamic frame format reception */
  if (wMBus_mode_global == C_MODE)
  {
    switch (wMBus_Format_global)
    {
    /* frame format is undefined */
    case WMBUS_FORMAT_A_B:

      /* check if T1C1_ACTIVATED_C1_MODE is activated */
#ifdef T1C1_ACTIVATED_C1_MODE

      /* in that case activate different C-mode Sync pattern & length */
      LL_MRSubG_SetSyncWord(WMBUS_SYNC_PATTERN_T1C1_MODE_M2O_GATEWAY);
      LL_MRSubG_SetSyncLength(WMBUS_SYNC_LENGTH_T1C1_MODE_M2O_GATEWAY - 1);

#else

      /* in that case activate different C-mode Sync pattern & length */
      LL_MRSubG_SetSyncWord(WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_UNDEFINED);
      LL_MRSubG_SetSyncLength(WMBUS_SYNC_LENGTH_C_MODE_M2O_FORMAT_UNDEFINED - 1);
#endif

      break;

    case WMBUS_FORMAT_A:
    case WMBUS_FORMAT_B:

      /* nothing to do - already done */
      break;

    default:

      break;
    }
  }

#endif

  /* initialize Rx state machine */
  xRxState = SM_RX_NEW_PACKET;

  /* force automatic Rx */
  wMBus_Auto_Rx = 1;
}

uint8_t wMBus_Phy_start_transmission(void)
{
  __HAL_MRSUBG_STROBE_CMD(CMD_TX);
#ifdef WMBUS_DEBUG
  wMBus_Phy_trace_debug_log(DEBUG_COMMAND_TX_LOG, 0, 0, 0);
#endif

  return SUCCESS;
}

uint8_t wMBus_Phy_start_continuousRx(void)
{
  /* force automatic Rx */
  wMBus_Auto_Rx = 1;

#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
  /* Call this API before RX */
  adjustFrequencyBaseFor169();
#endif
  /* Restart RX */
  FEM_Operation(FEM_RX);
#endif

  __HAL_MRSUBG_STROBE_CMD(CMD_RX);
#ifdef WMBUS_DEBUG
  wMBus_Phy_trace_debug_log(DEBUG_COMMAND_RX_CONTINUOUS_LOG, 0, 0, 0);
#endif

  return SUCCESS;
}

uint8_t wMBus_Phy_start_RxTimer(uint32_t RxTimeoutMicrosec, uint8_t RxTimeoutStopCondition)
{
  uint32_t RxTimerReg = 0;

  /* force automatic Rx */
  wMBus_Auto_Rx = 0;

  /* compute Rx timer value */
  RxTimerReg = HAL_MRSubG_Sequencer_Microseconds(RxTimeoutMicrosec);

  /* write Rx timer Register */
  __HAL_MRSUBG_SET_RX_TIMEOUT(RxTimerReg);

  /* set Stop timeout conditions */
  if ((RxTimeoutStopCondition & RX_CS_TIMEOUT_MASK) == RX_CS_TIMEOUT_MASK)
  {
    LL_MRSubG_SetRxCsTimeout(ENABLE);
  }
  else
  {
    LL_MRSubG_SetRxCsTimeout(DISABLE);
  }

  if ((RxTimeoutStopCondition & RX_PQI_TIMEOUT_MASK) == RX_PQI_TIMEOUT_MASK)
  {
    LL_MRSubG_SetRxPqiTimeout(ENABLE);
  }
  else
  {
    LL_MRSubG_SetRxPqiTimeout(DISABLE);
  }

  if ((RxTimeoutStopCondition & RX_SQI_TIMEOUT_MASK) == RX_SQI_TIMEOUT_MASK)
  {
    LL_MRSubG_SetRxSqiTimeout(ENABLE);
  }
  else
  {
    LL_MRSubG_SetRxSqiTimeout(DISABLE);
  }

  if ((RxTimeoutStopCondition & RX_OR_nAND_SELECT) == RX_OR_nAND_SELECT)
  {
    LL_MRSubG_SetRxOrnAndSelect(SET);
  }
  else
  {
    LL_MRSubG_SetRxOrnAndSelect(RESET);
  }

#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
  /* Call this API before RX */
  adjustFrequencyBaseFor169();
#endif
  /* Restart RX */
  FEM_Operation(FEM_RX);
#endif

  __HAL_MRSUBG_STROBE_CMD(CMD_RX);
#ifdef WMBUS_DEBUG
  wMBus_Phy_trace_debug_log(DEBUG_COMMAND_RX_TIMER_LOG, 0, 0, 0);
#endif

  return SUCCESS;
}

void wMBus_Phy_abort(void)
{
  /* disable automatic Rx */
  wMBus_Auto_Rx = 0;

  /* send sabort command to Radio IP */
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
  wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

  /* here need a smarter management :
     - either wait for Sabort Flag - if compilation switch activated
     - or check Radio FSM (already) in Idle mode
  */

  /* wait for SABORT IRQ confirmation or Radio FSM state Idle */
  while ((!xSabortCompletedFlag) && (LL_MRSubG_GetRadioFSMState() != STATE_IDLE));

  xSabortCompletedFlag = RESET;
}

#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE)
/**
  * @brief  Decode T1+C1 mode by Sw - gateway only case.
  * @note   This function is defined when T1C1_ACTIVATED or T1C1_ACTIVATED_C1_MODE is defined.
  * @param  tos_in Pointer to the input buffer.
  * @param  tos_out Pointer to the output buffer.
  * @retval uint8_t size of the output buffer.
  */
static uint8_t ToS_decode(uint8_t *tos_in, uint8_t *tos_out)
{
  uint8_t nibble;
  uint16_t semioct;
  uint8_t size = 0;

  tos_out[0] = 0;
  tos_out[1] = 0;

  for (uint8_t j = 0; j < 4; j++)
  {
    switch (j)
    {
    case 0:
      nibble = tos_in[0] >> 2;
      break;
    case 1:
      nibble = ((tos_in[0] & 0x03) << 4) + (tos_in[1] >> 4);
      break;
    case 2:
      nibble = ((tos_in[1] & 0x0F) << 2) + (tos_in[2] >> 6);
      break;
    case 3:
      nibble = tos_in[2] & 0x3F;
      break;
    }

    switch (nibble)
    {
    case 22:
      semioct = 0;
      break;
    case 13:
      semioct = 1;
      break;
    case 14:
      semioct = 2;
      break;
    case 11:
      semioct = 3;
      break;
    case 28:
      semioct = 4;
      break;
    case 25:
      semioct = 5;
      break;
    case 26:
      semioct = 6;
      break;
    case 19:
      semioct = 7;
      break;
    case 44:
      semioct = 8;
      break;
    case 37:
      semioct = 9;
      break;
    case 38:
      semioct = 10;
      break;
    case 35:
      semioct = 11;
      break;
    case 52:
      semioct = 12;
      break;
    case 49:
      semioct = 13;
      break;
    case 50:
      semioct = 14;
      break;
    case 41:
      semioct = 15;
      break;
    default:
      return size;
    }

    tos_out[j >> 1] |= (semioct << (4 * ((j % 2) == 0)));

    if (j == 1 || j == 3) size++;
  }

  return size;
}

/* Static variables to be used in wM-Bus Rx IRQ handler to decode T-mode by Sw using T1C1_ACTIVATED */
static uint16_t tos_q_head = 0, tos_q_tail = 0, tos_q_done_head = 0;
uint8_t tos_out[2], tos_in[3];
#endif

void wMBus_Phy_IRQHandler(void)
{
  uint32_t irq;

  irq = __HAL_MRSUBG_GET_RFSEQ_IRQ_STATUS();

  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F)
  {
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F);

#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_IRQ_TX_DONE_LOG, 0, 0, 0);
#endif

    wMBus_Radio_Event_bitmap |= WMBUS_TX_COMPLETED;
  }

  /* check SABORT DONE IRQ */
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F)
  {
    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SABORT_DONE_F);

#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_IRQ_ABORT_LOG, wMBus_Auto_Rx, 0, 0);
#endif

    if (wMBus_Auto_Rx == 1)
    {
#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
      /* Call this API after RX */
      revertFrequencyBaseFor169();
#endif
#endif

      /* initialize Rx state machine */
      xRxState = SM_RX_NEW_PACKET;

#ifndef WMBUS_FIRST_CRC_CHECK
      /* initialize Rx buffer threshold to xx bytes - size of max of 1st block size :format A */
      LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_BLOCK_SIZE_FORMAT_A);
#else
      /* initialize Rx buffer threshold to xx bytes - size of max number of bytes to include CRC - format A */
      LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
#endif

#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
      /* Call this API before RX */
      adjustFrequencyBaseFor169();
#endif
      /* Restart RX */
      FEM_Operation(FEM_RX);
#endif

      /* if sabort done then issue Rx command */
      __HAL_MRSUBG_STROBE_CMD(CMD_RX);
#ifdef WMBUS_DEBUG
      wMBus_Phy_trace_debug_log(DEBUG_COMMAND_RX_CONTINUOUS_LOG, 0, 0, 0);
#endif
    }
    else
    {
      /* indicate flag status */
      xSabortCompletedFlag = SET;
    }
  }

  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F);

#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
    /* Call this API after RX */
    revertFrequencyBaseFor169();
#endif
#endif

#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_IRQ_RX_TIMER_LOG, 0, 0, 0);
#endif

    wMBus_Radio_Event_bitmap |= WMBUS_RX_TIMEOUT;
  }

  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F)
  {
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_IRQ_RX_FIFO_LOG, 0, 0, 0);
#endif
    /* flag to set Sabort command to issue as multiple Sabort cases to handle */

    if (xRxState == SM_RX_NEW_PACKET)
    {
      RssiOnSync = LL_MRSubG_GetRssiLevelOnSync();
      /* save Sync pattern - upper layer purpose */
      RxSync_Ongoing = LL_MRSubG_GetSyncWord();

#if !defined(T1C1_ACTIVATED) && !defined(T1C1_ACTIVATED_C1_MODE)
#ifndef AUTOMATIC_FORMAT_DETECTION_C_MODE
      /* extract L-field */
      cLField = vectcRadioBuff[0];

      if (wMBus_Format_global == WMBUS_FORMAT_A)
      {
        nPcktLength = 1 + cLField + 2 + 2 * (CEILING(((float)cLField - 9) / 16));
        *(global_buffer_format_ptr) = WMBUS_FORMAT_A;
      }
      if (wMBus_Format_global == WMBUS_FORMAT_B)
      {
        nPcktLength = 1 + cLField;
        *(global_buffer_format_ptr) = WMBUS_FORMAT_B;
      }
#else // #ifndef AUTOMATIC_FORMAT_DETECTION_C_MODE
      /* manage dynamic C-mode detection */
      /* to be checked which C-mode frame format it is */
      /* only if C-mode is activated and frame format WMBUS_FORMAT_A_B (undefined) */
      if ((wMBus_mode_global == C_MODE) && (wMBus_Format_global == WMBUS_FORMAT_A_B))
      {
        if (vectcRadioBuff[0] == (WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A & 0x000000FF))
        {
          RxSync_Ongoing = WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A;
          *(global_buffer_format_ptr) = WMBUS_FORMAT_A;
        }
        else if (vectcRadioBuff[0] == (WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B & 0x000000FF))
        {
          RxSync_Ongoing = WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B;
          *(global_buffer_format_ptr) = WMBUS_FORMAT_B;
        }
        else
        {
          /* here there is an error and we should stop reception */
          /* Clear the IRQ flag */
          __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);

#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_C_MODE, vectcRadioBuff[0], 0, 0);
#endif

          __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

          wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_HEADER_ERROR;

          return;
        }

        /* here L-field is 2nd byte of Radio buffer - shifted by 1 */
        cLField = vectcRadioBuff[1];
        if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A)
        {
          nPcktLength = 1 + cLField + 2 + 2 * (CEILING(((float)cLField - 9) / 16));
        }
        if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B)
        {
          nPcktLength = 1 + cLField;
        }
      }
      else if ((wMBus_mode_global != C_MODE) || ((wMBus_mode_global == C_MODE) && (wMBus_Format_global != WMBUS_FORMAT_A_B)))
      /* here this normal case to handle : other than C-mode */
      {
        /* extract L-field */
        cLField = vectcRadioBuff[0];
        if (wMBus_Format_global == WMBUS_FORMAT_A)
        {
          nPcktLength = 1 + cLField + 2 + 2 * (CEILING(((float)cLField - 9) / 16));
          *(global_buffer_format_ptr) = WMBUS_FORMAT_A;
        }
        if (wMBus_Format_global == WMBUS_FORMAT_B)
        {
          nPcktLength = 1 + cLField;
          *(global_buffer_format_ptr) = WMBUS_FORMAT_B;
        }
      }
#endif
      /* L should be at least 10 or 11 depending frame format, otherwise abort */
      if (((cLField < 10) && (wMBus_Format_global == WMBUS_FORMAT_A)) || ((cLField < 11) && (wMBus_Format_global == WMBUS_FORMAT_B)))
      {
        /* Clear the IRQ flag before return*/
        __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);

#ifdef WMBUS_DEBUG
        wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_L_FIELD, cLField, wMBus_Format_global, 0);
#endif

        __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
        wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

        wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_HEADER_ERROR;
        return;
      }
      else
      {
#ifdef WMBUS_FIRST_CRC_CHECK
        if (!wMBus_Phy_first_CRC_check())
        {
          /* if 1st CRC check is failed then stop Rx ASAP */
          /* Clear the IRQ flag before return*/
          __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_FIRST_CRC_ERROR, cLField, wMBus_Format_global, 0);
#endif
          __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

          wMBus_Radio_Event_bitmap |= WMBUS_FIRST_CRC_ERROR;
          return;
        }
        else
        {
#endif
#ifndef AUTOMATIC_FORMAT_DETECTION_C_MODE
          /* update 2nd Rx buffer threshold */
#ifdef WMBUS_FIRST_CRC_CHECK
          if (nPcktLength < WMBUS_FIRST_RXBUFFER_THRESHOLD)
          {
            LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
          }
          else
          {
            LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength);
          }
#else
          LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength);
#endif
#else
        /* AUTOMATIC_FORMAT_DETECTION_C_MODE activated but check if C-mode & WMBUS_FORMAT_A_B */
        if ((wMBus_mode_global == C_MODE) && (wMBus_Format_global == WMBUS_FORMAT_A_B))
        {
#ifdef WMBUS_FIRST_CRC_CHECK
          if (nPcktLength + 1 < WMBUS_FIRST_RXBUFFER_THRESHOLD)
          {
            LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
          }
          else
          {
            LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength + 1);
          }
#else
          LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength + 1);
#endif
        }
        else
        {
#ifdef WMBUS_FIRST_CRC_CHECK
          if (nPcktLength < WMBUS_FIRST_RXBUFFER_THRESHOLD)
          {
            LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
          }
          else
          {
            LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength);
          }
#else
          LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength);
#endif
        }
#endif
          xRxState = SM_RX_LAST;

#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_END_RX_SM_FISRT_PROCESSING, (uint32_t)nPcktLength, (uint32_t)RxSync_Ongoing, (uint32_t)RssiOnSync);
#endif
#ifdef WMBUS_FIRST_CRC_CHECK
        }
#endif
      }
#else /* T1C1 ACTIVATED or T1C1 ACTIVATED_C1_MODE*/
      /* to be checked whether C-Sync 3rd byte (0x54) or 3o6 T-mode byte */

      /* check T or C mode requested by application (as other modes can be also used with that Sw configuration) */
      if ((wMBus_mode_global == C_MODE) || (wMBus_mode_global == T_MODE))
      {
        if (vectcRadioBuff[0] == ((WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A >> 8) & 0x000000FF))
        {
          /* to be checked which C-mode frame format it is */
          if (vectcRadioBuff[1] == (WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A & 0x000000FF))
          {
            RxSync_Ongoing = WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A;
            *(global_buffer_format_ptr) = WMBUS_FORMAT_A;
          }
          else if (vectcRadioBuff[1] == (WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B & 0x000000FF))
          {
            RxSync_Ongoing = WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B;
            *(global_buffer_format_ptr) = WMBUS_FORMAT_B;
          }
          else
          {
            /* here there is an error and we should stop reception */
            /* Clear the IRQ flag */
            __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);

#ifdef WMBUS_DEBUG
            wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_C_MODE, ((vectcRadioBuff[0] << 8) | vectcRadioBuff[1]), 0, 0);
#endif

            __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
            wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

            wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_HEADER_ERROR;
            return;
          }
        }
        else
        /* here it is supposed to be T-mode */
        {
          /* saved Sync word (T-mode) */
          RxSync_Ongoing = WMBUS_SYNC_PATTERN_T_MODE_M2O;
          *(global_buffer_format_ptr) = wMBus_Format_global;
        }

        /* now we have to check L-field consistency */

        /* decode L-field using 3o6 Sw decoding */
        if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_T_MODE_M2O)
        {
          uint8_t tos_in[3], tos_out[2];

          for (uint8_t j = 0; j < 3; j++)
          {
            tos_in[j] = vectcRadioBuff[j];
          }

          ToS_decode(tos_in, tos_out);
          cLField = tos_out[0];
        }
        else
        {
          /* in that case C-mode and NRZ encoded - L-field is in 3rd byte - 2 1st bytes are Sync pattern */
          cLField = vectcRadioBuff[2];
        }

        /* L should be at least 10 (C-field+M-field+A-field+CI-field),
        if not Sabort Rx */
        if (cLField < 10)
        {
          /* Clear the IRQ flag */
          __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_L_FIELD, cLField, RxSync_Ongoing, 0);
#endif
          __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

          wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_HEADER_ERROR;
          return;
        }

        /* now need to compute length of wM-Bus packet / buffer */
        /* as T1C1_ACTIVATED enabled we don't know frame format in case C-mode => we have to check it based on RxSync_Ongoing info */
        if ((RxSync_Ongoing == WMBUS_SYNC_PATTERN_T_MODE_M2O) && (wMBus_Format_global == WMBUS_FORMAT_A))
        {
          nPcktLength = 1 + cLField + 2 + 2 * (CEILING(((float)cLField - 9) / 16));
        }
        else if ((RxSync_Ongoing == WMBUS_SYNC_PATTERN_T_MODE_M2O) && (wMBus_Format_global == WMBUS_FORMAT_B))
        {
          nPcktLength = 1 + cLField;
        }
        else if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A)
        {
          nPcktLength = 1 + cLField + 2 + 2 * (CEILING(((float)cLField - 9) / 16));
        }
        else if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B)
        {
          nPcktLength = 1 + cLField;
        }

        /* here we have to take care 3o6 case where radio buffer is 50% longer */
        if ((RxSync_Ongoing == WMBUS_SYNC_PATTERN_T_MODE_M2O))
        {
          nPcktLengthRaw = nPcktLength * 3;

          if (nPcktLengthRaw % 2 == 0)
            nPcktLengthRaw /= 2;
          else
            nPcktLengthRaw = (nPcktLengthRaw + 1) / 2;

#ifdef WMBUS_FIRST_CRC_CHECK
          if (nPcktLengthRaw < WMBUS_FIRST_RXBUFFER_THRESHOLD)
          {
            LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
          }
          else
          {
            LL_MRSubG_SetAlmostFullThresholdRx(nPcktLengthRaw);
          }
#else
          LL_MRSubG_SetAlmostFullThresholdRx(nPcktLengthRaw);
#endif
        }
        else /* default case for nPcktLength buffer IRQ */
        {
          /* update 2nd Rx buffer threshold with +2 as 2 1st bytes are C-mode Sync */
#ifdef WMBUS_FIRST_CRC_CHECK
          if (nPcktLength + 2 < WMBUS_FIRST_RXBUFFER_THRESHOLD)
          {
            LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
          }
          else
          {
            LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength + 2);
          }
#else
          LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength + 2);
#endif
        }
#ifdef WMBUS_FIRST_CRC_CHECK
        if (!wMBus_Phy_first_CRC_check())
        {
          /* if 1st CRC check is failed then stop Rx ASAP */

          /* Clear the IRQ flag before return*/
          __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);

#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_FIRST_CRC_ERROR, cLField, wMBus_Format_global, 0);
#endif

          __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

          wMBus_Radio_Event_bitmap |= WMBUS_FIRST_CRC_ERROR;
          return;
        }
        else
        {
#endif

          /* update 2nd Rx buffer threshold */
          xRxState = SM_RX_LAST;
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_END_RX_SM_FISRT_PROCESSING, nPcktLength, RxSync_Ongoing, RssiOnSync);
#endif
#ifdef WMBUS_FIRST_CRC_CHECK
        }
#endif

      } /*end if ((wMBus_mode_global == C_MODE) || (wMBus_mode_global == T_MODE)) */
      /* this case can happen */
      else if ((wMBus_mode_global == S1M_MODE) || (wMBus_mode_global == S_MODE))
      {
        /* extract L-field */
        cLField = vectcRadioBuff[0];

        if (wMBus_Format_global == WMBUS_FORMAT_A)
        {
          nPcktLength = 1 + cLField + 2 + 2 * (CEILING(((float)cLField - 9) / 16));
          *(global_buffer_format_ptr) = WMBUS_FORMAT_A;
        }
        if (wMBus_Format_global == WMBUS_FORMAT_B)
        {
          nPcktLength = 1 + cLField;
          *(global_buffer_format_ptr) = WMBUS_FORMAT_B;
        }

        /* L should be at least 10 or 11 depending frame format, otherwise abort */
        if (((cLField < 10) && (wMBus_Format_global == WMBUS_FORMAT_A)) || ((cLField < 11) && (wMBus_Format_global == WMBUS_FORMAT_B)))
        {
          /* Clear the IRQ flag before return*/
          __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);

#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_L_FIELD, cLField, wMBus_Format_global, 0);
#endif

          __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
          wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

          wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_HEADER_ERROR;
          return;
        }
        else
        {

#ifdef WMBUS_FIRST_CRC_CHECK
          if (!wMBus_Phy_first_CRC_check())
          {
            /* if 1st CRC check is failed then stop Rx ASAP */

            /* Clear the IRQ flag before return*/
            __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);

#ifdef WMBUS_DEBUG
            wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_FIRST_CRC_ERROR, cLField, wMBus_Format_global, 0);
#endif

            __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
            wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

            wMBus_Radio_Event_bitmap |= WMBUS_FIRST_CRC_ERROR;
            return;
          }
          else
          {
#endif

            /* update 2nd Rx buffer threshold */
#ifdef WMBUS_FIRST_CRC_CHECK
            if (nPcktLength < WMBUS_FIRST_RXBUFFER_THRESHOLD)
            {
              LL_MRSubG_SetAlmostFullThresholdRx(WMBUS_FIRST_RXBUFFER_THRESHOLD);
            }
            else
            {
              LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength);
            }
#else
          LL_MRSubG_SetAlmostFullThresholdRx(nPcktLength);
#endif

            xRxState = SM_RX_LAST;

#ifdef WMBUS_DEBUG
            wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_END_RX_SM_FISRT_PROCESSING, (uint32_t)nPcktLength, (uint32_t)RxSync_Ongoing, (uint32_t)RssiOnSync);
#endif
#ifdef WMBUS_FIRST_CRC_CHECK
          }
#endif
        }
      }
#endif
    }
    else if (xRxState == SM_RX_LAST)
    {
      /* stop Rx - all packet bytes have been received */
      xRxDoneFlag = SET;

#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE)
      /* here need to re-format RxBuffer to be compliant with
      wMBus_Phy_wait_Rx_CRC_mngt()
      => using same Radio buffer*/

      if ((wMBus_mode_global == C_MODE) || (wMBus_mode_global == T_MODE))
      {
        uint16_t i = 0;

        /* T1C1 C-mode case : copy by shifting bytes by -2 */
        if ((RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A) || (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B))
        {
          /* nPcktLength NRZ bytes to copy */
          for (i = 0; i < nPcktLength; i++)
          {
            vectcRadioBuff_copy[i] = vectcRadioBuff[i + 2];
          }
        }

        /* T1C1 T-mode case : 3o6 decoding + copy (into same buffer) */
        else if ((RxSync_Ongoing == WMBUS_SYNC_PATTERN_T_MODE_M2O))
        {
          /* 3o6 decoding until nPcktLengthRaw */
          uint16_t tos_q_tgt = nPcktLengthRaw;

          tos_q_head = 0; // 1st 3o6 RAW byte to decode
          tos_q_tail = 0; // 1st decoded byte to write

          /* indexes of RAW 3o6 buffer */
          tos_q_done_head = 0;

          while (tos_q_done_head < tos_q_tgt)
          {
            ToS_decode(&vectcRadioBuff[tos_q_head], tos_out);

            tos_q_head = tos_q_head + 3;
            tos_q_done_head += 3;

            for (uint8_t j = 0; j < 2; j++)
            {
              vectcRadioBuff_copy[tos_q_tail] = tos_out[j];
              tos_q_tail++;
            }
          }
        }
        else /* default case : simply copy Radio buffer as is */
        {
          /* nPcktLength NRZ bytes to copy */
          for (i = 0; i < nPcktLength; i++)
          {
            vectcRadioBuff_copy[i] = vectcRadioBuff[i];
          }
        }

      } /* end if ((wMBus_mode_global == C_MODE) || (wMBus_mode_global == T_MODE)) */
      else if ((wMBus_mode_global == S1M_MODE) || (wMBus_mode_global == S_MODE))
      {
        uint16_t i = 0;

        /* nPcktLength NRZ bytes to copy */
        for (i = 0; i < nPcktLength; i++)
        {
          vectcRadioBuff_copy[i] = vectcRadioBuff[i];
        }
      }
#else /* T1C1_ACTIVATED */

      uint16_t i = 0;

#ifndef AUTOMATIC_FORMAT_DETECTION_C_MODE
      /* nPcktLength NRZ bytes to copy */
      for (i = 0; i < nPcktLength; i++)
      {
        vectcRadioBuff_copy[i] = vectcRadioBuff[i];
      }
#else
      /* nPcktLength NRZ bytes to copy */
      /* we have to verify which mode as automatic detection applies only to C-mode */
      if ((wMBus_mode_global == C_MODE) && (wMBus_Format_global == WMBUS_FORMAT_A_B))
      {
        for (i = 0; i < nPcktLength; i++)
        {
          vectcRadioBuff_copy[i] = vectcRadioBuff[i + 1];
        }
      }
      else
      {
        for (i = 0; i < nPcktLength; i++)
        {
          vectcRadioBuff_copy[i] = vectcRadioBuff[i];
        }
      }

#endif

#endif

#ifdef WMBUS_DEBUG
      wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_END_RX_SM_LAST_PROCESSING, nPcktLength, RxSync_Ongoing, RssiOnSync);
#endif

      wMBus_Radio_Event_bitmap |= WMBUS_RX_COMPLETED_WITH_RAW_BUFFER;

      *(global_buffer_size_ptr) = wMBus_Phy_wait_Rx_CRC_mngt(global_buffer_ptr, global_RssiDbm_ptr, *(global_buffer_format_ptr), global_RxSync_ptr);
    }

    /* Clear the IRQ flag */
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F);
  }

  /* check this case even if it shouldn't happen */
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F);

#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
    /* Call this API after RX */
    revertFrequencyBaseFor169();
#endif
#endif

#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_IRQ_RX_DONE_LOG, 0, 0, 0);
#endif

    wMBus_Radio_Event_bitmap |= WMBUS_RX_OVERFLOW_ERROR;

#ifdef STM32WL33XA
#ifdef ADJUST_FREQUENCY_169MHZ
    /* Call this API before RX */
    adjustFrequencyBaseFor169();
#endif
    /* Restart RX */
    FEM_Operation(FEM_RX);
#endif

    /* re-start Rx */
    __HAL_MRSUBG_STROBE_CMD(CMD_RX);
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_COMMAND_RX_CONTINUOUS_LOG, 0, 0, 0);
#endif
  }

#ifdef PREAMBLE_AND_SYNC_IRQ_ENABLE
  /* check this case even if it shouldn't happen */
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SYNC_VALID_F)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_SYNC_VALID_F);

    wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_SYNC_DETECTED;
  }

  /* check this case even if it shouldn't happen */
  if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_PREAMBLE_VALID_F)
  {
    __HAL_MRSUBG_CLEAR_RFSEQ_IRQ_FLAG(MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_PREAMBLE_VALID_F);

    wMBus_Radio_Event_bitmap |= WMBUS_RX_VALID_PREAMBLE_DETECTED;
  }
#endif
}

uint16_t wMBus_Phy_wait_Rx_CRC_mngt(uint8_t *RxBuffer, int32_t *RssiDbm, uint8_t FrameFormat, uint32_t *RxSync)
{
  /* CRC result */
  uint8_t xCrcResult = FALSE;

  /* remaining bytes to analyze */
  uint16_t dataBytesRemaining = 0;

  /* packet length without CRC */
  uint16_t nPcktLength_no_CRC = 0;

  /* store start of buffer */
  uint8_t *pStartBuffer = RxBuffer;

  while (!xRxDoneFlag);

  xRxDoneFlag = RESET;

  /* now copy Radio buffer into LL buffer to release Radio buffer */
  uint16_t i = 0;
  uint8_t j;
  uint8_t n_block = 1;

  /* nPcktLength computed during packet reception SM */
  if (global_buffer_allocation < nPcktLength)
  {
    /* in that case LL buffer is too small */
    /* just set nPcktLength = 0xFFFF and don't copy data to avoid memory corruption */
    nPcktLength = 0xFFFF;
    wMBus_Radio_Event_bitmap |= WMBUS_RX_BUFFER_SIZE_ERROR;
  }
  else
  {
    if (nPcktLength > WMBUS_RADIO_BUFFER_SIZE_MAX_3O6)
    {
      nPcktLength = 0xFFFF;
      wMBus_Radio_Event_bitmap |= WMBUS_RX_OVERFLOW_ERROR;
    }
    else
    {
      for (i = 0; i < nPcktLength; i++)
      {
        *(RxBuffer) = vectcRadioBuff_copy[i];
        RxBuffer++;
      }
    }
  }

  /* save RssiOnSync level and stop (and Start Rx) */
  RssiRxDone = RssiOnSync;

  /* save Sync Word used for Rx */
  RxSync_RxDone = RxSync_Ongoing;

  /* now send Sabort command and reset variables */
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
  wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

  *RssiDbm = __HAL_MRSUBG_CONVERT_RSSI_TO_DBM(RssiRxDone);
  *RxSync = RxSync_RxDone;

  /* check CRCs only if nPcktLength is consistent */
  if (nPcktLength != 0xFFFF)
  {
    /* as this point we can analyze wM-Bus CRCs - depends on Frame Format info
       or if dynamic frame format detection rely on RxSync_RxDone */
    /* different cases to consider */
    if (FrameFormat == WMBUS_FORMAT_A)
    {
      /* CRC verification */
      xCrcResult = TRUE;

      /* check 1st block CRC */
      xCrcResult = wMBus_Phy_CRC_check(pStartBuffer, pStartBuffer + WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2);

      /* update pStartBuffer ptr */
      pStartBuffer = pStartBuffer + WMBUS_FIRST_BLOCK_SIZE_FORMAT_A;

      /* update buffer size without CRC */
      nPcktLength_no_CRC = WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2;

      /* update remaining bytes to analyze */
      dataBytesRemaining = nPcktLength - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A;

      /* check CRCs of each block fully filled (WMBUS_FIRST_BLOCK_SIZE_FORMAT_A bytes) */
      for (uint16_t i = 0; i < (nPcktLength - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) / WMBUS_BLOCK_SIZE_FORMAT_A; i++)
      {
        xCrcResult &= wMBus_Phy_CRC_check(pStartBuffer, pStartBuffer + WMBUS_BLOCK_SIZE_FORMAT_A - 2);

        /* copy/shift by 2 bytes in buffer without CRCs - to LL*/
        for (j = 0; j < WMBUS_BLOCK_SIZE_FORMAT_A - 2; j++)
        {
          *(pStartBuffer - (2 * n_block) + j) = *(pStartBuffer + j);
        }

        /* update pointer */
        pStartBuffer = pStartBuffer + WMBUS_BLOCK_SIZE_FORMAT_A;
        n_block += 1;

        nPcktLength_no_CRC = nPcktLength_no_CRC + WMBUS_BLOCK_SIZE_FORMAT_A - 2;

        /* update remaining bytes to analyze */
        dataBytesRemaining = dataBytesRemaining - WMBUS_BLOCK_SIZE_FORMAT_A;
      }

      /* if last block has less than WMBUS_BLOCK_SIZE_FORMAT_A bytes */
      if ((nPcktLength - WMBUS_FIRST_BLOCK_SIZE_FORMAT_A) % WMBUS_BLOCK_SIZE_FORMAT_A != 0)
      {
        xCrcResult &= wMBus_Phy_CRC_check(pStartBuffer, pStartBuffer + dataBytesRemaining - 2);

        /* copy/shift by 2 bytes in buffer without CRCs - to LL*/
        for (j = 0; j < dataBytesRemaining - 2; j++)
        {
          *(pStartBuffer - (2 * n_block) + j) = *(pStartBuffer + j);
        }

        nPcktLength_no_CRC = nPcktLength_no_CRC + dataBytesRemaining - 2;
      }
    }
    else if (FrameFormat == WMBUS_FORMAT_B)
    {
      /* CRC verification */
      xCrcResult = TRUE;
      /* check if only second block or optional block to consider */
      if (nPcktLength <= (WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + WMBUS_SECOND_BLOCK_MAX_SIZE_FORMAT_B))
      {
        /* here only 1st + 2nb block => 1 CRC to check */
        xCrcResult = wMBus_Phy_CRC_check(pStartBuffer, pStartBuffer + nPcktLength - 2);
        nPcktLength_no_CRC = nPcktLength - 2;
      }
      else
      {
        /* here 2 CRCs to check : 2nd block + optional block */
        /* 2nd block CRC */
        xCrcResult = wMBus_Phy_CRC_check(pStartBuffer, pStartBuffer + WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + WMBUS_SECOND_BLOCK_MAX_SIZE_FORMAT_B - 2);
        /* optional block CRC */
        xCrcResult &= wMBus_Phy_CRC_check(pStartBuffer + WMBUS_FIRST_BLOCK_SIZE_FORMAT_B + WMBUS_SECOND_BLOCK_MAX_SIZE_FORMAT_B, pStartBuffer + nPcktLength - 2);
        nPcktLength_no_CRC = nPcktLength - 4;
      }
    }
  }

  if ((xCrcResult) && (nPcktLength != 0xFFFF))
  {
    wMBus_Radio_Event_bitmap |= WMBUS_RX_COMPLETED_WITH_VALID_CRC;
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_COMPLETED_WITH_VALID_CRC, nPcktLength, nPcktLength_no_CRC, 0);
#endif
    return nPcktLength_no_CRC;
  }
  else if (nPcktLength == 0xFFFF)
  {
    wMBus_Radio_Event_bitmap |= WMBUS_RX_BUFFER_SIZE_ERROR;
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_BUFFER_SIZE_ERROR, nPcktLength, nPcktLength_no_CRC, 0);
#endif
    return nPcktLength;
  }
  else
  {
    wMBus_Radio_Event_bitmap |= WMBUS_RX_COMPLETED_WITH_CRC_ERROR;
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_EVENT_WMBUS_RX_COMPLETED_WITH_CRC_ERROR, nPcktLength, nPcktLength_no_CRC, 0);
#endif
    return 0;
  }
}

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

void wMBus_Phy_CRC_append(uint8_t *pStart, uint8_t *pStop)
{
  uint16_t seed = 0x0000;
  while (pStart != pStop)
  {
    seed = wMBus_Phy_CRC_calc(seed, *pStart);
    pStart++;
  }
  seed = ~seed;
  pStop[0] = (uint8_t)(seed >> 8);
  pStop[1] = (uint8_t)seed;
}

uint8_t wMBus_Phy_check_radio_events(uint32_t wMBus_RadioEvent_Mask)
{
  if ((wMBus_Radio_Event_bitmap & wMBus_RadioEvent_Mask) == wMBus_RadioEvent_Mask)
  {
    /* clear radio flag event */
    wMBus_Radio_Event_bitmap &= ~wMBus_RadioEvent_Mask;
    return 1;
  }
  else
  {
    return 0;
  }
}

uint8_t wMBus_Phy_register_LL_buffer(uint8_t *RxBuffer, uint16_t *PacketLength_ptr, uint8_t *FrameFormat_ptr, uint32_t *RxSync_ptr, int32_t *RssiDbm_ptr)
{
  /* save LL Rx buffer pointer */
  global_buffer_ptr = RxBuffer;
  /* save LL buffer size pointer - used to report packet size to LL*/
  global_buffer_size_ptr = PacketLength_ptr;
  /* save LL buffer size (allocation) - used to avoid LL buffer corruption */
  global_buffer_allocation = *(PacketLength_ptr);
  /* save LL format */
  global_buffer_format_ptr = FrameFormat_ptr;

  /* save LL Rssi level of Rx buffer */
  global_RssiDbm_ptr = RssiDbm_ptr;

  /* report Rx Sync pattern used for Rx */
  global_RxSync_ptr = RxSync_ptr;

  /* check LL register size vs Radio buffer size : if too small => error */
  if (*PacketLength_ptr < WMBUS_RADIO_BUFFER_SIZE_MAX_NRZ)
  {
    /* error */
    return 1;
  }
  else
  {
    /* no error */
    return 0;
  }
}

uint8_t wMBus_Phy_start_Tx_test_mode(uint8_t Tx_TestMode)
{
  uint8_t TM_success = 0;

  switch (Tx_TestMode)
  {
  case TX_TM_CW:

    /* set CW - no modulation */
    HAL_MRSubG_SetModulation(MOD_CW, 0);

    /* set Test Mode */
    LL_MRSubG_SetTXMode(TX_PN);

    TM_success = SUCCESS;

    /* IRQ Config */
    __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SABORT_DONE_E);

    __HAL_MRSUBG_STROBE_CMD(CMD_TX);
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_COMMAND_TX_LOG, 0, 0, 0);
#endif

    break;

  case TX_TM_PN9:

    /* just set Test Mode (and keep same modulation as wM-Bus channel) */
    LL_MRSubG_SetTXMode(TX_PN);

    TM_success = SUCCESS;

    /* IRQ Config */
    __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_SABORT_DONE_E);

    __HAL_MRSUBG_STROBE_CMD(CMD_TX);
#ifdef WMBUS_DEBUG
    wMBus_Phy_trace_debug_log(DEBUG_COMMAND_TX_LOG, 0, 0, 0);
#endif
    break;

  default:

    TM_success = ERROR;
    break;
  }

  return TM_success;
}

uint8_t wMBus_Phy_stop_Tx_test_mode(void)
{
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
#ifdef WMBUS_DEBUG
  wMBus_Phy_trace_debug_log(DEBUG_COMMAND_SABORT_LOG, 0, 0, 0);
#endif

  /* wait for SABORT IRQ confirmation */
  while (!xSabortCompletedFlag);

  xSabortCompletedFlag = RESET;

  return SUCCESS;
}

int32_t wMBus_Phy_sense_rssi(uint32_t RxTimeoutMicrosec)
{
  /* IRQ Config */
  __HAL_MRSUBG_SET_RFSEQ_IRQ_ENABLE(MR_SUBG_GLOB_DYNAMIC_RFSEQ_IRQ_ENABLE_RX_TIMEOUT_E);

  /* start Rx timer without any Stop condition */
  wMBus_Phy_start_RxTimer(RxTimeoutMicrosec, 0);

  /* wait for Rx timeout event */
  while (!wMBus_Phy_check_radio_events(WMBUS_RX_TIMEOUT));

  /* return Rssi Dbm value */
  return HAL_MRSubG_GetRssidBm();
}

#ifdef WMBUS_RX_PERFORMANCE_ENABLED
void wMBus_Phy_Rx_performance_settings(uint8_t wMBus_mode, uint8_t wMBus_Direction)
{
  /* optimize RSSI threshold setting */
  LL_MRSubG_SetRssiThresholdRegister(wMBus_Phy_Rx_RssiThreshold[wMBus_mode][wMBus_Direction]);

  /* optimize Clock Recovery settings */;
  LL_MRSubG_SetClkRecCtrl0(wMBus_Phy_Rx_ClkRec[wMBus_mode][wMBus_Direction][0]);
  LL_MRSubG_SetClkRecCtrl1(wMBus_Phy_Rx_ClkRec[wMBus_mode][wMBus_Direction][1]);

  /* set RSSI_FLT register */
  LL_MRSubG_SetRSSIFilter(0xE3);

  /* only validated for T/S/C and even N - 2-FSK 868MHz modes */
  /* optimize AFC3 register to avoid residual PER */
  LL_MRSubG_SetAFC3(0xE9);

#ifdef STM32WL33XA
  if (wMBus_mode == N_MODE_Ng)
  {
    /* specific AFC settings */
    LL_MRSubG_SetAFC0(0x67);
    LL_MRSubG_SetAFC1(0x00);
    LL_MRSubG_SetAFC2(0x68);
    LL_MRSubG_SetAFC3(0xA8);
  }
#endif
}
#endif

#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
void wMBus_Phy_set_active_power_mode(uint8_t Active_PM)
{
  switch (Active_PM)
  {
  case WMBUS_LPM:
    /* in that case LPM means :
      - 1.2v SMPS
      - RF LDO bypassed
    */

    /* set SMPS level @ 1.2v */
    HAL_PWREx_ConfigSMPS_Update(LL_PWR_SMPS_OUTPUT_VOLTAGE_1V20, PWR_CR5_SMPSBOMSEL_1);

    /* bypass RF LDO */
    SET_BIT(PWR->CR2, PWR_CR2_RFREGBYP);

    break;
  case WMBUS_HPM:
    /* in that case HPM means :
      - 1.4v SMPS
      - RF LDO NOT bypassed
    */

    /* NOT bypass RF LDO */
    CLEAR_BIT(PWR->CR2, PWR_CR2_RFREGBYP);

    /* set SMPS level @ 1.4v */
    HAL_PWREx_ConfigSMPS_Update(LL_PWR_SMPS_OUTPUT_VOLTAGE_1V40, PWR_CR5_SMPSBOMSEL_1);

    break;

  case WMBUS_SMPS_BYPASS_STATIC:
    LL_PWR_DBGSMPS_Current_Selection(0);

    LL_PWR_EnableSMPSBOFStatic();
    LL_PWR_EnableSMPSBOF();

    break;
  default:
    break;
  }
}
#endif

#ifdef WMBUS_DEBUG
void wMBus_Phy_initialize_debug_log(void)
{
  uint8_t i = 0;

  for (i = 0; i < WMBUS_LOG_BUFFER_SIZE; i++)
  {
    wMBus_debug_log[i].Timestamp = 0;
    wMBus_debug_log[i].event = 0;
    wMBus_debug_log[i].debug_info1 = 0;
    wMBus_debug_log[i].debug_info2 = 0;
    wMBus_debug_log[i].debug_info3 = 0;
  }

  wMBus_debug_log_index = 0;
}

void wMBus_Phy_trace_debug_log(uint32_t event, uint32_t debug_info1, uint32_t debug_info2, uint32_t debug_info3)
{
  wMBus_debug_log[wMBus_debug_log_index].Timestamp = HAL_GetTick();
  wMBus_debug_log[wMBus_debug_log_index].event = event;
  wMBus_debug_log[wMBus_debug_log_index].debug_info1 = debug_info1;
  wMBus_debug_log[wMBus_debug_log_index].debug_info2 = debug_info2;
  wMBus_debug_log[wMBus_debug_log_index].debug_info3 = debug_info3;

  wMBus_debug_log_index++;

  if (wMBus_debug_log_index == WMBUS_LOG_BUFFER_SIZE)
  {
    wMBus_debug_log_index = 0;
  }
}
#endif

#ifdef WMBUS_FIRST_CRC_CHECK
uint8_t wMBus_Phy_first_CRC_check(void)
{
  /* copy Radio Buffer into Copy buffer */
  uint8_t i = 0;

  if (wMBus_Format_global != WMBUS_FORMAT_B)
  {
    /* copy into vectcRadioBuff_copy[] for CRC analysis
       if T+C mode and T-mode then perform 3o6 decoding by Sw */

    /* check first cases where only single modes are selected */
#if !defined(T1C1_ACTIVATED) && !defined(T1C1_ACTIVATED_C1_MODE)
#ifndef AUTOMATIC_FORMAT_DETECTION_C_MODE
    /* here no C-mode dynamic format, so copy directly NRZ bytes to vectcRadioBuff_copy[] */
    /* copy WMBUS_FIRST_BLOCK_SIZE_FORMAT_A bytes (12) */
    for (i = 0; i < WMBUS_FIRST_BLOCK_SIZE_FORMAT_A; i++)
    {
      vectcRadioBuff_copy[i] = vectcRadioBuff[i];
    }
#else
    /* here C-mode dynamic format to handle - if it is the case */
    /* check if C-mode requested + A_B format only and A format detected
       (then copy starting from 2nd Rx byte - otherwise copy from 1st byte */
    /* copy using NRZ */
    /* copy WMBUS_FIRST_BLOCK_SIZE_FORMAT_A bytes (12) */
    if ((wMBus_mode_global == C_MODE) && (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A) && (wMBus_Format_global == WMBUS_FORMAT_A_B))
    {
      for (i = 0; i < WMBUS_FIRST_BLOCK_SIZE_FORMAT_A; i++)
      {
        vectcRadioBuff_copy[i] = vectcRadioBuff[i + 1];
      }
    }
    else if ((wMBus_mode_global == C_MODE) && (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B))
    {
      return 1; // declare CRC Ok to not check CRC in case Frame Format B detected
    }
    else
    {
      /* other cases :
        - C-mode + A-format requested
        - S-mode
        - T-mode */
      for (i = 0; i < WMBUS_FIRST_BLOCK_SIZE_FORMAT_A; i++)
      {
        vectcRadioBuff_copy[i] = vectcRadioBuff[i];
      }
    }
#endif
#else // here need to manage T+C modes : so we have to check effective Rx mode and frame format
    if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A)
    {
      /* here C-mode dynamic format to handle - if it is the case */
      /* check if C-mode (then copy starting from 3rd Rx byte */
      /* copy using NRZ */
      /* copy WMBUS_FIRST_BLOCK_SIZE_FORMAT_A bytes (12) */
      for (i = 0; i < WMBUS_FIRST_BLOCK_SIZE_FORMAT_A; i++)
      {
        vectcRadioBuff_copy[i] = vectcRadioBuff[i + 2];
      }
    }
    else if (RxSync_Ongoing == WMBUS_SYNC_PATTERN_T_MODE_M2O)
    {
      /* here T-mode format to handle - if it is the case */
      /* perform 3o6 Sw decoding 3 bytes in -> 2 bytes out to copy */
      /* output : WMBUS_FIRST_BLOCK_SIZE_FORMAT_A bytes (12) */

      uint16_t tos_q_tgt = WMBUS_FIRST_BLOCK_SIZE_FORMAT_A + 6; // 12 NRZ bytes = 18 3o6 bytes

      tos_q_head = 0; // 1st 3o6 RAW byte to decode
      tos_q_tail = 0; // 1st decoded byte to write

      /* indexes of RAW 3o6 buffer */
      tos_q_done_head = 0;

      while (tos_q_done_head < tos_q_tgt)
      {
        ToS_decode(&vectcRadioBuff[tos_q_head], tos_out);

        tos_q_head = tos_q_head + 3;
        tos_q_done_head += 3;

        for (uint8_t j = 0; j < 2; j++)
        {
          vectcRadioBuff_copy[tos_q_tail] = tos_out[j];
          tos_q_tail++;
        }
      }
    }
    else
    /* other cases (S mode) : just copy NRZ bytes buffer to buffer */
    {
      for (i = 0; i < WMBUS_FIRST_BLOCK_SIZE_FORMAT_A; i++)
      {
        vectcRadioBuff_copy[i] = vectcRadioBuff[i];
      }
    }
#endif
    /* here vectcRadioBuff_copy[] is done with WMBUS_FIRST_BLOCK_SIZE_FORMAT_A bytes */
    /* check CRC */
    /* CRC verification */
    /* CRC result */
    uint8_t xCrcResult = TRUE;

    /* check 1st block CRC */
    xCrcResult = wMBus_Phy_CRC_check(&vectcRadioBuff_copy[0], &vectcRadioBuff_copy[WMBUS_FIRST_BLOCK_SIZE_FORMAT_A - 2]);

    /* CRC OK if xCrcResult = 1
       CRC KO if xCrcResult = 0 */
    return xCrcResult;
  }
  else
  {
    /* do not check CRC for frame format B */
    return 1;
  }
}
#endif

/**
  * @}
  */

/** @defgroup Radio_Private_Functions Radio Private Functions
  * @{
  */
void wMBus_Phy_MRSubG_MspInit(void)
{
  /* Peripheral clock enable */
  if (__HAL_RCC_MRSUBG_IS_CLK_DISABLED())
  {
    /* MRSUBG Peripheral reset */
    __HAL_RCC_MRSUBG_FORCE_RESET();
    __HAL_RCC_MRSUBG_RELEASE_RESET();
    
    /* Enable MRSUBG peripheral clock */
    __HAL_RCC_MRSUBG_CLK_ENABLE();
  }

  /* MRSUBG interrupt Init*/
  HAL_NVIC_SetPriority(MRSUBG_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MRSUBG_IRQn);
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
