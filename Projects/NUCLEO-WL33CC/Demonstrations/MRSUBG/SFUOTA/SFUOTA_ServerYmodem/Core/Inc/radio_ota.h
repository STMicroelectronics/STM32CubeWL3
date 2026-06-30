/**
  ******************************************************************************
  * @file    radio_ota.h
  * @author  GPAM Wireless Application Team
  * @brief   OTA utilities for the WL33 SFUOTA application.
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
#ifndef RADIO_OTA_H
#define RADIO_OTA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "stm32wl3x_hal.h"
#include "main.h"
#include "stm32wl3x_hal_flash.h"
#include "app_conf.h"
#include "system_stm32wl3x.h"
#include "ymodem.h"

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  OTA protocol state-machine states.
  */
typedef enum
{
  OTA_CONNECTION = 0,  /*!< Connection handshake state */
  OTA_SIZE = 1,        /*!< Image-size exchange state */
  OTA_START = 2,       /*!< Download-start state */
  OTA_DATAREQ = 3,     /*!< Data-request state */
  OTA_GETDATA = 4,     /*!< Data-reception state */
  OTA_SENDATA = 4,     /*!< Data-send acknowledgement state */
  OTA_FLASHDATA = 5,   /*!< Flash-programming state */
  OTA_COMPLETE = 6,    /*!< Download-complete state */
  OTA_ONGOING = 7,     /*!< Transfer-ongoing state */
} ota_state_machine_t;

/**
  * @brief  Ymodem sub-state machine states used by the OTA server.
  */
typedef enum
{
  YMODEM_SIZE_STATE = 0U,      /*!< Receive filename/header packet and extract image size. */
  YMODEM_LOAD_STATE = 1U,      /*!< Receive payload data packets. */
  YMODEM_WAIT_STATE = 2U,      /*!< Wait for OTA side to consume staged payload. */
  YMODEM_CLOSE_STATE = 4U,     /*!< Final payload chunk reached, prepare transfer closure. */
  YMODEM_COMPLETE_STATE = 5U,  /*!< Ymodem transfer completed successfully. */
  YMODEM_ABORT_STATE = 9U,     /*!< Ymodem transfer aborted due to protocol error. */
} ymodem_state_t;

/**
  * @brief  Ownership token shared between OTA and Ymodem state machines.
  */
typedef enum
{
  TOKEN_RELEASE = 0,       /*!< Shared context is free. */
  TOKEN_TAKEN_YMODEM = 1,  /*!< Shared context is owned by Ymodem processing. */
  TOKEN_TAKEN_OTA = 2,     /*!< Shared context is owned by OTA radio processing. */
} ota_app_token_t;

/* Exported macros -----------------------------------------------------------*/
#define RADIO_OTA_MANAGER_VERSION_STRING             "1.0.0"     /*!< OTA protocol version */

/* OTA protocol transport limits */
#define MAX_PACKET_LENGTH                           (255U)      /*!< Max OTA packet length */
#define MAX_RETRY                                   (10U)       /*!< Max retry count */
#define BYTE_IN_FRAME                               (16U)       /*!< Data bytes per frame */

/* OTA protocol frame headers */
#define HEADER_CONNECTION                          (0xA0U)      /*!< Connection header */
#define HEADER_SIZE                                (0xB0U)      /*!< Size header */
#define HEADER_START                               (0xC0U)      /*!< Start header */
#define HEADER_DATAREQ                             (0xD0U)      /*!< Data-request header */
#define HEADER_GETDATA                             (0xE0U)      /*!< Data frame header */
#define HEADER_SENDATA                             (0xE0U)      /*!< Data ACK/completion header */
#define HEADER_NOTSTART                            (0xF0U)      /*!< Not-start header */

/* Flash layout and sizing helpers */
#define PAGE_SIZE_ROUND(size)                        (FLASH_PAGE_SIZE * (((size) + FLASH_PAGE_SIZE - 1U) / FLASH_PAGE_SIZE))  /*!< Round size up to page */
#define PAGE_SIZE_TRUNC(size)                        (FLASH_PAGE_SIZE * ((size) / FLASH_PAGE_SIZE))                           /*!< Truncate size down to page */

#define SERVICE_MANAGER_OFFSET                       (0x0UL)                                                                  /*!< OTA manager base offset */
#define SERVICE_MANAGER_SIZE                         (PAGE_SIZE_ROUND((25U * 1024U)))                                         /*!< OTA manager image size */
#define NVM_SIZE                                     (PAGE_SIZE_ROUND((4U * 1024U)))                                          /*!< Reserved NVM area */

#define SM_APP_OFFSET                                (SERVICE_MANAGER_OFFSET + SERVICE_MANAGER_SIZE)                          /*!< Downloadable app offset */
#define APP_WITH_OTA_SERVICE_ADDRESS                 (_MEMORY_FLASH_BEGIN_ + SM_APP_OFFSET)                                   /*!< Downloadable app base address */
#define APP_OTA_SERVICE_ADDRESS                      (_MEMORY_FLASH_BEGIN_ + SERVICE_MANAGER_OFFSET)                          /*!< OTA manager base address */

#define SM_APP_SIZE                                  PAGE_SIZE_TRUNC((_MEMORY_FLASH_SIZE_ - SERVICE_MANAGER_SIZE - NVM_SIZE))         /*!< Max downloadable app size */
#define APP_WITH_OTA_SERVICE_PAGE_NUMBER_START       (SM_APP_OFFSET / FLASH_PAGE_SIZE)                                                /*!< First flash page of downloadable app */
#define APP_WITH_OTA_SERVICE_PAGE_NUMBER_END         (APP_WITH_OTA_SERVICE_PAGE_NUMBER_START + (SM_APP_SIZE / FLASH_PAGE_SIZE) - 1U)  /*!< Last flash page of downloadable app */

/* Boot-operation retained markers */
#define OTA_SWITCH_TO_OTA_RESET_MANAGER             (0xB0U)      /*!< Retained command to boot OTA manager */

/* OTA image validation tags */
#define OTA_IN_PROGRESS_TAG                        (0xFFFFFFFFUL)   /*!< Tag set while a new image is being written */
#define OTA_INVALID_OLD_TAG                        (0x00000000UL)   /*!< Tag marking previous app image as invalid */
#define OTA_VALID_TAG                              (0xAA5555AAUL)   /*!< Tag marking downloaded app image as bootable */
#define OTA_SERVICE_MANAGER_TAG                    (0xAABBCCDDUL)   /*!< Tag identifying OTA service manager image */

/* Vector-table slot used to store OTA tag state */
#define OTA_TAG_VECTOR_TABLE_ENTRY_INDEX           (4U)                                                                       /*!< Vector-table word index used as OTA image tag */
#define OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET          (OTA_TAG_VECTOR_TABLE_ENTRY_INDEX * 4U)                                    /*!< Byte offset of OTA tag from image base address */
#define TAG_VALUE(address)                         (*((volatile uint32_t *)((address) + OTA_TAG_VECTOR_TABLE_ENTRY_OFFSET)))  /*!< Read OTA tag word from app base address */

/* Exported functions --------------------------------------------------------*/
void preset_Ymodem_image(void);

/**
  * @brief  Initialize the OTA state machine and its protocol context.
  * @retval 0U if the OTA initialization completes successfully.
  * @retval Non-zero value if the OTA module cannot be initialized.
  */
uint8_t OTA_Init(void);

/**
  * @brief  Execute one OTA state-machine step.
  * @retval 0U if the state-machine step completes successfully.
  * @retval Non-zero value if the OTA transfer detects a protocol or flash-processing error.
  */
uint8_t OTA_Tick(void);

/**
  * @brief  Execute one Ymodem state-machine step.
  * @retval 0 when the step is processed normally.
  * @retval Non-zero Ymodem status/error code when a terminal condition is reached.
  */
int32_t OTA_ymodem_tick(void);

/**
  * @brief  Forward MRSubG IRQ events to OTA radio handling.
  */
void OTA_Radio_IRQHandler(void);

/**
  * @brief  Check whether the resident application must switch execution to OTA management mode.
  * @note   This function interprets the boot-operation marker stored in retained RAM.
  */
void OTA_Check_ServiceManager_Operation(void);

/**
  * @brief  Check the validity tags stored in the resident and downloadable application vector tables.
  * @retval OTA_VALID_TAG if a valid downloadable image is present.
  * @retval OTA_SERVICE_MANAGER_TAG if the resident management application image is selected as valid.
  * @retval Any other tag value when no bootable downloadable image is available.
  */
uint32_t OTA_Check_Application_Tags_Value(void);

/**
  * @brief  Jump to the selected application after final tag validation.
  * @note   This function transfers execution either to the resident management application image or to the downloaded application image.
  */
void OTA_Jump_To_New_Application(void);

/**
  * @brief  Get current OTA protocol state-machine status.
  * @retval Current value of @ref ota_state_machine_t.
  */
ota_state_machine_t OTA_GetStatus(void);

/**
  * @brief  Get current ownership token between OTA and Ymodem state machines.
  * @retval Current token value from @ref ota_app_token_t.
  */
uint8_t OTA_Get_App_Token_Status(void);

/**
  * @brief  Push received UART bytes into the Ymodem RX ring/buffer.
  * @param  data_buffer Pointer to the received byte buffer.
  * @param  nb_bytes Number of bytes available in @p data_buffer.
  */
void processInputData(uint8_t *data_buffer, uint16_t nb_bytes);

#ifdef __cplusplus
}
#endif

#endif /* RADIO_OTA_H */