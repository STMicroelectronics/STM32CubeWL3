/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    crc_4wkup_rf.c
  * @author  GPM WBL Application Team
  * @brief   This code implements a bidirectional point to point communication.
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

/* Includes ------------------------------------------------------------------*/
#include "crc_4wkup_rf.h"

/**
* @brief  function reflecting bits of 16-bit word                 
*/
uint8_t Reflected8(uint8_t input)
{
  uint8_t reflecting_8bit=0;
  uint8_t temp =0;
  uint8_t index =0;
  
  for (index =0 ; index<8 ;index++)
  {
    /* isolate bit to reflect */
    temp = 0;
    temp = (input >> index);
    temp = temp & 0x01;
    
    /* set bit into reflecting_16bit */
    reflecting_8bit = reflecting_8bit | (temp << (7-index));
  }

  return reflecting_8bit;
}

/**
* @brief  function reflecting bits of 16-bit word                 
*/
uint16_t Reflected16(uint16_t input)
{
  uint16_t reflecting_16bit=0;
  uint16_t temp =0;
  uint8_t index =0;
  
  for (index =0 ; index<16 ;index++)
  {
    /* isolate bit to reflect */
    temp = 0;
    temp = (input >> index);
    temp = temp & 0x0001;
    
    /* set bit into reflecting_16bit */
    reflecting_16bit = reflecting_16bit | (temp << (15-index));
  }

  return reflecting_16bit;
}

/**
* @brief  Calculates the 16-bit CRC. The function requires
*         that the CRC_POLYNOM is defined,
*           which gives the wanted CRC polynom. 
* @param  crcData  - Data to perform the CRC-16 operation on.
*         crcReg   - Current or initial value of the CRC calculation
* @retval crcReg Current or initial value of the CRC calculation                   
*/
uint16_t crcCalc(uint16_t crcReg, uint8_t crcData) 
{
  uint8_t i;
  
  crcData = Reflected8(crcData);
  
  for (i = 0; i < 8; i++) 
  {
    // If upper most bit is 1
    if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
      crcReg = (crcReg << 1)  ^ CRC_POLYNOM;
    else
      crcReg = (crcReg << 1);
    
    crcData <<= 1;
  }
  
  return crcReg;
}

void EvaluateCrc(uint8_t * LPAWUR_payload){
  uint16_t crc_seed = 0;
  uint16_t crc_data = 0;
  uint8_t crc_data2Fifo[2];
  uint8_t i;
  
  /* initialization vector */
  crc_seed = 0x0000;
  crc_data = crc_seed;
  
  /* loop for CRC computation */
  for (i=0 ; i <PAYLOAD_LEN ; i++){
    crc_data = crcCalc(crc_data, LPAWUR_payload[i]);
  }
  
  /* reflect result of 16-bit CRC */
  crc_data = Reflected16(crc_data);  
  
  crc_data2Fifo[0] = (uint8_t) (crc_data >> 8);
  crc_data2Fifo[1] = (uint8_t) (crc_data & 0x00FF);
  
  *(LPAWUR_payload + PAYLOAD_LEN) = crc_data2Fifo[0];
  *(LPAWUR_payload + PAYLOAD_LEN +1) = crc_data2Fifo[1]; 
}


