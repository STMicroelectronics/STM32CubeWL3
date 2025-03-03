/**
 ******************************************************************************
 * @file    stm32wl3_wmbphy_radio.h
 * @author  MCD Application Team
 * @brief   Header file of WMBUS PHY RADIO module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WL3_WMBPHY_RADIO_H
#define STM32WL3_WMBPHY_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal_pwr.h"
#include "stm32wl3x_ll_mrsubg.h"

/** @addtogroup STM32WL3_WMBUS_PHY
 * @{
 */

/** @addtogroup RADIO
 * @{
 */

/** @defgroup RADIO_Private_Constants RADIO Private Constants
 * @{
 */
/**
 * @brief  WMBUS PHY Radio mode number
 */
#define WMBUS_MODE_NUMBER 11
/**
 * @}
 */

/** @defgroup RADIO_Exported_Types RADIO Exported Types
 * @{
 */
/**
 * @brief Enumeration of WMBUS PHY Radio submodes
 */
typedef enum {
    T_MODE = 0,    /**< T-mode */
    S_MODE = 1,    /**< S-mode with long preamble */
    S1M_MODE = 2,  /**< S-mode with short preamble */
    C_MODE = 3,    /**< C-mode */
    N_MODE_Na = 4, /**< N-mode, N1a, N2a */
    N_MODE_Nb = 5, /**< N-mode, N1b, N2b */
    N_MODE_Nc = 6, /**< N-mode, N1c, N2c */
    N_MODE_Nd = 7, /**< N-mode, N1d, N2d */
    N_MODE_Ne = 8, /**< N-mode, N1e, N2e */
    N_MODE_Nf = 9, /**< N-mode, N1f, N2f */
    N_MODE_Ng = 10 /**< N-mode, N2g */
} wmbphy_submode_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Rx state machine states
 */
typedef enum {
    SM_RX_NEW_PACKET = 0, /**< New packet received state */
    SM_RX_INTERM = 1,     /**< Intermediate state */
    SM_RX_LAST = 2        /**< Last state */
} wmbphy_state_machine_Rx_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Frame Formats
 * @note  If AUTOMATIC_FORMAT_DETECTION_C_MODE is defined, value 0 is WMBUS_FORMAT_A_B else WMBUS_FORMAT_UNDEFINED
 */
typedef enum {
#ifndef AUTOMATIC_FORMAT_DETECTION_C_MODE
    WMBUS_FORMAT_UNDEFINED = 0, /**< Undefined format */
#else
    WMBUS_FORMAT_A_B = 0,       /**< Format A or B */
#endif
    WMBUS_FORMAT_A = 1,         /**< Format A */
    WMBUS_FORMAT_B = 2          /**< Format B */
} wmbphy_frame_format_t;

/**
 * @brief Enumeration of WMBUS PHY Radio 1st Block size
 */
typedef enum {
    WMBUS_FIRST_BLOCK_SIZE_FORMAT_A = 12, /**< First block size for format A */
    WMBUS_FIRST_BLOCK_SIZE_FORMAT_B = 10  /**< First block size for format B */
} wmbphy_first_block_size_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Subsequent Block size
 */
typedef enum {
    WMBUS_BLOCK_SIZE_FORMAT_A = 18,            /**< Block size for format A */
    WMBUS_SECOND_BLOCK_SIZE_FORMAT_B = 118     /**< Second block size for format B */
} wmbphy_subsequent_block_size_t;

#ifdef WMBUS_DEBUG
/**
 * @brief Structure of WMBUS PHY Radio debug
 * @note  This struct is defined only if WMBUS_DEBUG is defined
 */
typedef struct {
    uint32_t Timestamp;    /**< Timestamp */
    uint32_t event;        /**< Event */
    uint32_t debug_info1;  /**< Debug information 1 */
    uint32_t debug_info2;  /**< Debug information 2 */
    uint32_t debug_info3;  /**< Debug information 3 */
} wmbphy_debug_t;

/**
 * @brief Enumeration of WMBUS PHY Radio debug information
 * @note  This enum is defined only if WMBUS_DEBUG is defined
 */
typedef enum {
    DEBUG_COMMAND_TX_LOG = 0x00000001,                              /**< Log TX command */
    DEBUG_COMMAND_RX_CONTINUOUS_LOG = 0x00000002,                   /**< Log RX continuous command */
    DEBUG_COMMAND_RX_TIMER_LOG = 0x00000003,                        /**< Log RX timer command */
    DEBUG_COMMAND_SABORT_LOG = 0x00000004,                          /**< Log Abort command */

    DEBUG_IRQ_TX_DONE_LOG = 0x00000101,                             /**< Log TX done IRQ */
    DEBUG_IRQ_RX_DONE_LOG = 0x00000102,                             /**< Log RX done IRQ */
    DEBUG_IRQ_RX_FIFO_LOG = 0x00000103,                             /**< Log RX FIFO IRQ */
    DEBUG_IRQ_RX_TIMER_LOG = 0x00000104,                            /**< Log RX timer IRQ */
    DEBUG_IRQ_ABORT_LOG = 0x00000105,                               /**< Log Abort IRQ */

    DEBUG_EVENT_WMBUS_TX_COMPLETED = 0x00000201,                    /**< Log TX completed event */
    DEBUG_EVENT_WMBUS_RX_COMPLETED_WITH_RAW_BUFFER = 0x00000202,    /**< Log RX completed with raw buffer event */
    DEBUG_EVENT_WMBUS_RX_COMPLETED_WITH_VALID_CRC = 0x00000203,     /**< Log RX completed with valid CRC event */
    DEBUG_EVENT_WMBUS_RX_COMPLETED_WITH_CRC_ERROR = 0x00000204,     /**< Log RX completed with CRC error event */
    DEBUG_EVENT_WMBUS_RX_VALID_PREAMBLE_DETECTED = 0x00000205,      /**< Log Valid preamble detected event */
    DEBUG_EVENT_WMBUS_RX_VALID_SYNC_DETECTED = 0x00000206,          /**< Log Valid sync detected event */
    DEBUG_EVENT_WMBUS_RX_VALID_HEADER_DETECTED = 0x00000207,        /**< Log Valid header detected event */
    DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR = 0x00000208,           /**< Log Valid header error event */
    DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_C_MODE = 0x00000209,    /**< Log Valid header error in C mode event */
    DEBUG_EVENT_WMBUS_RX_VALID_HEADER_ERROR_L_FIELD = 0x0000020A,   /**< Log Valid header error in L field event */
    DEBUG_EVENT_WMBUS_RX_TIMEOUT = 0x0000020B,                      /**< Log RX timeout event */
    DEBUG_EVENT_WMBUS_RX_BUFFER_SIZE_ERROR = 0x0000020C,            /**< Log RX buffer size error event */
    DEBUG_EVENT_WMBUS_RX_OVERFLOW_ERROR = 0x0000020D,               /**< Log RX overflow error event */
    DEBUG_EVENT_WMBUS_END_RX_SM_FISRT_PROCESSING = 0x0000020E,      /**< Log End RX state machine first processing event */
    DEBUG_EVENT_WMBUS_END_RX_SM_LAST_PROCESSING = 0x0000020F,       /**< Log End RX state machine last processing event */
    DEBUG_EVENT_WMBUS_FIRST_CRC_ERROR = 0x00000210                  /**< Log First CRC error event */
} wmbphy_debug_info_t;
#endif

/**
 * @brief Enumeration of WMBUS PHY Radio Rx Timer timeout mask
 */
typedef enum {
    RX_CS_TIMEOUT_MASK = 0x01,  /**< CS timeout mask */
    RX_PQI_TIMEOUT_MASK = 0x02, /**< PQI timeout mask */
    RX_SQI_TIMEOUT_MASK = 0x04, /**< SQI timeout mask */
    RX_OR_nAND_SELECT = 0x08    /**< OR/AND select mask */
} wmbphy_RxTimer_timeout_mask_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Direction
 */
typedef enum {
    METER_TO_OTHER = 0, /**< Meter to other direction */
    OTHER_TO_METER = 1  /**< Other to meter direction */
} wmbphy_direction_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Meter modes
 */
typedef enum {
    OTHER = 0, /**< Other mode for bidirectional meter */
    METER = 1  /**< Meter mode for bidirectional meter */
} wmbphy_meter_modes_t;

/**
 * @brief Enumeration of WMBUS PHY Radio events
 */
typedef enum {
    WMBUS_TX_COMPLETED = 0x00000001,                    /**< TX completed with Tx done IRQ event */
    WMBUS_RX_COMPLETED_WITH_RAW_BUFFER = 0x00000002,    /**< RX completed with valid Sync and raw buffer event */
    WMBUS_RX_COMPLETED_WITH_VALID_CRC = 0x00000004,     /**< RX completed with valid Sync and valid CRC verification event */
    WMBUS_RX_COMPLETED_WITH_CRC_ERROR = 0x00000008,     /**< RX completed with CRC error event */
    WMBUS_RX_VALID_PREAMBLE_DETECTED = 0x00000010,      /**< Valid preamble detected event */
    WMBUS_RX_VALID_SYNC_DETECTED = 0x00000020,          /**< Valid sync detected event */
    WMBUS_RX_VALID_HEADER_DETECTED = 0x00000040,        /**< Valid header detected event */
    WMBUS_RX_VALID_HEADER_ERROR = 0x00000080,           /**< Valid header error event */
    WMBUS_RX_TIMEOUT = 0x00000100,                      /**< RX timeout event */
    WMBUS_RX_BUFFER_SIZE_ERROR = 0x00000200,            /**< RX buffer size error event */
    WMBUS_RX_OVERFLOW_ERROR = 0x00000400,               /**< RX overflow error event */
    WMBUS_FIRST_CRC_ERROR = 0x00000800                  /**< First CRC error event */
} wmbphy_events_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Sync patterns
 * @note  If T1C1_ACTIVATED or T1C1_ACTIVATED_C1_MODE are defined WMBUS_SYNC_PATTERN_T1C1_MODE_M2O_GATEWAY is defined
 *        If AUTOMATIC_FORMAT_DETECTION_C_MODE is defined WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_UNDEFINED is defined    
 */
typedef enum {
    WMBUS_SYNC_PATTERN_T_MODE_M2O = 0x55543D00,                 /**< T mode Meter to Other sync pattern */
    WMBUS_SYNC_PATTERN_T_MODE_O2M = 0x1DA58000,                 /**< T mode Other to Meter sync pattern */
    WMBUS_SYNC_PATTERN_S_MODE_M2O = 0x54769600,                 /**< S mode Meter to Other sync pattern */
    WMBUS_SYNC_PATTERN_S_MODE_O2M = 0x1DA58000,                 /**< S mode Other to Meter sync pattern */
    WMBUS_SYNC_PATTERN_S1_MODE_M2O = 0x54769600,                /**< S1 mode Meter to Other sync pattern */
    WMBUS_SYNC_PATTERN_S1_MODE_O2M = 0x1DA58000,                /**< S1 mode Other to Meter sync pattern */
    WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_A = 0x543D54CD,        /**< C mode Meter to Other format A sync pattern */
    WMBUS_SYNC_PATTERN_C_MODE_O2M_FORMAT_A = 0x543D54CD,        /**< C mode Other to Meter format A sync pattern */
    WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_B = 0x543D543D,        /**< C mode Meter to Other format B sync pattern */
    WMBUS_SYNC_PATTERN_C_MODE_O2M_FORMAT_B = 0x543D543D,        /**< C mode Other to Meter format B sync pattern */
#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE)
    WMBUS_SYNC_PATTERN_T1C1_MODE_M2O_GATEWAY = 0x55543D00,      /**< T1C1 mode Meter to Other gateway sync pattern */
#endif
#ifdef AUTOMATIC_FORMAT_DETECTION_C_MODE
    WMBUS_SYNC_PATTERN_C_MODE_M2O_FORMAT_UNDEFINED = 0x543D5400, /**< C mode Meter to Other format undefined sync pattern */
#endif
} wmbphy_sync_pattern_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Sync lengths
 * @note  If T1C1_ACTIVATED or T1C1_ACTIVATED_C1_MODE are defined WMBUS_SYNC_LENGTH_T1C1_MODE_M2O_GATEWAY is defined
 *        If AUTOMATIC_FORMAT_DETECTION_C_MODE is defined WMBUS_SYNC_LENGTH_C_MODE_M2O_FORMAT_UNDEFINED is defined
 */
typedef enum {
    WMBUS_SYNC_LENGTH_T_MODE_M2O = 24,                  /**< T mode Meter to Other sync length */
    WMBUS_SYNC_LENGTH_T_MODE_O2M = 18,                  /**< T mode Other to Meter sync length */
    WMBUS_SYNC_LENGTH_S_MODE_M2O = 24,                  /**< S mode Meter to Other sync length */
    WMBUS_SYNC_LENGTH_S_MODE_O2M = 18,                  /**< S mode Other to Meter sync length */
    WMBUS_SYNC_LENGTH_S1_MODE_M2O = 24,                 /**< S1 mode Meter to Other sync length */
    WMBUS_SYNC_LENGTH_S1_MODE_O2M = 18,                 /**< S1 mode Other to Meter sync length */
    WMBUS_SYNC_LENGTH_C_MODE_M2O = 32,                  /**< C mode Meter to Other sync length */
    WMBUS_SYNC_LENGTH_C_MODE_O2M = 32,                  /**< C mode Other to Meter sync length */
#if defined(T1C1_ACTIVATED) || defined(T1C1_ACTIVATED_C1_MODE)
    WMBUS_SYNC_LENGTH_T1C1_MODE_M2O_GATEWAY = 24,       /**< T1C1 mode Meter to Other gateway sync length */
#endif
#ifdef AUTOMATIC_FORMAT_DETECTION_C_MODE
    WMBUS_SYNC_LENGTH_C_MODE_M2O_FORMAT_UNDEFINED = 24, /**< C mode Meter to Other format undefined sync length */
#endif
} wmbphy_sync_length_t;

/**
 * @brief Enumeration of WMBUS PHY Radio Tx test modes
 */
typedef enum {
    TX_TM_CW = 0x01, /**< Continuous wave test mode */
    TX_TM_PN9 = 0x02 /**< PN9 test mode */
} wmbphy_tx_test_mode_t;

#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
/**
 * @brief Enumeration of WMBUS PHY Radio Active Power modes
 * @note  This enumeration is defined only if WMBUS_ACTIVE_POWER_MODE_ENABLED is defined
 */
typedef enum {
    WMBUS_LPM = 0x01,                   /**< Low power mode */
    WMBUS_HPM = 0x02,                   /**< High power mode */
    WMBUS_SMPS_BYPASS_DYNAMIC = 0x03,   /**< SMPS bypass dynamic mode */
    WMBUS_SMPS_BYPASS_STATIC = 0x04     /**< SMPS bypass static mode */
} wmbphy_active_power_mode_t;
#endif

/**
 * @}
 */

/** @defgroup RADIO_Exported_Functions RADIO Exported Functions
 * @{
 */

void wmbphy_IRQHandler(void);

uint8_t wmbphy_start_transmission(void);
uint8_t wmbphy_start_continuousRx(void);
uint8_t wmbphy_start_RxTimer(uint32_t RxTimeout, uint8_t RxTimeoutStopCondition);

void wmbphy_init(uint8_t WmBus_mode, uint8_t WmBus_Direction, uint8_t WmBus_Format);

#ifdef WMBUS_CRC_IN_HAL
void wmbphy_prepare_Tx_CRC_mngt(uint8_t *WmBus_TxBuffer, uint16_t WmBusTxBuffer_size, uint8_t FrameFormat);
uint16_t wmbphy_wait_Rx_CRC_mngt(uint8_t *LL_RxBuffer, int32_t *RssiDbm, uint8_t FrameFormat, uint32_t *RxSync);
uint16_t wmbphy_CRC_calc(uint16_t crcReg, uint8_t crcData);
uint8_t wmbphy_CRC_check(uint8_t *pStart, uint8_t *pStop);
void wmbphy_CRC_append(uint8_t *pStart, uint8_t *pStop);
#endif

void wmbphy_prepare_Tx(uint8_t *WmBus_TxBuffer, uint16_t WmBusTxBuffer_size);
void wmbphy_prepare_Rx(void);
void wmbphy_abort(void);
uint8_t wmbphy_wait_Tx_completed(void);
uint16_t wmbphy_wait_Rx_completed(uint8_t *LL_RxBuffer, int32_t *RssiDbm, uint32_t *RxSync);

#ifdef WMBUS_NO_BLOCKING_HAL
uint8_t wmbphy_check_radio_events(uint32_t WmBus_RadioEvent_Mask);
uint8_t wmbphy_register_LL_buffer(uint8_t *LL_RxBuffer, uint16_t *LL_PacketLength_ptr, uint8_t *LL_FrameFormat_ptr, uint32_t *LL_WmBus_RxSync_ptr, int32_t *LL_WmBus_RssiDbm_ptr);
#endif

uint8_t wmbphy_start_Tx_test_mode(uint8_t Tx_TestMode);
uint8_t wmbphy_stop_Tx_test_mode(void);
int32_t wmbphy_sense_rssi(uint32_t RxTimeoutMicrosec);

#ifdef WMBUS_RX_PERFORMANCE_ENABLED
void wmbphy_Rx_performance_settings(uint8_t WmBus_mode, uint8_t WmBus_Direction);
#endif

#ifdef WMBUS_DEBUG
void wmbphy_initialize_debug_log(void);
void wmbphy_trace_debug_log(uint32_t event, uint32_t debug_info1, uint32_t debug_info2, uint32_t debug_info3);
#endif

#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
void wmbphy_set_active_power_mode(uint8_t Active_PM);
#endif

#ifdef WMBUS_FIRST_CRC_CHECK
uint8_t wmbphy_first_CRC_check(void);
#endif

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* STM32WL3_WMBPHY_RADIO_H */
