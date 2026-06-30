/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ymodem.c
  * @author  GPAM Wireless Application Team
  * @brief   YMODEM protocol implementation.
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "ymodem.h"

/* Private define -----------------------------------------------------------*/
#define IS_AF(c)       (((c) >= 'A') && ((c) <= 'F'))
#define IS_af(c)       (((c) >= 'a') && ((c) <= 'f'))
#define IS_09(c)       (((c) >= '0') && ((c) <= '9'))
#define ISVALIDHEX(c)  (IS_AF(c) || IS_af(c) || IS_09(c))
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  ((c) - '0')
#define CONVERTHEX_ALPHA(c)  (IS_AF(c) ? ((c) - 'A' + 10) : ((c) - 'a' + 10))
#define CONVERTHEX(c)  (IS_09(c) ? ((c) - '0') : CONVERTHEX_ALPHA(c))

/* Private variables --------------------------------------------------------*/
uint8_t ymodem_rx_buffer[RX_BUFFER_SIZE];             /*!< Shared YMODEM UART RX byte buffer. */
uint32_t ymodem_rx_buffer_size = 0U;                  /*!< Number of unread bytes in ymodem_rx_buffer. */
uint8_t *ymodem_rx_buffer_ptr = ymodem_rx_buffer;     /*!< Read pointer inside ymodem_rx_buffer. */
uint8_t *ymodem_rx_buffer_tail_ptr = ymodem_rx_buffer;/*!< Write pointer inside ymodem_rx_buffer. */

/* Private functions prototypes ----------------------------------------------*/
static uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum);
static int32_t Receive_Byte(uint8_t *c, uint32_t timeout);
static uint32_t Send_Byte(uint8_t c);
static int32_t Receive_Packet(uint8_t *data, int32_t *length, uint32_t timeout);

/**
  * @brief  Converts decimal/hex size string to 32-bit integer.
  * @param  inputstr Pointer to input string.
  * @param  intnum Pointer receiving converted integer value.
  * @retval 1U when conversion succeeds.
  * @retval 0U when conversion fails.
  */
static uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0U;

  if ((inputstr[0] == '0') && ((inputstr[1] == 'x') || (inputstr[1] == 'X')))
  {
    if (inputstr[2] == '\0')
    {
      return 0U;
    }

    for (i = 2U; i < 11U; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1; */
        res = 1;
        break;
      }

      if (ISVALIDHEX(inputstr[i]))
      {
        val = (val << 4) + (uint32_t)CONVERTHEX(inputstr[i]);
      }
      else
      {
        /* Return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 8 digit hex --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }
  else /* max 10-digit decimal input */
  {
    for (i = 0U; i < 11U; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1 */
        res = 1;
        break;
      }
      else if (((inputstr[i] == 'k') || (inputstr[i] == 'K')) && (i > 0U))
      {
        val = val << 10;
        *intnum = val;
        res = 1;
        break;
      }
      else if (((inputstr[i] == 'm') || (inputstr[i] == 'M')) && (i > 0U))
      {
        val = val << 20;
        *intnum = val;
        res = 1;
        break;
      }
      else if (ISVALIDDEC(inputstr[i]))
      {
        val = (val * 10U) + (uint32_t)CONVERTDEC(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 10 digit decimal --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }

  return res;
}

/**
  * @brief  Receives one byte from YMODEM RX buffer with timeout.
  * @param  c Pointer receiving the byte.
  * @param  timeout Timeout in ms.
  * @retval 0 when one byte is received.
  * @retval -1 on timeout.
  */
static int32_t Receive_Byte(uint8_t *c, uint32_t timeout)
{
  uint32_t start = HAL_GetTick();

  if (timeout == 0U)
  {
    timeout = 1U;
  }

  while ((HAL_GetTick() - start) < timeout)
  {
    if (ymodem_rx_buffer_size > 0U)
    {
      __disable_irq();
      *c = *ymodem_rx_buffer_ptr++;
      ymodem_rx_buffer_size--;
      __enable_irq();
      return 0;
    }
  }

  return -1;
}

/**
  * @brief  Sends one byte on the YMODEM output stream.
  * @param  c Byte to send.
  * @retval 0U always.
  */
static uint32_t Send_Byte(uint8_t c)
{
  putchar((int)c);
  return 0U;
}

/**
  * @brief  Receives one YMODEM packet.
  * @param  data Pointer to packet buffer.
  * @param  length Pointer receiving decoded packet payload length.
  * @param  timeout Timeout in ms for byte reception.
  * @retval 0 on valid packet/EOT/CA-CA sequence.
  * @retval 1 on ABORT1/ABORT2 user abort.
  * @retval -1 on timeout or malformed packet.
  */
static int32_t Receive_Packet(uint8_t *data, int32_t *length, uint32_t timeout)
{
  uint16_t i;
  uint16_t packet_size;
  uint8_t c;

  *length = 0;
  if (Receive_Byte(&c, timeout) != 0)
  {
    return -1;
  }

  switch (c)
  {
    case SOH:
      packet_size = PACKET_SIZE;
      break;

    case STX:
      packet_size = PACKET_1K_SIZE;
      break;

    case EOT:
      return 0;

    case CA:
      if ((Receive_Byte(&c, timeout) == 0) && (c == CA))
      {
        *length = -1;
        return 0;
      }
      else
      {
        return -1;
      }
    case ABORT1:
    case ABORT2:
      return 1;

    default:
      return -1;
  }

  *data = c;
  for (i = 1U; i < (packet_size + PACKET_OVERHEAD); i++)
  {
    if (Receive_Byte(data + i, timeout) != 0)
    {
      return -1;
    }
  }

  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xFFU) & 0xFFU))
  {
    return -1;
  }

  *length = (int32_t)packet_size;
  return 0;
}

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Aborts YMODEM transfer by sending CA twice.
  * @retval None.
  */
void Ymodem_Abort(void)
{
  (void)Send_Byte(CA);
  (void)Send_Byte(CA);
}

/**
  * @brief  Sends ACK for the current YMODEM packet.
  * @retval None.
  */
void Ymodem_SendAck(void)
{
  (void)Send_Byte(ACK);
}

/**
  * @brief  Initializes YMODEM runtime RX buffer state.
  * @retval None.
  */
void Ymodem_Init(void)
{
  ymodem_rx_buffer_size = 0U;
  ymodem_rx_buffer_ptr = ymodem_rx_buffer;
  ymodem_rx_buffer_tail_ptr = ymodem_rx_buffer;
}

/**
  * @brief  Executes one YMODEM receive step.
  * @param  buf Pointer to destination payload buffer.
  * @param  buf_size Size of @p buf in bytes.
  * @param  size Pointer receiving payload or file size.
  * @param  packets_received Expected packet sequence index (0 for filename packet).
  * @retval YMODEM_CONTINUE when a valid packet is received and transfer must continue.
  * @retval YMODEM_DONE when end of transmission is detected.
  * @retval YMODEM_ABORTED when transfer is aborted by peer/user.
  * @retval YMODEM_TOO_MANY_ERRORS when maximum retry threshold is exceeded.
  * @retval YMODEM_NO_FILE when empty filename packet is received.
  */
int32_t Ymodem_Receive(uint8_t *buf, uint32_t buf_size, uint32_t *size, uint32_t packets_received)
{
  uint8_t *file_ptr;
  uint8_t *filesize_ptr;
  int32_t i;
  int32_t packet_length;
  int32_t errors = 0;
  int32_t return_value = YMODEM_CONTINUE;
  int32_t done = 0;
  uint8_t *packet_data = buf;

  (void)buf_size;

  while (done == 0)
  {
    switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT))
    {
      case 0:
        errors = 0;
        switch (packet_length)
        {
          /* Abort by sender */
          case -1:
            (void)Send_Byte(ACK);
            return_value = YMODEM_ABORTED;
            done = 1;
            break;

          /* End of transmission */
          case 0:
            (void)Send_Byte(ACK);
            return_value = YMODEM_DONE;
            done = 1;
            break;

          /* Normal packet */
          default:
            if ((packet_data[PACKET_SEQNO_INDEX] & 0xFFU) == (packets_received & 0xFFU))
            {
              if (packets_received == 0U)
              {
                /* Filename packet */
                if (packet_data[PACKET_HEADER] != 0U)
                {
                  /* Filename packet has valid data. */
                  for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < (int32_t)FILE_NAME_LENGTH);) //i++
                  {
                    /* Skip filename. */
                    file_ptr++;
                  }

                  /* Record start of filesize string. */
                  filesize_ptr = file_ptr + 1;
                  for (i = 0, file_ptr++; (*file_ptr != ' ') && (i < (int32_t)FILE_SIZE_LENGTH);) // i++
                  {
                    file_ptr++;
                  }
                  /* Add termination char. */
                  *file_ptr = '\0';
                  /* Convert to integer. */
                  (void)Str2Int(filesize_ptr, size);
                }
                else
                {
                  /* Filename packet is empty, end session. */
                  (void)Send_Byte(ACK);
                  return_value = YMODEM_NO_FILE;
                  done = 1;
                  break;
                }
              }
              else
              {
                /* Data packet. */
                int i;
                for (i = 0; i < packet_length; i++)
                {
                  buf[i] = buf[(uint32_t)i + PACKET_HEADER];
                }
                *size = (uint32_t)packet_length;
              }

              packets_received++;
              return_value = YMODEM_CONTINUE;
              done = 1;
            }
            break;
        }
        break;

      case 1:
        /* ABORT1 or ABORT2. */
        (void)Send_Byte(CA);
        (void)Send_Byte(CA);
        return_value = YMODEM_ABORTED;
        done = 1;
        break;

      default:
        if (packets_received > 0U)
        {
          errors++;
        }

        if (errors > (int32_t)MAX_ERRORS)
        {
          (void)Send_Byte(CA);
          (void)Send_Byte(CA);
          return_value = YMODEM_TOO_MANY_ERRORS;
          done = 1;
        }
        else
        {
          (void)Send_Byte(CRC16);
        }
        break;
    }
  }

  return return_value;
}
