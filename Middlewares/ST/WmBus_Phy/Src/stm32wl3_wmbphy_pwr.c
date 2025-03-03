/**
 ******************************************************************************
 * @file    stm32wl3_wmbphy_pwr.c
 * @author  MCD Application Team
 * @brief   WMBUS PHY PWR module driver.
 *          This file provides firmware functions to manage the Power module
 *          of the WMBUS PHY driver
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
#include "stm32wl3_wmbphy_pwr.h"

/** @addtogroup STM32WL3_WMBUS_PHY
 * @{
 */

/** @addtogroup PWR
 * @{
 */
/* Exported functions --------------------------------------------------------*/
/** @addtogroup PWR_Exported_Functions  PWR Exported Functions
 * @{
 */
#ifdef WMBUS_ACTIVE_POWER_MODE_ENABLED
/**
 * @brief  Configure the SMPS output voltage and BOM.
 * @note   This function is defined only if WMBUS_ACTIVE_POWER_MODE_ENABLED is defined
 * @param  outputVoltage: SMPS output voltage.
 * @param  BOM: SMPS BOM.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PWREx_ConfigSMPS_Update(uint32_t outputVoltage, uint32_t BOM) {
    /* Check the parameter */
    assert_param(IS_PWR_SMPS_OUTPUT_VOLTAGE(outputVoltage));
    assert_param(IS_PWR_SMPS_BOM(BOM));

    /* follow Reference manual procedure 5.8.2 */
    /* set precharge mode = 1 */
    LL_PWR_SetSMPSPrechargeMode(LL_PWR_SMPS_PRECHARGE);

    /* wait for SMPS Ready flag */
    while (LL_PWR_IsSMPSReady() != 0);

    /* now modify SMPS level */
    LL_PWR_SMPS_SetOutputVoltageLevel(outputVoltage);
    LL_PWR_SetSMPSBOM(BOM);

    /* clear precharge mode = 0 */
    LL_PWR_SetSMPSPrechargeMode(LL_PWR_NO_SMPS_PRECHARGE);

    /* wait for SMPS Ready flag */
    while (LL_PWR_IsSMPSReady() != 1);

    return HAL_OK;
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
