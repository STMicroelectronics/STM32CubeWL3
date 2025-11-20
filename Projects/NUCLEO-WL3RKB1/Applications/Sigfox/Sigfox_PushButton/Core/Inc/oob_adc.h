/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    oob_adc.h
  * @author  GPM WBL Application Team
  * @brief   Header for oob_adc module
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

#ifndef __OOB_ADC_H
#define __OOB_ADC_H

/**
 * @brief  Initialize ADC for Sigfox out-of-band frame voltage and temperature measurements.
 * @retval None
 */
void OOB_ADC_Init(void);
float APP_GetBatteryVoltageMillivolts(void);
int16_t APP_GetTemperatureTenthCelsius(void);

#endif
