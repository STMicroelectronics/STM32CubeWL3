/**
  ******************************************************************************
  * @file    sfuota_radio.h
  * @author  GPAM Wireless Application Team
  * @brief   MRSubG PHY adaptation layer for the WL33 SFUOTA applications.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SFUOTA_RADIO_H
#define SFUOTA_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal.h"
#include "app_conf.h"

/* Exported macros -----------------------------------------------------------*/
#define SFUOTA_RADIO_MAX_PACKET                  (255U)  /*!< Max radio packet length handled by this layer. */

/* Hardware-independent reception error causes reported to the protocol layer. */
#define SFUOTA_RADIO_ERR_CRC    (0x01U)  /*!< Frame discarded because of a CRC mismatch. */
#define SFUOTA_RADIO_ERR_OTHER  (0x02U)  /*!< Frame discarded because of any other radio failure. */

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  Radio availability reported to the protocol layer before a transmission.
  */
typedef enum
{
  SFUOTA_RADIO_READY = 0,  /*!< Radio can start a transmission immediately. */
  SFUOTA_RADIO_BUSY = 1,   /*!< Transmission or receive abort still in progress, retry on next tick. */
} sfuota_radio_status_t;

/**
  * @brief  Radio events latched in interrupt context and consumed in task context.
  */
typedef struct
{
  uint8_t rx_ok;           /*!< A packet has been received. */
  uint8_t tx_done;         /*!< The pending transmission has completed. */
  uint8_t rx_error;        /*!< A reception error has been reported. */
  uint8_t rx_error_flags;  /*!< Combination of SFUOTA_RADIO_ERR_* describing the failure. */
  const uint8_t *rx_data;  /*!< Received packet buffer, valid while @ref rx_ok is set. */
  uint16_t rx_length;      /*!< Number of bytes available in @ref rx_data. */
} sfuota_radio_event_t;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Enable the radio IRQ sources used by SFUOTA.
  * @note   Reception is not armed here: the caller arms it through SFUOTA_Radio_EnsureReceiving().
  */
void SFUOTA_Radio_Init(void);

/**
  * @brief  Drop pending radio events and clear the local scheduling state.
  */
void SFUOTA_Radio_Reset(void);

/**
  * @brief  Latch the MRSubG IRQ sources for deferred processing.
  * @note   Must be called from the MRSubG interrupt handler.
  */
void SFUOTA_Radio_IRQHandler(void);

/**
  * @brief  Collect and clear the radio events latched since the previous call.
  * @note   Also recovers from a missed TX completion interrupt.
  * @param  events Destination event structure.
  */
void SFUOTA_Radio_GetEvents(sfuota_radio_event_t *events);

/**
  * @brief  Check whether the radio can start a transmission, aborting reception if needed.
  * @retval SFUOTA_RADIO_READY when @ref SFUOTA_Radio_Transmit can be called.
  * @retval SFUOTA_RADIO_BUSY when the caller must retry on the next tick.
  */
sfuota_radio_status_t SFUOTA_Radio_PrepareTx(void);

/**
  * @brief  Transmit one packet.
  * @param  packet Packet payload.
  * @param  length Packet length in bytes.
  */
void SFUOTA_Radio_Transmit(const uint8_t *packet, uint8_t length);

/**
  * @brief  Arm reception when the radio is otherwise idle.
  */
void SFUOTA_Radio_EnsureReceiving(void);

/**
  * @brief  Report whether a transmission is still ongoing.
  * @retval Non-zero while the radio is transmitting.
  */
uint8_t SFUOTA_Radio_IsTxBusy(void);

/**
  * @brief  Report whether reception is currently armed.
  * @retval Non-zero while the receiver is armed.
  */
uint8_t SFUOTA_Radio_IsRxArmed(void);

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_RADIO_H */
