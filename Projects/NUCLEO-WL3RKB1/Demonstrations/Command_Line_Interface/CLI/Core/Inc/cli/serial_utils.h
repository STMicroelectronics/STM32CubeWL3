/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file    serial_utils.h
* @author  GPM WBL Application Team
* @brief   Entry point of the application
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SERIAL_UTILS_H_
#define _SERIAL_UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32wl3x.h"

typedef uint8_t  (printfFlushHandler)(uint8_t *contents,
                                      uint8_t length);


/* Functions Prototypes ------------------------------------------------------*/

uint8_t *writeHex(uint8_t *charBuffer, uint16_t value, uint8_t charCount);
uint8_t serialWriteString(const char * string);
uint8_t serialReadPartialLine(char *data, uint16_t max, uint16_t * index);
uint8_t serialWriteData(uint8_t *data, uint8_t length);
uint8_t serialWriteByte(uint8_t dataByte);
uint8_t serialReadByte(uint8_t *dataByte);

void __io_putchar( char c );
void __io_flush( void );
uint8_t __io_getcharNonBlocking(uint8_t *data);

#endif /* _SERIAL_UTILS_H_*/

