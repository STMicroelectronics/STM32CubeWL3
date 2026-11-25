/**
  ******************************************************************************
  * @file    sfuota_image.h
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA image store: destination flash area of the downloaded image.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SFUOTA_IMAGE_H
#define SFUOTA_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "sfuota_flashmap.h"

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Get the largest image size the destination area can hold.
  * @retval Capacity in bytes.
  */
uint32_t SFUOTA_Image_GetCapacity(void);

/**
  * @brief  Get the base address the image is programmed at.
  * @retval Destination base address.
  */
uint32_t SFUOTA_Image_GetBaseAddress(void);

/**
  * @brief  Discard any staged data and restart from the first destination address.
  */
void SFUOTA_Image_Reset(void);

/**
  * @brief  Erase the destination area before starting a new download session.
  */
void SFUOTA_Image_Erase(void);

/**
  * @brief  Stage one received chunk into the page assembly buffer.
  * @param  data Chunk payload.
  * @param  length Chunk length in bytes.
  * @retval 1U if the chunk fits in the current page buffer.
  * @retval 0U if the chunk would overflow the page buffer.
  */
uint8_t SFUOTA_Image_Stage(const uint8_t *data, uint8_t length);

/**
  * @brief  Program the staged page to flash when a full page or the last chunk is ready.
  * @param  remaining_size Number of image bytes still to be programmed, updated on return.
  * @retval 1U if the operation completes successfully.
  * @retval 0U on flash programming error.
  */
uint8_t SFUOTA_Image_Program(uint32_t *remaining_size);

/**
  * @brief  Get the number of bytes currently staged in the page assembly buffer.
  * @retval Staged byte count.
  */
uint16_t SFUOTA_Image_GetStagedSize(void);

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_IMAGE_H */
