/**
  ******************************************************************************
  * @file    sfuota_boot.c
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

/* Includes ------------------------------------------------------------------*/
#include "sfuota_boot.h"

/* Private define -----------------------------------------------------------*/
#define CFG_OTA_REBOOT_VAL_MSG RAM_VR.bootloader_vr  /*!< Mapping of SFUOTA retained message in SRAM. */

/* Private variables --------------------------------------------------------*/
/* Static storage: after __set_MSP the caller stack frame is no longer addressable. */
static uint32_t app_stack_pointer;
static void (*app_reset_handler)(void);

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Marks the downloaded image as bootable.
  * @param  None.
  * @retval None.
  */
void SFUOTA_Boot_MarkImageValid(void)
{
  SFUOTA_Boot_SetTag(SFUOTA_MAP_APP_ADDR, SFUOTA_TAG_VALID);
}

/**
  * @brief  Programs SFUOTA validity tag at the application tag vector-table offset.
  * @param  address Base address of the application image.
  * @param  data Tag value to program.
  * @retval None.
  */
void SFUOTA_Boot_SetTag(uint32_t address, uint32_t data)
{
  /* Program tag word using the WL3 FLASH HAL API. */
  (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + SFUOTA_TAG_OFFSET, data);
}

/**
  * @brief  Handles retained boot command for SFUOTA service manager request.
  *
  * If retained RAM requests SFUOTA service manager boot, this function invalidates the
  * downloadable application tag and clears the retained request marker.
  *
  * @param  None.
  * @retval None.
  */
void SFUOTA_Boot_CheckRequest(void)
{
  if (CFG_OTA_REBOOT_VAL_MSG == SFUOTA_BOOT_REQ_MANAGER)
  {
    /* Set invalid tag for the SFUOTA application to allow jumping to SFUOTA service manager. */
    SFUOTA_Boot_SetTag(SFUOTA_MAP_APP_ADDR, SFUOTA_TAG_INVALID);

    /* Reset the retained service manager request after it has been consumed. */
    CFG_OTA_REBOOT_VAL_MSG = 0U;
  }
}

/**
  * @brief  Selects the boot target by checking SFUOTA validity tags.
  *
  * Compares service-manager and downloadable-application tags and returns the
  * base address of the image to run.
  *
  * @param  None.
  * @retval Base address of selected application image.
  * @retval 0U when no valid application target is available.
  */
uint32_t SFUOTA_Boot_SelectImage(void)
{
  uint32_t appAddress = 0U;

  /* Manager present and downloaded image not usable yet: run the manager. */
  if (((SFUOTA_TAG_AT(SFUOTA_MAP_MANAGER_ADDR) == SFUOTA_TAG_MANAGER) &&
       (SFUOTA_TAG_AT(SFUOTA_MAP_APP_ADDR) == SFUOTA_TAG_IN_PROGRESS)) ||
      ((SFUOTA_TAG_AT(SFUOTA_MAP_MANAGER_ADDR) == SFUOTA_TAG_MANAGER) &&
       (SFUOTA_TAG_AT(SFUOTA_MAP_APP_ADDR) == SFUOTA_TAG_INVALID)))
  {
    appAddress = SFUOTA_MAP_MANAGER_ADDR;
  }
  /* Manager present and downloaded image valid: run the downloaded image. */
  else if ((SFUOTA_TAG_AT(SFUOTA_MAP_MANAGER_ADDR) == SFUOTA_TAG_MANAGER) &&
           (SFUOTA_TAG_AT(SFUOTA_MAP_APP_ADDR) == SFUOTA_TAG_VALID))
  {
    /* Jump to application using SFUOTA service manager. */
    appAddress = SFUOTA_MAP_APP_ADDR;
  }

  return appAddress;
}

/**
  * @brief  Jumps to the selected valid application image.
  * @param  None
  * @retval None
  */
void SFUOTA_Boot_JumpToApplication(void)
{
  uint32_t applicationAddress = SFUOTA_Boot_SelectImage();

  /* Abort if no valid application address is available. */
  if (applicationAddress == 0U)
  {
    return;
  }

  /* Read initial stack pointer and reset handler from application vector table. */
  app_stack_pointer = *((volatile uint32_t *)applicationAddress);
  app_reset_handler = (void (*)(void))(*((volatile uint32_t *)(applicationAddress + 4U)));

  /* Stop current runtime context before handing control to the application. */
  __disable_irq();
  HAL_DeInit();
  SysTick->CTRL = 0U;
  SysTick->LOAD = 0U;
  SysTick->VAL = 0U;
  NVIC->ICER[0] = 0xFFFFFFFFU;
  NVIC->ICPR[0] = 0xFFFFFFFFU;

  /* Switch vector table and stack pointer, then jump to application reset handler. */
  RAM_VR.AppBase = applicationAddress;
  SCB->VTOR = applicationAddress;
  __set_MSP(app_stack_pointer);
  app_reset_handler();
}
