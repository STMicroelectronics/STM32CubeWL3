/**
  ******************************************************************************
  * @file    sfuota_flashmap.h
  * @author  GPAM Wireless Application Team
  * @brief   Flash layout and image validity tags of the SFUOTA target device.
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
#ifndef SFUOTA_FLASHMAP_H
#define SFUOTA_FLASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal.h"
#include "stm32wl3x_hal_flash.h"
#include "system_stm32wl3x.h"

/* Exported macros -----------------------------------------------------------*/
/* Flash layout and sizing helpers */
#define SFUOTA_PAGE_ROUND(size)  (FLASH_PAGE_SIZE * (((size) + FLASH_PAGE_SIZE - 1U) / FLASH_PAGE_SIZE))  /*!< Round size up to page */
#define SFUOTA_PAGE_TRUNC(size)  (FLASH_PAGE_SIZE * ((size) / FLASH_PAGE_SIZE))                           /*!< Truncate size down to page */

#define SFUOTA_MAP_MANAGER_OFFSET  (0x0UL)                             /*!< SFUOTA service manager base offset */
#define SFUOTA_MAP_MANAGER_SIZE    (SFUOTA_PAGE_ROUND((27U * 1024U)))  /*!< SFUOTA service manager image size */
#define SFUOTA_MAP_NVM_SIZE        (SFUOTA_PAGE_ROUND((4U * 1024U)))   /*!< Reserved NVM area */

#define SFUOTA_MAP_APP_OFFSET    (SFUOTA_MAP_MANAGER_OFFSET + SFUOTA_MAP_MANAGER_SIZE)  /*!< Downloadable app offset */
#define SFUOTA_MAP_APP_ADDR      (_MEMORY_FLASH_BEGIN_ + SFUOTA_MAP_APP_OFFSET)         /*!< Downloadable app base address */
#define SFUOTA_MAP_MANAGER_ADDR  (_MEMORY_FLASH_BEGIN_ + SFUOTA_MAP_MANAGER_OFFSET)     /*!< SFUOTA service manager base address */

#define SFUOTA_MAP_APP_SIZE        SFUOTA_PAGE_TRUNC((_MEMORY_FLASH_SIZE_ - SFUOTA_MAP_MANAGER_SIZE - SFUOTA_MAP_NVM_SIZE))  /*!< Max downloadable app size */
#define SFUOTA_MAP_APP_PAGE_FIRST  (SFUOTA_MAP_APP_OFFSET / FLASH_PAGE_SIZE)  /*!< First flash page of downloadable app */
#define SFUOTA_MAP_APP_PAGE_LAST   (SFUOTA_MAP_APP_PAGE_FIRST + (SFUOTA_MAP_APP_SIZE / FLASH_PAGE_SIZE) - 1U)  /*!< Last flash page of downloadable app */

/* Boot-operation retained markers */
#define SFUOTA_BOOT_REQ_MANAGER  (0xB0U)  /*!< Retained command to boot SFUOTA service manager */

/* SFUOTA image validation tags */
#define SFUOTA_TAG_IN_PROGRESS  (0xFFFFFFFFUL)  /*!< Tag set while a new image is being written */
#define SFUOTA_TAG_INVALID      (0x00000000UL)  /*!< Tag marking previous app image as invalid */
#define SFUOTA_TAG_VALID        (0xAA5555AAUL)  /*!< Tag marking downloaded app image as bootable */
#define SFUOTA_TAG_MANAGER      (0xAABBCCDDUL)  /*!< Tag identifying SFUOTA service manager image */

/* Vector-table slot used to store SFUOTA tag state */
#define SFUOTA_TAG_VT_INDEX     (4U)                                                       /*!< Vector-table word index used as SFUOTA image tag */
#define SFUOTA_TAG_OFFSET       (SFUOTA_TAG_VT_INDEX * 4U)                                 /*!< Byte offset of SFUOTA tag from image base address */
#define SFUOTA_TAG_AT(address)  (*((volatile uint32_t *)((address) + SFUOTA_TAG_OFFSET)))  /*!< Read SFUOTA tag word from app base address */

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_FLASHMAP_H */
