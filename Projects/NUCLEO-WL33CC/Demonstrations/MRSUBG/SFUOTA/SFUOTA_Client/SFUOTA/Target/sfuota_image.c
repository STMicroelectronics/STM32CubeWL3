/**
  ******************************************************************************
  * @file    sfuota_image.c
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

/* Includes ------------------------------------------------------------------*/
#include "sfuota_image.h"

/* Private define -----------------------------------------------------------*/
#define SFUOTA_WORD_SIZE (4U)  /*!< Flash write granularity. */

/* Private variables --------------------------------------------------------*/
static uint32_t write_address = SFUOTA_MAP_APP_ADDR;                             /*!< Flash destination write address. */
static __attribute__((aligned(4))) uint8_t page_buffer[FLASH_PAGE_SIZE] = {0U};  /*!< Flash page assembly buffer. */
static uint16_t page_size = 0U;                                                  /*!< Current page buffer size. */
static uint16_t page_currently_written = 0U;                                     /*!< Number of pages written so far. */

/* Private functions prototypes ----------------------------------------------*/
static HAL_StatusTypeDef SFUOTA_ProgramWord(uint32_t address, uint32_t data);

/* Functions Definition -----------------------------------------------------*/

/**
  * @brief  Returns the largest image size the destination area can hold.
  * @param  None.
  * @retval Capacity in bytes.
  */
uint32_t SFUOTA_Image_GetCapacity(void)
{
  return (uint32_t)SFUOTA_MAP_APP_SIZE;
}

/**
  * @brief  Returns the base address the image is programmed at.
  * @param  None.
  * @retval Destination base address.
  */
uint32_t SFUOTA_Image_GetBaseAddress(void)
{
  return (uint32_t)SFUOTA_MAP_APP_ADDR;
}

/**
  * @brief  Discards staged data and restarts from the first destination address.
  * @param  None.
  * @retval None.
  */
void SFUOTA_Image_Reset(void)
{
  for (uint16_t index = 0U; index < (uint16_t)sizeof(page_buffer); index++)
  {
    page_buffer[index] = 0U;
  }

  write_address = SFUOTA_MAP_APP_ADDR;
  page_size = 0U;
  page_currently_written = 0U;
}

/**
  * @brief  Erases destination flash pages before starting SFUOTA upgrade session.
  * @param  None.
  * @retval None.
  */
void SFUOTA_Image_Erase(void)
{
  FLASH_EraseInitTypeDef p_erase_init = {0};
  uint32_t page_error = 0U;

  p_erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  p_erase_init.Page = SFUOTA_MAP_APP_PAGE_FIRST;
  p_erase_init.NbPages = (SFUOTA_MAP_APP_PAGE_LAST - SFUOTA_MAP_APP_PAGE_FIRST) + 1U;

  /* Erase the destination page range. */
  (void)HAL_FLASHEx_Erase(&p_erase_init, &page_error);
}

/**
  * @brief  Stages one received chunk into the page assembly buffer.
  * @param  data Chunk payload.
  * @param  length Chunk length in bytes.
  * @retval 1U if the chunk fits in the current page buffer.
  * @retval 0U if the chunk would overflow the page buffer.
  */
uint8_t SFUOTA_Image_Stage(const uint8_t *data, uint8_t length)
{
  /* Reject chunks that would overflow the page buffer. */
  if ((data == NULL) || (((uint32_t)page_size + (uint32_t)length) > FLASH_PAGE_SIZE))
  {
    return 0U;
  }

  for (uint8_t i = 0U; i < length; i++)
  {
    page_buffer[page_size + i] = data[i];
  }

  page_size = (uint16_t)(page_size + length);
  return 1U;
}

/**
  * @brief  Returns the number of bytes currently staged in the page assembly buffer.
  * @param  None.
  * @retval Staged byte count.
  */
uint16_t SFUOTA_Image_GetStagedSize(void)
{
  return page_size;
}

/**
  * @brief  Programs the staged page to flash when a full page or the last chunk is ready.
  *
  * Handles word alignment padding and clears the tag entry of the first page so
  * that the image is only marked valid at the end of the transfer.
  *
  * @param  remaining_size Number of image bytes still to be programmed, updated on return.
  * @retval 1U if the operation completes successfully.
  * @retval 0U on flash programming error.
  */
uint8_t SFUOTA_Image_Program(uint32_t *remaining_size)
{
  int32_t app_size_tmp;
  uint16_t page_limit;
  uint32_t data_word;

  if (remaining_size == NULL)
  {
    return 0U;
  }

  /* Nothing to do until a full page or the last chunk is ready. */
  if ((page_size != FLASH_PAGE_SIZE) && ((*remaining_size - page_size) != 0U))
  {
    return 1U;
  }

  app_size_tmp = (int32_t)(*remaining_size) - (int32_t)page_size;
  if (app_size_tmp < 0)
  {
    return 0U;
  }

  /* Blank the tag so the image stays unbootable until the transfer completes. */
  if ((page_currently_written == 0U) && (SFUOTA_TAG_OFFSET < FLASH_PAGE_SIZE))
  {
    page_buffer[SFUOTA_TAG_OFFSET + 0U] = 0xFFU;
    page_buffer[SFUOTA_TAG_OFFSET + 1U] = 0xFFU;
    page_buffer[SFUOTA_TAG_OFFSET + 2U] = 0xFFU;
    page_buffer[SFUOTA_TAG_OFFSET + 3U] = 0xFFU;
  }

  page_limit = page_size;
  /* Pad buffer to SFUOTA_WORD_SIZE alignment if needed */
  if ((page_limit % SFUOTA_WORD_SIZE) != 0U)
  {
    for (uint16_t i = page_limit; i < (uint16_t)(page_limit + (SFUOTA_WORD_SIZE - (page_limit % SFUOTA_WORD_SIZE))); i++)
    {
      page_buffer[i] = 0xFFU;
    }
    page_limit = (uint16_t)(page_limit + (SFUOTA_WORD_SIZE - (page_limit % SFUOTA_WORD_SIZE)));
  }

  /* Program the page word by word. */
  for (uint16_t i = 0U; i < page_limit; i = (uint16_t)(i + SFUOTA_WORD_SIZE))
  {
    data_word = ((uint32_t)page_buffer[i + 0U]) |
                (((uint32_t)page_buffer[i + 1U]) << 8) |
                (((uint32_t)page_buffer[i + 2U]) << 16) |
                (((uint32_t)page_buffer[i + 3U]) << 24);

    if (SFUOTA_ProgramWord(write_address + i, data_word) != HAL_OK)
    {
      return 0U;
    }
  }

  /* Update write address and clear buffer */
  write_address += page_limit;
  page_currently_written++;
  for (uint16_t i = 0U; i < page_size; i++)
  {
    page_buffer[i] = 0U;
  }
  *remaining_size = (uint32_t)app_size_tmp;
  page_size = 0U;

  return 1U;
}

/* Private functions --------------------------------------------------------*/
/**
  * @brief  Programs one 32-bit word to flash memory.
  * @param  address Destination flash address.
  * @param  data 32-bit data word to program.
  * @retval HAL status returned by HAL_FLASH_Program.
  */
static HAL_StatusTypeDef SFUOTA_ProgramWord(uint32_t address, uint32_t data)
{
  return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
}
