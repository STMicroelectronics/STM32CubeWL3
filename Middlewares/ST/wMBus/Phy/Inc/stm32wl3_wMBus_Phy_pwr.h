/**
  ******************************************************************************
  * @file    stm32wl3_wMBus_Phy_pwr.h
  * @author  MCD Application Team
  * @brief   Header file of wM-Bus Phy pwr module.
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
#ifndef WL3_WMBUS_PHY_PWR_H
#define WL3_WMBUS_PHY_PWR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x_hal_def.h"
#include "stm32wl3x_ll_pwr.h"

/** @addtogroup STM32WL3_wM-Bus
  * @{
  */

/** @addtogroup Phy
  * @{
  */

/** @addtogroup Pwr
  * @{
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup Pwr_Exported_Functions  Pwr Exported Functions
  * @{
  */
#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
/**
  * @brief  Configure the SMPS output voltage and BOM.
  * @note   This function is defined only if WMBUS_ACTIVE_POWER_MODE_ENABLED is defined.
  * @param  outputVoltage SMPS output voltage.
  * @param  BOM           SMPS BOM.
  * @retval HAL_StatusTypeDef Status of the operation (see @ref HAL_StatusTypeDef).
  */
HAL_StatusTypeDef HAL_PWREx_ConfigSMPS_Update(uint32_t outputVoltage, uint32_t BOM);

/**
  * @brief  Set BOF_CUR_SEL (bit 14:15) Bypass On the Fly current limitation in PWRC_DBGSMPS register.
  * @note   This function is defined only if WMBUS_ACTIVE_POWER_MODE_ENABLED is defined
  * @param  current is the new current: 00 : 20mA, 01 : 40mA, 10 : 60mA (default), 11 : no limit
  * @retval None.
  */
__STATIC_INLINE void LL_PWR_DBGSMPS_Current_Selection(uint8_t current)
{
  uint32_t register_temp;

  register_temp = READ_REG(PWR->RESERVED3);

  switch (current)
  {
  case 0:
    /* clear b15 & b14 */
    register_temp &= 0xFFFF3FFF;
    register_temp |= 0x00000000;
    WRITE_REG(PWR->RESERVED3, register_temp);
    break;
  case 1:
    /* clear b15 & set b14 */
    register_temp &= 0xFFFF3FFF;
    register_temp |= 0x00004000;
    WRITE_REG(PWR->RESERVED3, register_temp);
    break;
  case 2:
    /* set b15 & clear b14 */
    register_temp &= 0xFFFF3FFF;
    register_temp |= 0x00008000;
    WRITE_REG(PWR->RESERVED3, register_temp);
    break;
  case 3:
    /* set b15 & set b14 */
    register_temp &= 0xFFFF3FFF;
    register_temp |= 0x0000C000;
    WRITE_REG(PWR->RESERVED3, register_temp);
    break;
  default:
    break;
  }
}
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

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* WL3_WMBUS_PHY_PWR_H */
