/**
  ******************************************************************************
  * @file    sfuota_image.h
  * @author  GPAM Wireless Application Team
  * @brief   SFUOTA image source: sliding window of the image served over the air.
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
#include "stm32wl3x_hal.h"

/* Exported macros -----------------------------------------------------------*/
/* Must hold one full Ymodem 1K packet plus its framing overhead. */
#define SFUOTA_IMAGE_STAGING_SIZE                     (1024U + 8U)  /*!< Size of the staging window. */

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Drop the staged window and the negotiated image size.
  */
void SFUOTA_Image_Reset(void);

/**
  * @brief  Set the total image size announced by the loader.
  * @param  size Image size in bytes.
  */
void SFUOTA_Image_SetSize(uint32_t size);

/**
  * @brief  Get the total image size.
  * @retval Image size in bytes.
  */
uint32_t SFUOTA_Image_GetSize(void);

/**
  * @brief  Get how many image bytes are available for transmission so far.
  * @retval Cumulative number of staged bytes.
  */
uint32_t SFUOTA_Image_GetAvailable(void);

/**
  * @brief  Get the buffer the loader must fill with the next chunk.
  * @retval Pointer to the staging window, @ref SFUOTA_IMAGE_STAGING_SIZE bytes wide.
  */
uint8_t *SFUOTA_Image_GetStagingBuffer(void);

/**
  * @brief  Account for a chunk just written into the staging buffer.
  * @param  length Number of bytes written by the loader.
  * @retval 1U if the announced image size has been reached.
  * @retval 0U if more chunks are expected.
  */
uint8_t SFUOTA_Image_CommitStaging(uint32_t length);

/**
  * @brief  Copy image bytes out of the staging window.
  * @param  offset Absolute byte offset from the beginning of the image.
  * @param  data Destination buffer.
  * @param  length Number of bytes to copy.
  * @retval 1U if the requested range was available and copied.
  * @retval 0U otherwise.
  */
uint8_t SFUOTA_Image_Read(uint32_t offset, uint8_t *data, uint8_t length);

/**
  * @brief  Fill the window with a known pattern to exercise the SFUOTA path without a loader.
  * @param  size Image size to simulate.
  */
void SFUOTA_Image_Preset(uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* SFUOTA_IMAGE_H */
