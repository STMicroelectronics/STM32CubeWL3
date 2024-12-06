/**
******************************************************************************
* @file    osal.c 
* @author  WBL RF Application Team
* @version V1.0.0
* @date    21-January-2016
* @brief   osal APIs
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
******************************************************************************
*/ 
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


