/**
  ******************************************************************************
  * @file    sfuota_boot.h
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA image validity tags and application startup handover.
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
#ifndef SFUOTA_BOOT_H
#define SFUOTA_BOOT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "sfuota_flashmap.h"

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Mark the downloaded image as bootable.
  * @note   This is the single point where image validity is granted.
  */
void SFUOTA_Boot_MarkImageValid(void);

/**
  * @brief  Program the SFUOTA validity tag at the application tag vector-table offset.
  * @param  address Base address of the application image.
  * @param  data Tag value to program.
  */
void SFUOTA_Boot_SetTag(uint32_t address, uint32_t data);

/**
  * @brief  Check whether the resident application must switch execution to SFUOTA management mode.
  * @note   This function interprets the boot-operation marker stored in retained RAM.
  */
void SFUOTA_Boot_CheckRequest(void);

/**
  * @brief  Check the validity tags stored in the resident and downloadable application vector tables.
  * @retval Base address of the application image to run.
  * @retval 0U when no valid application target is available.
  */
uint32_t SFUOTA_Boot_SelectImage(void);

/**
  * @brief  Jump to the selected application after final tag validation.
  * @note   Execution is transferred either to the resident management application
  *         image or to the downloaded application image.
  */
void SFUOTA_Boot_JumpToApplication(void);

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_BOOT_H */
