/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    wl3sfx_oob.c
  * @author  GPM WBL Application Team
  * @brief   Utilities for retrieving measurements required for Sigfox out-of-band frames (temperature / battery voltage).
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include <stdint.h>
#include "stm32wl3x_hal.h"
#include "wl3sfx_oob.h"

static uint16_t _batt_voltage_during_tx;

/*  -------------------- Weak symbols: To be implemented by application! -------------------- */
__weak uint32_t APP_GetBatteryVoltageMillivolts(void)
{
  return 0;
}

__weak int16_t APP_GetTemperatureTenthCelsius(void)
{
  return 0;
}

/*  ---------------------------- WL3SFX Out-of-Band API functions ---------------------------- */
void wl3sfx_oob_update_tx_voltage(void)
{
  _batt_voltage_during_tx = APP_GetBatteryVoltageMillivolts();
}

void wl3sfx_oob_get_voltage_temperature(uint16_t* voltage_idle, uint16_t* voltage_tx, int16_t* temperature)
{
  // Measuring battery voltage for idle state when this function is called is good enough
  *voltage_idle = APP_GetBatteryVoltageMillivolts();
  *voltage_tx = _batt_voltage_during_tx;
  *temperature = APP_GetTemperatureTenthCelsius();
}
