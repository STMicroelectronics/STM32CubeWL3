/**
  ************************************************************************************
  * @file    crc_4wkup_rf.h
  * @author  RF Application Team
  * @brief   This file contains CRC utils for generating packets for the Wakeup Radio.
  *
  ************************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics. 
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the 
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

#ifndef CRC_4WKUP_RF_H
#define CRC_4WKUP_RF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define CRC_POLYNOM         0x8005
#define PAYLOAD_LEN              7

/**
* @brief  function reflecting bits of 16-bit word                 
*/
uint8_t Reflected8(uint8_t input);

/**
* @brief  function reflecting bits of 16-bit word                 
*/
uint16_t Reflected16(uint16_t input);

/**
* @brief  Calculates the 16-bit CRC. The function requires
*         that the CRC_POLYNOM is defined,
*           which gives the wanted CRC polynom. 
* @param  crcData  - Data to perform the CRC-16 operation on.
*         crcReg   - Current or initial value of the CRC calculation
* @retval crcReg Current or initial value of the CRC calculation                   
*/
uint16_t crcCalc(uint16_t crcReg, uint8_t crcData);

void EvaluateCrc(uint8_t * LPAWUR_payload); 

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
