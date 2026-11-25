/**
  ******************************************************************************
  * @file    sfuota_image.c
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

/* Includes ------------------------------------------------------------------*/
#include "sfuota_image.h"

/* Private define -----------------------------------------------------------*/
/* The window holds one loader chunk at a time; reads wrap around this span. */
#define SFUOTA_IMAGE_WINDOW_SPAN                      (1024U)

/* Private variables --------------------------------------------------------*/
static uint8_t image[SFUOTA_IMAGE_STAGING_SIZE];  /*!< Staging window filled by the loader. */
static uint32_t image_size = 0U;                  /*!< Total image size announced by the loader. */
static uint32_t data_len = 0U;                    /*!< Cumulative number of staged bytes. */

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Drops the staged window and the negotiated image size.
  * @param  None.
  * @retval None.
  */
void SFUOTA_Image_Reset(void)
{
  image_size = 0U;
  data_len = 0U;
}

/**
  * @brief  Sets the total image size announced by the loader.
  * @param  size Image size in bytes.
  * @retval None.
  */
void SFUOTA_Image_SetSize(uint32_t size)
{
  image_size = size;
}

/**
  * @brief  Returns the total image size.
  * @param  None.
  * @retval Image size in bytes.
  */
uint32_t SFUOTA_Image_GetSize(void)
{
  return image_size;
}

/**
  * @brief  Returns how many image bytes are available for transmission so far.
  * @param  None.
  * @retval Cumulative number of staged bytes.
  */
uint32_t SFUOTA_Image_GetAvailable(void)
{
  return data_len;
}

/**
  * @brief  Returns the buffer the loader must fill with the next chunk.
  * @param  None.
  * @retval Pointer to the staging window.
  */
uint8_t *SFUOTA_Image_GetStagingBuffer(void)
{
  return image;
}

/**
  * @brief  Accounts for a chunk just written into the staging buffer.
  * @param  length Number of bytes written by the loader.
  * @retval 1U if the announced image size has been reached.
  * @retval 0U if more chunks are expected.
  */
uint8_t SFUOTA_Image_CommitStaging(uint32_t length)
{
  /* The last chunk is padded by the loader, so it may overshoot the announced size. */
  if ((length + data_len) > image_size)
  {
    data_len = image_size;
    return 1U;
  }

  data_len += length;
  return 0U;
}

/**
  * @brief  Copies image bytes out of the staging window.
  * @param  offset Absolute byte offset from the beginning of the image.
  * @param  data Destination buffer.
  * @param  length Number of bytes to copy.
  * @retval 1U if the requested range was available and copied.
  * @retval 0U otherwise.
  */
uint8_t SFUOTA_Image_Read(uint32_t offset, uint8_t *data, uint8_t length)
{
  uint32_t window_base;
  uint32_t window_offset;

  /* Reject invalid outputs and ranges past what has been staged. */
  if ((data == NULL) || (data_len == 0U) || ((offset + (uint32_t)length) > data_len))
  {
    return 0U;
  }

  /* Only the chunk still staged is readable: the loader overwrote the earlier ones. */
  window_base = (data_len - 1U) - ((data_len - 1U) % SFUOTA_IMAGE_WINDOW_SPAN);
  if (offset < window_base)
  {
    return 0U;
  }

  window_offset = offset % SFUOTA_IMAGE_WINDOW_SPAN;
  for (uint8_t i = 0U; i < length; i++)
  {
    data[i] = image[window_offset + i];
  }

  return 1U;
}

/**
  * @brief  Fills the window with a known pattern to exercise the SFUOTA path without a loader.
  * @param  size Image size to simulate.
  * @retval None.
  */
void SFUOTA_Image_Preset(uint32_t size)
{
  image_size = size;
  data_len = size;

  for (uint32_t i = 0U; i < size; i++)
  {
    image[i % SFUOTA_IMAGE_WINDOW_SPAN] = (uint8_t)(i & 0xFFU);
  }
}
