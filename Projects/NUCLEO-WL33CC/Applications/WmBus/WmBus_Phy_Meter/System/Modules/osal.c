/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    osal.c
  * @author  MCD Application Team
  * @brief   osal APIs
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
#include <string.h>
#include "osal.h"

/**
 * Osal_MemSet
 *
 */
void Osal_MemSet(void *ptr, int value,unsigned int size)
{
    memset(ptr,value,size);
}

/**
 * Osal_MemCmp
 *
 */
int Osal_MemCmp(void *s1,void *s2,unsigned int size)
{
    return(memcmp(s1,s2,size));
}

/**
 * A version of the memcpy that only uses 32-bit accesses.
 * dest and src must be 32-bit aligned and size must be a multiple of 4.
 */
void Osal_MemCpy4(uint32_t *dest, const uint32_t *src, unsigned int size)
{
    for (unsigned int i = 0; i < size/4; i++)
        dest[i] = src[i];
}
