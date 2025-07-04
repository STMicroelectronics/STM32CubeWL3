/**
  ******************************************************************************
  * @file    stm32wl3_wMBus_DataLink.h
  * @author  MCD Application Team
  * @brief   Header file of wM-Bus DataLink module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WL3_WMBUS_DATALINK_H
#define STM32WL3_WMBUS_DATALINK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3_wMBus_Phy_pwr.h"
#include "stm32wl3_wMBus_Phy_radio.h"
#include "stm32wl3_wMBus_DataLink_timer.h"
#include "stm32wl3x_hal_pwr.h"
#include "stm32wl3x_ll_mrsubg.h"

/** @addtogroup STM32WL3_wM-Bus
  * @{
  */

/** @addtogroup DataLink
  * @{
  */

/** @defgroup DataLink_Private_Constants DataLink Private Constants
  * @{
  */
#define DATALINK_LAYER_OFFSET 9 /*!<Link layer offset:(C-field + M-field + A-field)*/
#define CI_FIELD_SIZE 1         /*!<CI-field offset*/
#define CRC_FIELD_SIZE 2        /*!<CRC field size*/
#define LINK_TIMEOUT 250        /*!<Link timeout in ms*/

/**
  * @}
  */

/** @defgroup DataLink_Exported_Variables DataLink Exported Variables
  * @{
  */
extern uint8_t new_Rx_frame;
/**
  * @}
  */

/** @defgroup DataLink_Exported_Types DataLink Exported Types
  * @{
  */
/**
  * @brief  wM-Bus DataLink Layer Status Enum.
  */
typedef enum
{
  LINK_STATUS_SUCCESS = 0x0000,              /*!< No error*/
  PHY_TX_COMPLETED = 0x0001,                 /**< TX completed with Tx done IRQ event */
  PHY_RX_COMPLETED_WITH_RAW_BUFFER = 0x0002, /**< RX completed with valid Sync and raw buffer event */
  PHY_RX_COMPLETED_WITH_VALID_CRC = 0x0004,  /**< RX completed with valid Sync and valid CRC verification event */
  PHY_RX_COMPLETED_WITH_CRC_ERROR = 0x0008,  /**< RX completed with CRC error event */
  PHY_RX_VALID_PREAMBLE_DETECTED = 0x0010,   /**< Valid preamble detected event */
  PHY_RX_VALID_SYNC_DETECTED = 0x0200,       /**< Valid sync detected event */
  PHY_RX_VALID_HEADER_DETECTED = 0x0040,     /**< Valid header detected event */
  PHY_RX_VALID_HEADER_ERROR = 0x0080,        /**< Valid header error event */
  PHY_RX_TIMEOUT = 0x0100,                   /**< RX timeout event */
  PHY_RX_BUFFER_SIZE_ERROR = 0x0200,         /**< RX buffer size error event */
  PHY_RX_OVERFLOW_ERROR = 0x0400,            /**< RX overflow error event */
  PHY_FIRST_CRC_ERROR = 0x0800,              /**< First CRC error event */
  /* LINK layer status codes */
  LINK_STATUS_INVALID_SIZE = 0x4000,         /*!<The data-field buffer is too large*/
  LINK_STATUS_INVALID_ATTR,                  /*!< The attribute ID is invalid*/
  LINK_STATUS_INVALID_ATTR_VAL,              /*!< The attribute value is invalid.*/
  LINK_STATUS_NOT_PERMITTED,                 /*!<Link Status for no permission*/
  LINK_STATUS_CRC_ERROR,                     /*!<CRC for first block is invalid*/
  LINK_STATUS_FRAME_ERROR,                   /*!<There is not enough data to parse the first block */
  LINK_STATUS_TX_RETRY_ERROR,                /*!<Link Status for re-transmission timeout*/
  LINK_STATUS_UNKNOWN_ERROR = 0xFFFF         /*!<Data is not received within the timeout period specified*/
} wMBus_DataLink_status_t;

/**
  * @brief  wM-Bus Data field structure.
  */
typedef struct
{
  uint8_t size;     /*!< Payload Size; */
  uint8_t *payload; /*!< pointer to Payload buffer; */
} wMBus_DataLink_data_field_t;

/**
  * @brief  wM-Bus DataLink layer Frame primitive definition.
  */
typedef struct
{
  uint8_t l_field;                        /*!< L-field attribute*/
  uint8_t c_field;                        /*!< C-field attribute*/
  uint8_t *m_field;                       /*!< M-field attribute*/
  uint8_t *a_field;                       /*!< A-field attribute*/
  uint8_t ci_field;                       /*!< CI-field attribute*/
  wMBus_DataLink_data_field_t data_field; /*!< Data field; */
} wMBus_DataLink_frame_t;

/**
  * @brief  wM-Bus DataLink layer attributes.
  */
typedef struct
{
  wMBus_Phy_meter_modes_t DevType;          /*!< Device type attribute*/
  uint8_t MField[2];                        /*!< M-field attribute*/
  uint8_t AField[6];                        /*!< A-field attribute*/
  wMBus_Phy_frame_format_t LinkFrameFormat; /*!< Frame format attribute*/
  uint32_t LinkTimeout;                     /*!< Timeout attribute*/
  uint8_t LinkFreqAccessCycle;              /*!< Frequency access cycle attribute*/
  wMBus_Phy_direction_t direction;          /*!< Direction attribute*/
  wMBus_Phy_submode_t submode;              /*!< Submode attribute*/
} wMBus_DataLink_attributes_t;

/**
  * @brief  wM-Bus DataLink State Enum.
  */
typedef enum
{
  DL_SM_IDLE = 0,         /*!<Data Link Layer state machine state is idle*/
  DL_SM_SEND_NKE,         /*!<Data Link Layer state machine state is in SND-NKE state*/
  DL_SM_SENT_SND_IR,      /*!<Data Link Layer state machine state is in SND-IR state*/
  DL_SM_SEND_CNF_IR,      /*!<Data Link Layer state machine state is in CNF-IR state*/
  DL_SM_SENT_SND_NR,      /*!<Data Link Layer state machine state is in SND-NR state*/
  DL_SM_SEND_SND_UD,      /*!<Data Link Layer state machine state is in SND-UD state*/
  DL_SM_SEND_SND_UD2,     /*!<Data Link Layer state machine state is in SND-UD2 state*/
  DL_SM_SEND_REQ_UD1,     /*!<Data Link Layer state machine state is in REQ-DU1 state*/
  DL_SM_SEND_REQ_UD2,     /*!<Data Link Layer state machine state is in REQ-UD2 state*/
  DL_SM_SEND_RSP_UD,      /*!<Data Link Layer state machine state is in RSP-UD state*/
  DL_SM_SEND_ACK,         /*!<Data Link Layer state machine state is in ACK state*/
  DL_SM_SENT_ACC_DMD,     /*!<Data Link Layer state machine state is in ACC-DMD state*/
  DL_INVALID_EVENT = 0xFF /*!<Data Link Layer state machine state is in invalid state*/
} wMBus_DataLink_state_t;

/**
  * @brief  wM-Bus DataLink Layer C-Field Enum.
  */
typedef enum
{
  /* Primary Station codes PRM bit=1 */
  DL_SND_NKE = 0x40, /*!< Reset Link after communication; Direction from Concentrator to Meter */
  DL_SND_UD = 0x53,  /*!< Send User data; Direction from Concentrator to Meter. Meter shall respond with ACK if receive FCV=1 */
  DL_SND_UD2 = 0x43, /*!< Send User data; Direction from Concentrator to Meter. Meter shall respond with RSP_UD if receive FCV=0 */
  DL_SND_NR = 0x44,  /*!< Send Application Data; Direction from Meter to Concentrator without request Send/No reply */
  DL_SND_IR = 0x46,  /*!< Installation Request; Direction from Meter to Concentrator; Concentrator shall respond with CNF_IR */
  DL_ACC_NR = 0x47,  /*!< Access Request; Direction from Meter to Concentrator without request Send/No reply */
  DL_ACC_DMD = 0x48, /*!< Access Demand; Direction from Meter to Concentrator; Concentrator shall respond with ACK */
  DL_REQ_UD1 = 0x4A, /*!< Alarm request (Request User Data Class 1); Direction from Concentrator to Meter; Meter shall respond with RSP_UD or ACK */
  DL_REQ_UD2 = 0x4B, /*!< Data Request (Request User Data Class 2); Direction from Concentrator to Meter; Meter shall respond with RSP_UD */
  /* Secondary Station codes PRM bit=0 */
  DL_ACK = 0x00,     /*!< Acknowledgement; Both directions; Shall be sent after ACC-DMD, SND-UD or also REQ-UD1 when no alert happened */
  DL_CNF_IR = 0x06,  /*!< Confirms the successful installation of the meter; Direction from Concentrator to Meter. Shall be sent after SND-IR */
  DL_RSP_UD = 0x08,  /*!< Response of User Data; Direction from Meter to Concentrator. Shall be sent after REQ-UD1 or REQ-UD2 */
  DL_NO_FRAME = 0xFF /*!< No C-field */
} wMBus_DataLink_C_field_t;

/**
  * @brief  wM-Bus DataLink Layer CI-Field Enum.
  */
typedef enum
{
  CI_DATA_TO_METER_NO_TPL = 0x51,            /*!< Data from Readout device to Meter without Transport Layer */
  CI_DATA_TO_METER_SHORT_TPL = 0x5A,         /*!< Data from Readout device to Meter with short Transport Layer */
  CI_DATA_TO_METER_LONG_TPL = 0x5B,          /*!< Data from Readout device to Meter with long Transport Layer */
  CI_COSEM_TO_METER_LONG_TPL = 0x60,         /*!< COSEM Data from Readout device to Meter with long Transport Layer */
  CI_COSEM_TO_METER_SHORT_TPL = 0x61,        /*!< COSEM Data from Readout device to Meter with short Transport Layer */
  CI_OBIS_TO_METER_LONG_TPL = 0x64,          /*!< OBIS Data from Readout device to Meter with long Transport Layer */
  CI_OBIS_TO_METER_SHORT_TPL = 0x65,         /*!< OBIS Data from Readout device to Meter with short Transport Layer */
  CI_EN13757_3_FF_NO_TPL = 0x69,             /*!< EN 13757-3 Application Layer with Format frame without Transport Layer */
  CI_EN13757_3_FF_SHORT_TPL = 0x6A,          /*!< EN 13757-3 Application Layer with Format frame and short Transport Layer */
  CI_EN13757_3_FF_LONG_TPL = 0x6B,           /*!< EN 13757-3 Application Layer with Format frame and long Transport Layer */
  CI_CLOCK_SYNC_ABS = 0x6C,                  /*!< Clock synchronization absolute */
  CI_CLOCK_SYNC_REL = 0x6D,                  /*!< Clock synchronization relative */
  CI_APPL_ERROR_FROM_METER_SHORT_TPL = 0x6E, /*!< Application error from Meter with short Transport Layer */
  CI_APPL_ERROR_FROM_METER_LONG_TPL = 0x6F,  /*!< Application error from Meter with long Transport Layer */
  CI_APPL_ERROR_FROM_METER_NO_TPL = 0x70,    /*!< Application error from Meter without Transport Layer */
  CI_ALARM_FROM_METER_NO_TPL = 0x71,         /*!< Alarm from Meter without Transport Layer */
  CI_EN13757_3_LONG_TPL = 0x72,              /*!< EN 13757-3 Application Layer with long Transport Layer */
  CI_EN13757_3_CF_LONG_TPL = 0x73,           /*!< EN 13757-3 Application Layer with Compact frame and long Transport Layer */
  CI_ALARM_FROM_METER_SHORT_TPL = 0x74,      /*!< Alarm from Meter with short Transport Layer */
  CI_ALARM_FROM_METER_LONG_TPL = 0x75,       /*!< Alarm from Meter with long Transport Layer */
  CI_EN13757_3_NO_TPL = 0x78,                /*!< EN 13757-3 Application Layer without Transport Layer */
  CI_EN13757_3_CF_NO_TPL = 0x79,             /*!< EN 13757-3 Application Layer with Compact frame without Transport Layer */
  CI_EN13757_3_SHORT_TPL = 0x7A,             /*!< EN 13757-3 Application Layer with short Transport Layer */
  CI_EN13757_3_CF_SHORT_TPL = 0x7B,          /*!< EN 13757-3 Application Layer with Compact frame and short Transport Layer */
  CI_COSEM_APPL_LONG_TPL = 0x7C,             /*!< COSEM Application Layer with long Transport Layer */
  CI_COSEM_APPL_SHORT_TPL = 0x7D,            /*!< COSEM Application Layer with short Transport Layer */
  CI_OBIS_APPL_LONG_TPL = 0x7E,              /*!< OBIS Application Layer with long Transport Layer */
  CI_OBIS_APPL_SHORT_TPL = 0x7F,             /*!< OBIS Application Layer with short Transport Layer */
  CI_EN13757_3_TO_METER_LONG_TPL = 0x80,     /*!< EN 13757-3 Transport Layer (long) from Readout device to the Meter */
  CI_NETWORK_LAYER = 0x81,                   /*!< Network Layer data */
  CI_RESERVED = 0x82,                        /*!< Reserved for future use */
  CI_NETWORK_MNGMNT = 0x83,                  /*!< Network Management application */
  CI_TPL_TO_METER_CF = 0x84,                 /*!< Transport layer to Meter (M Bus-Compact frame expected) */
  CI_TPL_TO_METER_FF = 0x85,                 /*!< Transport layer to Meter (M Bus-Format frame expected) */
  CI_RESERVED_NETWORK_MNGMT = 0x89,          /*!< Reserved for Network management data (EN13757-5) */
  CI_EN13757_3_SHORT_TPL_FROM_METER = 0x8A,  /*!< EN 13757-3 Transport Layer (short) from the Meter to the Readout device */
  CI_EN13757_3_LONG_TPL_FROM_METER = 0x8B,   /*!< EN 13757-3 Transport Layer (long) from the Meter to the Readout device */
  CI_EXTENDED_LINK_LAYER_I = 0x8C,           /*!< Extended link layer: 2 Bytes */
  CI_EXTENDED_LINK_LAYER_II = 0x8D,          /*!< Extended link layer: 8 Bytes */
  CI_EXTENDED_LINK_LAYER_III = 0x8E,         /*!< Extended link layer: 10 Bytes */
  CI_EXTENDED_LINK_LAYER_IV = 0x8F           /*!< Extended link layer: 16 Bytes */
} wMBus_DataLink_CI_field_t;
/**
  * @}
  */

/** @defgroup DataLink_Exported_Functions DataLink Exported Functions
  * @{
  */
/**
  * @brief  Initializes the wM-Bus Link Layer.
  * @param  m_field Pointer to the M-field attribute.
  * @param  a_field Pointer to the A-field attribute.
  * @param  dev_type Device type.
  * @param  submode Submode of the communication.
  * @param  frame_format Frame format type.
  * @param  direction Direction of the communication.
  * @retval wMBus_DataLink_status_t Status of the initialization.
  */
wMBus_DataLink_status_t wMBus_DataLink_init(uint8_t *m_field, uint8_t *a_field, wMBus_Phy_meter_modes_t dev_type, wMBus_Phy_submode_t submode, wMBus_Phy_frame_format_t frame_format, wMBus_Phy_direction_t direction);

/**
  * @brief  State machine for the wM-Bus DataLink layer.
  * @retval None
  */
void wMBus_DataLink_state_machine(void);

/**
  * @brief  Sends a primary frame based on the C-field for Primary station.
  * @param  c_field: C-field of the primary station.
  * @retval None
  */
void wMBus_DataLink_send_frame(wMBus_DataLink_C_field_t c_field, wMBus_DataLink_CI_field_t ci_field, uint8_t *data, uint8_t length, uint8_t **rxBuf, uint16_t *rxBufferLen);

#ifdef PROCESS_FRAME
/**
 * @brief Receives a frame using the wM-Bus DataLink layer and parses it into a structured frame.
 * @param frame Pointer to a wMBus_DataLink_frame_t structure where the received frame will be stored.
 * @return wMBus_DataLink_status_t Status of the frame reception and parsing. 
 */
wMBus_DataLink_status_t wMBus_DataLink_receive_frame(wMBus_DataLink_frame_t *frame);
#else
/**
 * @brief Receives a frame using the wM-Bus DataLink layer and provides the raw buffer.
 * @param rxBuf Pointer to a pointer where the raw received buffer will be stored.
 * @param rxBufferLen Pointer to a variable where the length of the raw received buffer will be stored.
 * @return wMBus_DataLink_status_t Status of the frame reception.
 */
wMBus_DataLink_status_t wMBus_DataLink_receive_frame(uint8_t **rxBuf, uint16_t *rxBufferLen);
#endif

/**
  * @brief Set DataLink layer to start receiving data.
  * @return wMBus_DataLink_status_t status of the operation.
  */
wMBus_DataLink_status_t wMBus_DataLink_startRx(void);

/**
  * @brief Callback function invoked to send a CNF-IR frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a CNF-IR frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_CNF_IR(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked to send an ACK frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send an ACK frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_ACK(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked to send a RSP-UD frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a RSP-UD frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_RSP_UD(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked when a SND-NR frame is received.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior when a SND-NR frame is received.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_C_field_t wMBus_DataLink_received_SND_NR(void);

/**
  * @brief Callback function invoked when a REQ-UD1 frame is received.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior when a REQ-UD1 frame is received.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_C_field_t wMBus_DataLink_received_REQ_UD1(void);

/**
  * @brief Callback function invoked to send a SND-UD frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a SND-UD frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_SND_UD(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked to send a SND-UD2 frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a SND-UD2 frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_SND_UD2(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked to send a SND-NKE frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a SND-NKE frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_SND_NKE(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked to send a REQ-UD1 frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a REQ-UD1 frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_REQ_UD1(uint8_t **rxBuf, uint16_t *rxBufferLen);

/**
  * @brief Callback function invoked to send a REQ-UD2 frame.
  * @param rxBuf Pointer to the raw received buffer.
  * @param rxBufferLen Length of the raw received buffer.
  * @return wMBus_DataLink_status_t Status of the operation.
  * @note This function is weakly defined and can be overridden by the user
  *       to implement custom behavior to send a REQ-UD1 frame.
  * @attention Ensure that the callback implementation does not block for
  *            extended periods, as it may affect the timing of the wM-Bus
  *            DataLink layer.
  */
wMBus_DataLink_status_t wMBus_DataLink_send_REQ_UD2(uint8_t **rxBuf, uint16_t *rxBufferLen);
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

#endif /* STM32WL3_WMBUS_DATALINK_H */
