/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  GPM WBL Application Team
  * @brief   Entry point of the application
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
#include "main.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "stm32wl3x_ll_usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */

#define PACKET_PAYLOAD_SIZE 16
#define CRC_SIZE 1
#define MSG_SIZE (PACKET_PAYLOAD_SIZE + CRC_SIZE)

/* defines the longest sequence of 1's or 0's that can be decoded */
#define MAX_SEQ_LEN 10

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
#define MIN(a,b)                        (((a) < (b))? (a) : (b))

#define PRINT_RADIO_REG(REG, LABEL)     \
  do                                   \
  {                                    \
    uint32_t reg_value = READ_REG(MR_SUBG_RADIO->REG); \
    printf("%s 0x%08X", (LABEL), reg_value);           \
    printf("]\r\n");                   \
  } while (0)


#define STATE_HARDWARE_SYNC_SEARCH 0
#define STATE_PAYLOAD_RECEPTION 1
#define STATE_RECEPTION_COMPLETED 2
#define STATE_ABORT_RECEPTION 3

#define   TIM2_CLOCK 4000000UL // number of Timer clock cycle per seconds

/* define Tbit duration expressed in Timer clock count */
#define   TBIT_DURATION (TIM2_CLOCK/DATARATE)

#define TBIT_DURATION_MARGIN TBIT_DURATION/20 // 5% margin
//#define   TBIT_DURATION_MARGIN TBIT_DURATION/10 // 10% margin

#define SYNC_WORD 0x88888888

/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Frequency Value */

/* Capture variables */
uint16_t uhCaptureIndex = 0;
uint32_t uwIC2Value1 = 0;
uint32_t uwIC2Value2 = 0;
uint32_t uwDiffCapture = 0;

#ifdef DEBUG
typedef struct
{
  uint32_t duration;
  uint8_t edge_type; // 0 for rising edge, 1 for falling edge
  uint8_t dec_len;
} CaptureEvent;
CaptureEvent Capture_Table[200];
uint8_t Capture_Table_Idx = 0;
uint16_t syncOK_at;
uint8_t missed_edge = 0;
#endif

volatile uint8_t Rx_State = STATE_HARDWARE_SYNC_SEARCH;

/* Rx decode variables  */
uint8_t Rx_bit=0;
uint8_t Old_Rx_bit=0;
uint8_t seq_len_int = 10;
uint16_t seq_len_mod = 0;
uint8_t seq_len = 0;
uint8_t RxBuff_Byte_index = 0;
uint8_t RxBuff_Bit_index = 7;
uint32_t HwSync = 0x00000000;

/* CRC management */
static uint32_t crc;
static struct {
  uint32_t poly;
  uint8_t size;
  uint32_t seed;
  uint32_t upper_bit_mask;
  uint32_t mask;
  uint8_t reverse;
} crc_config;


uint16_t PktCnt =0;
uint8_t ValidPkt = 0;

__attribute__((aligned(4))) uint8_t vectcRxBuff[MSG_SIZE+1];
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
static void SystemApp_Init(void);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN PFP */
static void resetCtx(void);
static void decode_OOK(void);
static void crc_init(uint32_t poly, uint8_t size, uint32_t seed, uint8_t reverse);
static void crc_update(uint8_t data);
static uint32_t crc_get(void);
static void crc_reset_seed(void);

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
uint32_t MX_APPE_Init(void *p_param)
{
  UNUSED(p_param);

  /* USER CODE BEGIN APPE_Init_1 */

  /* USER CODE END APPE_Init_1 */
  SystemApp_Init();

  /* USER CODE BEGIN APPE_Init_2 */

  printf("STM32WL3 MRSUBG Direct GPIO Rx demonstration\n\r");

  PRINT_RADIO_REG(CLKREC_CTRL0, "ClockRec0");
  PRINT_RADIO_REG(CLKREC_CTRL1, "ClockRec1");
  PRINT_RADIO_REG(AGC0_CTRL, "AGC0");
  PRINT_RADIO_REG(AGC2_CTRL, "AGC2");
  PRINT_RADIO_REG(AGC3_CTRL, "AGC3");
  PRINT_RADIO_REG(AFC0_CONFIG, "AFC0");
  PRINT_RADIO_REG(AFC1_CONFIG, "AFC1");
  PRINT_RADIO_REG(AFC2_CONFIG, "AFC2");
  PRINT_RADIO_REG(RSSI_FLT, "RSSI_FLT");
  printf("TBIT duration is %d\r\n",(uint16_t)TBIT_DURATION);

  resetCtx();

  /* Send the RX command */
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);
  /* USER CODE END APPE_Init_2 */
   return 0;
}

void MX_APPE_Process(void)
{
  /* USER CODE BEGIN MX_APPE_Process_1 */
  if (Rx_State == STATE_RECEPTION_COMPLETED)
  {
    printf("Packet received\r\n");

#if (CRC_SIZE == 1) /* check CRC */
    uint8_t EXP_CRC_VAL;
    ValidPkt = 0;

    crc_init(0x07,1,0xff,0);

    /* compute CRC on received payload */
    for(uint8_t i=0;i<PACKET_PAYLOAD_SIZE;i++)
    {
      crc_update(vectcRxBuff[i]);
    }

    EXP_CRC_VAL=crc_get();
    printf("Computed CRC value is 0x%02X\r\n", EXP_CRC_VAL);
    if (vectcRxBuff[PACKET_PAYLOAD_SIZE]== (uint8_t) (EXP_CRC_VAL))
    {
      ValidPkt = 1;
    }
#endif

    if (ValidPkt == 1)
    {
      PktCnt++;
      printf("Valid packet received : cnt %d ",PktCnt);
      printf("\n\r");
      BSP_LED_On(LD2);
      HAL_Delay(300);
    }

    /* reset state and related variables, aborts RX */
    resetCtx();

    /* Send the RX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_RX);
  } /* end if(state = STATE_RECEPTION_COMPLETED) */

  if (Rx_State == STATE_ABORT_RECEPTION)
  {
    printf("Reception Aborted\n\r");

    /* reset Rx state and variables */
    resetCtx();

    /* Send the RX command */
    __HAL_MRSUBG_STROBE_CMD(CMD_RX);
  }

/* USER CODE END MX_APPE_Process_1 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

/**
  * @brief  Input capture callback in non blocking mode
  * @param  htim : htim handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    if(uhCaptureIndex == 0)
    {
      /* Get the 1st Input Capture value */
      uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
      uhCaptureIndex = 1;
    }
    else if(uhCaptureIndex == 1)
    {
      /* Get the 2nd Input Capture value */
      uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

      /* Capture computation */
      if (uwIC2Value2 > uwIC2Value1)
      {
        uwDiffCapture = (uwIC2Value2 - uwIC2Value1);
      }
      else if (uwIC2Value2 < uwIC2Value1)
      {
        /* 0xFFFF is max TIM2_CCRx value */
        uwDiffCapture = ((0xFFFF - uwIC2Value1) + uwIC2Value2) + 1;
      }
      else
      {
        /* If capture values are equal, we have reached the limit of frequency
           measures */
        Error_Handler();
      }
      uwIC2Value1 = uwIC2Value2;
#ifdef DEBUG
      Capture_Table[Capture_Table_Idx].duration = uwDiffCapture;
      Capture_Table[Capture_Table_Idx].edge_type = Old_Rx_bit;
      Capture_Table_Idx++;
      if (Capture_Table_Idx == 200)
      {
        Capture_Table_Idx = 0;
      }
#endif
      Old_Rx_bit = Rx_bit;
    }
    /* decode OOK data */
    decode_OOK();
  }
}


/**
  * @brief  Decode OOK data in non blocking mode
  *         first the hardware sync is searched,
  *         then the payload is decoded and a global rx buffer is filled.
  * @param  None
  * @retval None
  */
static void decode_OOK(void)
{
  Rx_bit = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) ? 0x01 : 0x00;

  if (Rx_State == STATE_HARDWARE_SYNC_SEARCH)
  {
    if (Rx_bit == Old_Rx_bit) /* case where Rx_bit is still 1 or 0, nothing to do*/
    {
    }
    else if (Rx_bit != Old_Rx_bit)  /* bit inversion */
    {
       /* check durations inside bit timings */
      if ((uwDiffCapture>= (TBIT_DURATION - TBIT_DURATION_MARGIN))
            && (uwDiffCapture<= (TBIT_DURATION + TBIT_DURATION_MARGIN))) /* single bit case */
      {
        HwSync = (HwSync << 1) | Old_Rx_bit;
      }
      else if ((uwDiffCapture>= 2*(TBIT_DURATION - TBIT_DURATION_MARGIN))
               && (uwDiffCapture<= 2*(TBIT_DURATION + TBIT_DURATION_MARGIN)))  /* 2 bits case */
      {
        for (uint8_t i = 0; i < 2; i++)
        {
          HwSync = (HwSync << 1) | Old_Rx_bit;
        }
      }
      else if ((uwDiffCapture>= 3*(TBIT_DURATION - TBIT_DURATION_MARGIN))
               && (uwDiffCapture<= 3*(TBIT_DURATION + TBIT_DURATION_MARGIN)))  /* 3 bits case */
      {
        for (uint8_t i = 0; i < 3; i++)
        {
          HwSync = (HwSync << 1) | Old_Rx_bit;
        }
      }
      else
      {
        /* if Bit timing not inside margin, reset HwSync */
        HwSync = 0x00000000;
      }
    }
    /* check Hw Sync with Sw variable */
    if (HwSync == SYNC_WORD)
    {
#ifdef DEBUG
      syncOK_at = Capture_Table_Idx;
#endif
      Rx_State = STATE_PAYLOAD_RECEPTION;
    }

  } /* end if (Rx_State == STATE_HARDWARE_SYNC_SEARCH) */

  else if (Rx_State == STATE_PAYLOAD_RECEPTION)
  {

    /* case where Rx_bit is still 1 or 0, nothing to do */
    if (Rx_bit == Old_Rx_bit)
    {
#ifdef DEBUG
      missed_edge++;
#endif
    }
    else if (Rx_bit != Old_Rx_bit) /* here there is transition : check durations inside bit timings */
    {
      seq_len_int = (uint8_t)(uwDiffCapture/TBIT_DURATION);
      seq_len_mod = (uint16_t)(uwDiffCapture % TBIT_DURATION);
      seq_len = 0;

      /* if duration is consistent with n bits (n*TBIT_DURATION) with margin, then decode n bits with same value */
      if ((seq_len_int < MAX_SEQ_LEN) && (seq_len_mod <= TBIT_DURATION * 0.1))
      {
        seq_len = seq_len_int;
      }
      else if ((seq_len_int < MAX_SEQ_LEN) && (seq_len_mod >= TBIT_DURATION * 0.9))
      {
        seq_len = seq_len_int + 1;
      }
      if (seq_len > 0)
      {
#ifdef DEBUG
        Capture_Table[Capture_Table_Idx-1].dec_len = seq_len;
#endif
        for (uint8_t i = 0; i < seq_len; i++)
        {
          vectcRxBuff[RxBuff_Byte_index] = vectcRxBuff[RxBuff_Byte_index] | (Old_Rx_bit << RxBuff_Bit_index);

          //update bit & byte indexes
          if (RxBuff_Bit_index == 0)
          {
            RxBuff_Bit_index = 7;
            RxBuff_Byte_index++;
          }
          else
          {
            RxBuff_Bit_index --;
          }
        }
      }
      else /* if Bit timing not inside margin, re-start state machine */
      {
        HwSync = 0x00000000;
        /* abort Rx */
        Rx_State = STATE_ABORT_RECEPTION;
      }
      /* check last bit */
      if(((RxBuff_Byte_index == PACKET_PAYLOAD_SIZE+CRC_SIZE-1) && (RxBuff_Bit_index == 0)) ||
         (RxBuff_Byte_index > PACKET_PAYLOAD_SIZE+CRC_SIZE-1))
      {
        if ((RxBuff_Byte_index == PACKET_PAYLOAD_SIZE+CRC_SIZE-1) && (RxBuff_Bit_index == 0))
        {
           vectcRxBuff[RxBuff_Byte_index] = vectcRxBuff[RxBuff_Byte_index] | (Rx_bit << RxBuff_Bit_index);
        }
        Rx_State = STATE_RECEPTION_COMPLETED;
#ifdef DEBUG
        for (uint8_t j = 0; j < PACKET_PAYLOAD_SIZE+CRC_SIZE; j++)
        {
          printf("B%d : 0x%02X\r\n", j, vectcRxBuff[j]);
        }
#endif
      }
    }  /* end else if (Rx_bit != Old_Rx_bit) */
  } /* end else if (Rx_State == STATE_PAYLOAD_RECEPTION) */

  /* update Old bit state at the end */
  Old_Rx_bit = Rx_bit;
}

/* reset Rx state and variables */
static void resetCtx()
{
  Rx_State = STATE_HARDWARE_SYNC_SEARCH;
  RxBuff_Byte_index = 0;
  RxBuff_Bit_index = 7;

#ifdef DEBUG
  missed_edge = 0;
  Capture_Table_Idx = 0;
  for (uint8_t j = 0; j < 200; j++)
  {
    Capture_Table[j].duration = 0;
    Capture_Table[j].dec_len = 0;
  }
#endif

  BSP_LED_Off(LD2);
  BSP_LED_Off(LD3);

  /* Send the SABORT command */
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);

  /* wait for Sabort process completed */
  while(LL_MRSubG_GetRadioFSMState() != STATE_IDLE)
  {
  }
}

static void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */
  BSP_LED_Init(LD2);
  BSP_LED_Init(LD3);
  /* USER CODE END SystemApp_Init_1 */

  if (__HAL_RCC_MRSUBG_IS_CLK_DISABLED())
  {
    /* Radio Peripheral reset */
    __HAL_RCC_MRSUBG_FORCE_RESET();
    __HAL_RCC_MRSUBG_RELEASE_RESET();

    /* Enable Radio peripheral clock */
    __HAL_RCC_MRSUBG_CLK_ENABLE();
  }

  /* USER CODE BEGIN SystemApp_Init_2 */

  /* USER CODE END SystemApp_Init_2 */
}


/* SW CRC management */
static uint32_t reverse(uint32_t x)
{
  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
  x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
  x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
  x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
  return((x >> 16) | (x << 16));
}

void crc_reset_seed(void)
{
  if(crc_config.reverse)
  {
    crc=((uint32_t)reverse(crc_config.seed)>>(8*(4-crc_config.size)));
  }
  else
  {
    crc=crc_config.seed;
  }
}

void crc_init(uint32_t poly, uint8_t size, uint32_t seed, uint8_t reverse)
{
  crc_config.poly=poly;
  crc_config.size=size;
  crc_config.seed=seed;
  crc_config.upper_bit_mask=0x80<<(8*(size-1));
  crc_config.mask=0;
  for(uint8_t i=0;i<size*8;i++)
  {
    crc_config.mask |=(((uint32_t)1)<<i);
  }
  crc_config.reverse=reverse;
  crc_reset_seed();
}

void crc_update(uint8_t data)
{
  uint8_t i;

  if(crc_config.reverse)
  {
    data=((uint32_t)reverse(data)>>24);
  }

  for (i = 0; i < 8; i++)
  {
    if ((((crc & crc_config.upper_bit_mask) >> (8*(crc_config.size-1))) ^ (data & 0x80)) & crc_config.mask)
    {
      crc = (crc << 1)  ^ crc_config.poly;
    }
    else
    {
      crc = (crc << 1);
    }
    data <<= 1;
  }
  crc &= crc_config.mask;
}

uint32_t crc_get(void)
{
  if(crc_config.reverse) {
    crc=reverse(crc)>>(8*(4-crc_config.size));
  }
  return crc & crc_config.mask;
}