/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    wl3sfx_nvm.c
  * @author  GPM WBL Application Team
  * @brief   Nonvolatile memory (NVM) contents.
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

#include "wl3sfx_nvm.h"

NO_INIT_SECTION(const uint8_t wl3sfx_nvm_boarddata[FLASH_PAGE_SIZE], ".noinit.sigfox_boarddata");
NO_INIT_SECTION(const uint8_t wl3sfx_nvm_records[FLASH_PAGE_SIZE], ".noinit.sigfox_nvm_records");
NO_INIT_SECTION(const uint8_t wl3sfx_nvm_records_backup[FLASH_PAGE_SIZE], ".noinit.sigfox_nvm_records_backup");
