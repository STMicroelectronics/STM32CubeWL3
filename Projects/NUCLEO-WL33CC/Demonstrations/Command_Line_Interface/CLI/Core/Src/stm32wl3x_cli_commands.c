/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32wl3x_cli_commands.c
  * @author  GPM WBL Application Team
  * @brief   This file provides functions to implement commands issued
  *          over the serial port
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024-2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "stm32wl3x_cli_commands.h"

#define MIN(a,b) ((a) > (b) ? (b):(a))
#define MAX(a,b) ((a) < (b) ? (b):(a))

#define IQ_ITEM_SIZE 4
#define IQ_ITEM_PER_ROW 32
#define SOFT_SYM_ITEM_SIZE 1
#define SOFT_SYM_ITEM_PER_ROW 32
#define FREQ_DETECT_ITEM_SIZE 1
#define FREQ_DETECT_ITEM_PER_ROW 32
#define DIRECT_BUFFER_ITEM_SIZE 1
#define DIRECT_BUFFER_ITEM_PER_ROW 1

#define IQ_BUFFER_MARGIN 32 /* This margin is needed to assess that the buffer has filled with a margin */

#define MAX_ITEM_SIZE MAX( \
MAX( \
  MAX(IQ_ITEM_SIZE*IQ_ITEM_PER_ROW, SOFT_SYM_ITEM_SIZE*SOFT_SYM_ITEM_PER_ROW), \
    FREQ_DETECT_ITEM_SIZE*FREQ_DETECT_ITEM_PER_ROW), \
      DIRECT_BUFFER_ITEM_SIZE*DIRECT_BUFFER_ITEM_PER_ROW)

volatile uint8_t stopCmdFlag = 0;

/* Struct to dump Data Buffer when in RX */
typedef struct {
  uint16_t itemNumber;                                          /* Number of items generated in a single output */ 
  uint16_t itemSize;                                            /* Size of each record item */ 
  char *commandName;                                            /* The CLI command that handles data buffer action */
  char *dbHeader;                                               /* The header of the response for response parsing */
} SDBDumpRecord;


const char *status_msg[] = {
  "ok",
  "ov"
};

static void DBPrintHeader(const char* cmdname, const char* dbHeader){
  responseHeaderPrintf("#{&N &t &t &t}\r\n", cmdname, "timestamp", dbHeader, "status");
}

static void DBPrintRow(uint8_t *buffer, uint32_t timer, uint8_t status, uint32_t dataSize){
  uint16_t i;
  
  responsePrintf("{&4x {", timer);
  for (i=0; i < dataSize; i++)
    printf("%02x", buffer[i]);
  responsePrintf("} &s}\r\n", status_msg[status]);
}

uint8_t checkStop(void)
{
  uint8_t c;

  stopCmdFlag = 0;

  if (__io_getcharNonBlocking(&c)) {
    stopCmdFlag = (c == 'S');
  }

  return stopCmdFlag;
}

void ReadRegisterAction(void){
  uint32_t regAddr;

  regAddr = unsignedCommandArgument(0);
  responsePrintf("{&N utility call... &t4x}\r\n", "ReadRegister", "value", READ_REG(*(uint32_t*)regAddr));
}

void WriteRegisterAction(void){
  uint32_t regAddr;
  uint32_t regVal;

  regAddr = unsignedCommandArgument(0);
  regVal = unsignedCommandArgument(1);

  WRITE_REG(*(uint32_t*)regAddr, regVal);
  responsePrintf("{&N utility call...}\r\n", "WriteRegister");
}

/**
* @brief  Get the CLI timer.
*         <p><b>HowToCall</b>: CliGetTimer</p>
* @param  None
* @retval timer CLI_timer
* @retval counter CLI_timer counter
* @retval prescaler CLI_timer prescaler
*/
void CliGetTimerAction(void)
{
  /* The validate function in the test harness script takes a lot of time */
  responsePrintf("{&N API call...&t4x}\r\n", "CliGetTimer", "timer", TIMER_UTILS_GetTimerValue());
}

/**
* @brief  Reset the CLI timer.
*         <p><b>HowToCall</b>: CliResetTimer</p>
* @param  None
* @retval None
*/
void CliResetTimerAction(void)
{
  TIMER_UTILS_TimestampReset();

  /* The validate function in the test harness script takes a lot of time */
  responsePrintf("{&N API call...}\r\n", "CliResetTimer");
}

/* This table represent an approximated value for 100*10^x where x is i/20 with i [0-19] */
static const uint16_t lookup_pow10[] =
{
  100,
  112,
  126,
  141,
  158,
  178,
  200,
  224,
  251,
  282,
  316,
  355,
  398,
  447,
  501,
  562,
  631,
  708,
  794,
  891
};
static uint64_t power10(uint16_t rssi_reg)
{
  uint16_t v = rssi_reg/20;
  uint8_t rem = rssi_reg  % 20;
  uint64_t result = 1;
  uint16_t i;

  for (i = 0; i < v; i++)
        result = 10 * result;

  return (result * lookup_pow10[rem]);
}
static const unsigned char BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

static uint32_t bitcount(uint8_t *buffer, uint8_t length)
{
  unsigned int i, result;

  result = 0;
  for (i = 0; i < length; i++) {
    result += BitsSetTable256[buffer[i]];
  }
  return result;
}

static uint32_t compare(uint8_t *buffer1, uint8_t *buffer2, uint8_t length)
{
  uint8_t results[256];
  uint32_t i;
  for (i = 0; i < length; i++) {
    results[i] = buffer1[i] ^ buffer2[i];
  }
  return (bitcount(results, length));
}

static void RxPacketsInternal (uint32_t timeout, uint32_t interval, uint8_t summary_mode)
{
  static uint8_t payload_memory[255];
  uint32_t start;
  char *status;
  uint16_t bad_counter = 0;
  uint16_t good_counter = 0;
  uint8_t expected_packet_length;
  uint32_t ber = 0;
  uint64_t rssi_cum = 0;

  uint32_t overflow = 0; /* Flag to check if we have overflow in the RX queue */

  if (summary_mode) {
    expected_packet_length = copyStringArgument(2, payload_memory, sizeof(payload_memory), 0);
  }

  // Print multi header format example
  // #{{(rx)} {timestamp} {payload} {rssi}}
  responsePrintf("Reception started, type \'e\' to abort\r\n");

  if (summary_mode)
    responseHeaderPrintf("#{&N &t &t &t &t &t &t}\r\n", "rx", "timestamp", "status", "rssi", "good", "bad", "BER");
  else
    responseHeaderPrintf("#{&N &t &t &t &t &t &t}\r\n", "rx", "timestamp", "status", "rssi", "qi_info", "dummy", "payload");

  /* Start RX */
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  /* Record start time */
  start = TIMER_UTILS_GetTimerValue();

  /* Confirm start of reception */
  responsePrintf("{&4x &s &4x &2x &2x {}}\r\n", start, "started", 0, 0, 0);

  while (1) {
    /* Poll irq*/
    uint32_t irq = READ_REG(MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS);
    uint32_t timer = TIMER_UTILS_GetTimerValue();
    uint8_t  c;
    uint32_t irq_rx_mask = (MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_OK_F | MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F);
    uint32_t irq_error_mask = (MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_HW_ANA_FAILURE_F |
                               MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_AHB_ACCESS_ERROR_F |
                               MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_COMMAND_REJECTED_F );

    if (irq & irq_rx_mask) {
      /* Dump packet received and associated information */
      /* Format example */
      /* {{02F1EA0C} {rx ok} {00B4} {00} {20} {000102030405060708090a0b0c0d0e0f10111213}} */
      uint16_t rssi  = READ_REG(MR_SUBG_GLOB_STATUS->RX_INDICATOR) & MR_SUBG_GLOB_STATUS_RX_INDICATOR_RSSI_LEVEL_ON_SYNC;
      uint16_t count = READ_REG(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO) & MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_CURRENT_DATABUFFER_COUNT;
      uint32_t qi_info   = MR_SUBG_GLOB_STATUS->QI_INFO;
      uint8_t *data_ptr = (uint8_t *) MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR;

      /* Clear interrupts */
      MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = irq_rx_mask;

      count = MIN(count, sizeof(payload_memory));
      if (summary_mode) {
        uint32_t payload_biterror_count = compare(payload_memory, data_ptr, count);
        if (payload_biterror_count) {
          bad_counter++;
        } else {
          good_counter++;
        }
        rssi_cum += power10(rssi);
        ber += payload_biterror_count;
      } else {
        status = (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_CRC_ERROR_F) ? "rx (crc error)": "rx ok";
        memcpy(payload_memory, data_ptr, count);
      }
      /* Restart RX */
      __HAL_MRSUBG_STROBE_CMD(CMD_RX);

      if (!summary_mode) {
        responsePrintf("{&4x &s &2x &4x {} {",
                       timer,
                       status,
                       rssi,
                       qi_info);

        for (int index = 0; index < count; index++) {
          printf("%02x", payload_memory[index]);
        }

        responsePrintf("}}\r\n");
      } else {
        /* Check applicable only in summary mode */
        if (count != expected_packet_length) {
          responsePrintf("{&4x &s &4x &2x &2x {}}\r\n", TIMER_UTILS_GetTimerValue(), "invalid length", count, 0, 0);
        }
      }
      /* Check for overflow */
      irq = READ_REG(MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS);
      overflow = irq & irq_rx_mask;
      if (overflow) {
        uint16_t rssi  = READ_REG(MR_SUBG_GLOB_STATUS->RX_INDICATOR) & MR_SUBG_GLOB_STATUS_RX_INDICATOR_RSSI_LEVEL_ON_SYNC;
        responsePrintf("{&4x &s &4x &2x &2x {}}\r\n", TIMER_UTILS_GetTimerValue(), "overflow", rssi, 0, 0);
      }
    }
    else if (irq & irq_error_mask) {
        responsePrintf("{&4x &s &4x &2x &2x {}}\r\n", TIMER_UTILS_GetTimerValue(), "error", irq, 0, 0);
        /* Clear interrupts */
        MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = irq_error_mask;
    }
    else if (irq & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F) {
      responsePrintf("{&4x &s &4x &2x &2x {}}\r\n", timer, "timeout", 0, 0, 0);
      MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_TIMEOUT_F;
    }
    else if ((timeout >0) && ((timer-start)>timeout)) {
      responsePrintf("{&4x &s {%4x%4x} &2x &2x &4x}\r\n", timer, "time expired", (uint32_t) (rssi_cum>>32), (uint32_t) rssi_cum, good_counter, bad_counter, ber);
      break;
    }
    if (serialReadByte(&c)) {
      if (c=='e') {
        responsePrintf("{&4x &s {%4x%4x} &2x &2x &4x}\r\n", timer, "user abort", (uint32_t) (rssi_cum>>32), (uint32_t) rssi_cum, good_counter, bad_counter, ber);
        break;
      }
    }
  }

  /* Stop reception and return */
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
}

void RxPacketsSumAction (void)
{
  uint32_t timeout = unsignedCommandArgument(0); /* unit is us. zero means infinite timeout */
  uint32_t interval = unsignedCommandArgument(1); /* Not used/useful ?*/
  RxPacketsInternal(timeout, interval, 1);
}

void RxPacketsAction (void)
{
  uint32_t timeout = unsignedCommandArgument(0); /* unit is us. zero means infinite timeout */
  uint32_t interval = unsignedCommandArgument(1); /* Not used/useful ?*/
  RxPacketsInternal(timeout, interval, 0);
}

void RxDataAction (void)
{
  /* Start RX and it waits that DATABUFFER reach almost full state, then it dumps the received bytes.
  *  Before calling this command, the RX chain should be setup and the threshold for desired number of bits should be setup
  */

  uint32_t error=0;

  /* Clear almost full flag */
  MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F;

  /* Start RX */
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  /* Wait number of bits to be collected */
  while( (READ_REG(MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS ) & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_RX_ALMOST_FULL_0_F) == 0) {
    uint8_t c;
    if (serialReadByte(&c)) {
      if (c=='e') {
        error=1;
        break;
      }
    }
  }

  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);

  /* Output bitstream */
  responsePrintf("{&N call...&t4x {data:", "RxData", "error", error);

  uint16_t count = READ_REG(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO) & MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_CURRENT_DATABUFFER_COUNT;
  uint8_t *data_ptr = (uint8_t *) MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR;

  for (int index = 0; index < count; index++) {
    printf("%02x", data_ptr[index]);
  }

  responsePrintf("}}\r\n");

  return;
}

void TxPacketsAction(void){

  uint32_t txPeriod = unsignedCommandArgument(0); /* unit is us */
  uint32_t nPackets = unsignedCommandArgument(1); /* 0 means infinite */
  uint16_t max_len = MIN(MR_SUBG_GLOB_STATIC->DATABUFFER_SIZE, 255);
  uint16_t nBytes = copyStringArgument(2, (uint8_t *)MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR, max_len, 0);

  /* 'e' char to quit the transmission cycle*/
  uint8_t  c;

  responsePrintf("Transmission started, type \'e\' to abort.\r\n");
  responseHeaderPrintf("#{&N &t &t}\r\n", "tx", "timestamp", "status");
  /* Acknowledge the command */
  responsePrintf("{&4x &s}\r\n", TIMER_UTILS_GetTimerValue(), "started");

  for(int i=0; (i<nPackets) || (nPackets == 0); i++){
    uint32_t timer = TIMER_UTILS_GetTimerValue();
    int8_t error = 0;
    int last = (i > 0) && (i == (nPackets - 1));

    uint8_t timer_wrap = timer > (timer+txPeriod);    
    
    /* Start transmission */
    __HAL_MRSUBG_STROBE_CMD(CMD_TX);

    /* Wait for the TX period to end */
    error = 1; /* Assume no TX done flag will be received */
    
     while(1) {
       uint32_t timer_now = TIMER_UTILS_GetTimerValue();
       
       /* Wait for tx period to expire */
       if ((timer_now < timer) || !timer_wrap){
         if (timer_now >= (timer+txPeriod))
           break;
       }
       
      if (MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS & MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F) {
        responsePrintf("{&4x &s}\r\n", TIMER_UTILS_GetTimerValue(), "TX OK");
        error = 0; /* No error, TX done received */
        
        /* Clear the IRQ flag */
        MR_SUBG_GLOB_STATUS->RFSEQ_IRQ_STATUS = MR_SUBG_GLOB_STATUS_RFSEQ_IRQ_STATUS_TX_DONE_F;
        if (last) {
          break;
        }
      }
      if (serialReadByte(&c)) {
        if (c=='e') {
          responsePrintf("{&4x &s}\r\n", TIMER_UTILS_GetTimerValue(), "user abort");
          error = 2;
          break;
        }
      }
    }
    if (error == 1) {
      /* Here you can have an error due to missing TX done (wrong setup or critical
      error) or timeout smaller than packet length
      */
      responsePrintf("{&4x &s}\r\n", TIMER_UTILS_GetTimerValue(), "tx error");
    }
    if (error) {
      break;
    }
  }
}

void RssiLiveAction(void){
  uint32_t start, current, target;
  uint32_t update_interval = unsignedCommandArgument(0); /* unit is ms. */

  responsePrintf("RSSI Live started, type \'e\' to abort\r\n");
  responseHeaderPrintf("#{&N &t &t &t}\r\n", "RSSILive", "timestamp", "rssi", "status");

  /* Minimum value is 50 ms */
  update_interval = update_interval < 50 ? 50 : update_interval;
  /* Move to us */
  update_interval *= 1000;

  start = TIMER_UTILS_GetTimerValue();
  /* Confirm start of reception */
  responsePrintf("{&4x &4x &s}\r\n", start, 0, "started");

  target = start + update_interval;
  do  {
    uint16_t rssi ;
    uint8_t c;
    current = TIMER_UTILS_GetTimerValue();
    if (current >= target) {
      rssi = READ_REG_FIELD(MR_SUBG_GLOB_STATUS->RX_INDICATOR, MR_SUBG_GLOB_STATUS_RX_INDICATOR_RSSI_LEVEL_RUN);
      responsePrintf("{&4x &2x &s}\r\n",
                     current,
                     rssi,
                     "ok"
                       );
      target = current + update_interval;
    }
    if (serialReadByte(&c)) {
      if (c=='e') {
        responsePrintf("{&4x &4x &s}\r\n", current, 0, "user abort");
        break;
      }
    }
  } while (1);
}

void WakeupRxAction(void)
{
  /* Start Wakeup Radio and it waits for packet reception, then it dumps the received bytes.
  *  Before calling this command, the Wakeup RX chain should be configured
  */
  char * error_string[]={
    "rx ok",
    "rx error sync",
    "rx error framing",
    "rx error crc"
  };

  uint32_t status_mask = LPAWUR_STATUS_ERROR_F | 
    LPAWUR_STATUS_FRAME_VALID_F |
      LPAWUR_STATUS_FRAME_COMPLETE_F |
        LPAWUR_STATUS_FRAME_SYNC_COMPLETE_F |
          LPAWUR_STATUS_BIT_SYNC_DETECTED_F;

  // Print multi header format example
  // #{{(wrx)} {timestamp} {status} {payload}}
  printf("Reception started, type \'e\' to abort\r\n");
  responseHeaderPrintf("#{&N &t &t &t}\r\n", "wrx", "timestamp", "status", "payload");

  LPAWUR->STATUS = status_mask;

  SET_BIT(LPAWUR->RFIP_CONFIG,LPAWUR_RFIP_CONFIG_LPAWUR_ENABLE);

  /* Confirm start of reception */
  responsePrintf("{&4x &s {}}\r\n", TIMER_UTILS_GetTimerValue(), "started");

  while(1)
  {
    /* Poll status*/
    uint32_t status = READ_REG(LPAWUR->STATUS);
    uint32_t timer = TIMER_UTILS_GetTimerValue();
    uint8_t error_f = (status & LPAWUR_STATUS_ERROR_F_Msk) >> LPAWUR_STATUS_ERROR_F_Pos;
    uint8_t  c;

    if ((status & LPAWUR_STATUS_FRAME_COMPLETE_F) || (status & LPAWUR_STATUS_FRAME_VALID_F)) {
      /* Frame (payload + CRC) received, the content of the PAYLOAD_X registers is valid.
      Note: the frame may have been received with CRC or Framing error.
      */
      uint8_t payload_memory[8];
      uint8_t payload_length = READ_REG_FIELD(LPAWUR->FRAME_CONFIG0,LPAWUR_FRAME_CONFIG0_PAYLOAD_LENGTH);
      LPAWUR->STATUS = status_mask;
      
      *((uint32_t*) payload_memory) = LPAWUR->PAYLOAD_0;
      *((uint32_t*) (payload_memory + 4)) = LPAWUR->PAYLOAD_1;

      /* Restart RX*/
      SET_BIT(LPAWUR->RFIP_CONFIG,LPAWUR_RFIP_CONFIG_LPAWUR_ENABLE);

      /* Workaround to the fact that the enable bit take up to about 90 us since it is running on slow clock */ 
      while (READ_REG_FIELD(LPAWUR->RFIP_CONFIG, LPAWUR_RFIP_CONFIG_LPAWUR_ENABLE) == 0) {
        SET_BIT(LPAWUR->RFIP_CONFIG,LPAWUR_RFIP_CONFIG_LPAWUR_ENABLE);
      }
      
      responsePrintf("{&4x &s {",
                     timer,
                     error_string[error_f]);

      for (int index = 0; index < payload_length; index++) {
        printf("%02x", payload_memory[index]);
      }
      printf("}}\r\n");
    }

    if (serialReadByte(&c)) {
      if (c=='e') {
        responsePrintf("{&4x &s {}}\r\n", timer, "user abort");
        break;
      }
    }
  }
  CLEAR_BIT(LPAWUR->RFIP_CONFIG,LPAWUR_RFIP_CONFIG_LPAWUR_ENABLE);
}

static void circular_to_linear(uint8_t *linear_dest,
                               uint8_t *circular_base,
                               uint32_t circular_size,
                               uint32_t circular_current,
                               uint32_t items,
                               uint32_t item_size,
                               uint32_t subsampling
                               )
{
  uint32_t i = 0, j = 0;
  while (i < items) {
    j = 0;
    while (j < item_size) {
      uint32_t ptr = i*item_size+j;
      linear_dest[ptr] = circular_base[(circular_current+ptr)%circular_size];
      j++;
    }
    i = i + (1 + subsampling);
  }
}

static void DBDump(SDBDumpRecord recordInfo, uint32_t size, uint16_t subsampling)
{
  uint8_t c;
  uint32_t circular_buffer_read = 0; /* Offset in the RX fifo */
  uint8_t linear_buffer[MAX_ITEM_SIZE];
  uint32_t used_mask = MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_NB_DATABUFFER_USED >> MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_NB_DATABUFFER_USED_Pos;
  uint8_t overflow_flag = 0;

  responsePrintf("Dump started, type \'e\' to abort\r\n");  
  
  /* Print databuffer header */
  DBPrintHeader(recordInfo.commandName, recordInfo.dbHeader);

  /* Put the device in RX */
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  /* Confirm start of reception */
  responsePrintf("{&4x {} &s}\r\n", TIMER_UTILS_GetTimerValue(), "started");

  uint32_t dbLimit = READ_REG(MR_SUBG_GLOB_STATIC->DATABUFFER_SIZE);
  
  if (dbLimit < IQ_BUFFER_MARGIN) { dbLimit = IQ_BUFFER_MARGIN; }

  if (size > 0) {
    if (size >= (dbLimit-IQ_BUFFER_MARGIN))
      size = dbLimit-IQ_BUFFER_MARGIN;
    while ((READ_REG(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO) & MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_CURRENT_DATABUFFER_COUNT) < size);
    /* Abort command */
    __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
  }

  while(1){
    /* Count number of received bytes */
    uint16_t count = READ_REG(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO) & MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_CURRENT_DATABUFFER_COUNT;
    uint16_t used = READ_REG_FIELD(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO, MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_NB_DATABUFFER_USED); /* Read number of buffer used */
    uint16_t bytes = (count-circular_buffer_read) % dbLimit; /* new data */
    uint16_t base = circular_buffer_read;
    uint16_t count1, used1;
    uint32_t row_size = recordInfo.itemSize * recordInfo.itemNumber * (subsampling+1);

    while (bytes >= row_size) {
      circular_to_linear(linear_buffer,
                         (uint8_t *)MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR,
                         dbLimit,
                         circular_buffer_read,
                         recordInfo.itemNumber,
                         recordInfo.itemSize,
                         subsampling
                         );
      
      /* Print databuffer row */
      DBPrintRow(linear_buffer, TIMER_UTILS_GetTimerValue(), overflow_flag, (recordInfo.itemSize*recordInfo.itemNumber));
      
      overflow_flag = 0;
      circular_buffer_read = (circular_buffer_read + row_size) % dbLimit;
      bytes -= row_size;
    }
    count1 = READ_REG(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO) & MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_CURRENT_DATABUFFER_COUNT;
    used1 = READ_REG_FIELD(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO, MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_NB_DATABUFFER_USED); /* Read number of buffer used */
    
    if (used1 != used) {/* Possible overflow */
      overflow_flag = 1;
      if (used1 == ((used + 1) & used_mask)) {
        overflow_flag = (count1 >= base);
      }
    }

    if (serialReadByte(&c)) {
      if (c=='e') {
        responsePrintf("{&4x {} &s}\r\n", TIMER_UTILS_GetTimerValue(), "user abort");
        break;
      }
    }
  }

  /* Abort command */
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
}

void IQDumpAction(){
  uint32_t size = unsignedCommandArgument(0); /* unit is bytes. */
  uint32_t subsampling = unsignedCommandArgument(1); /* */

  SDBDumpRecord iqRecord;
  iqRecord.itemSize = IQ_ITEM_SIZE;
  iqRecord.itemNumber = IQ_ITEM_PER_ROW;
  iqRecord.commandName = "IQDump";
  iqRecord.dbHeader = "IQ";   

  DBDump(iqRecord, size, subsampling);
}

void SoftSymDumpAction(){
  uint32_t size = unsignedCommandArgument(0); /* unit is bytes. */
  uint32_t subsampling = unsignedCommandArgument(1); /* */
  
  SDBDumpRecord ssRecord;
  ssRecord.itemSize = SOFT_SYM_ITEM_SIZE;
  ssRecord.itemNumber = SOFT_SYM_ITEM_PER_ROW;
  ssRecord.commandName = "SoftSymDump";
  ssRecord.dbHeader = "SSYM";  

  DBDump(ssRecord, size, subsampling);
}

void FreqDetectDumpAction(){
  uint32_t size = unsignedCommandArgument(0); /* unit is bytes. */
  uint32_t subsampling = unsignedCommandArgument(1); /* */
  
  SDBDumpRecord fdRecord;
  fdRecord.itemSize = FREQ_DETECT_ITEM_SIZE;
  fdRecord.itemNumber = FREQ_DETECT_ITEM_PER_ROW;
  fdRecord.commandName = "FreqDetectDump";
  fdRecord.dbHeader = "FDET";  

  DBDump(fdRecord, size, subsampling);
}

void DirectBufferDumpAction(){
  SDBDumpRecord dbRecord;
  dbRecord.itemSize = DIRECT_BUFFER_ITEM_SIZE;
  dbRecord.itemNumber = 1;
  dbRecord.commandName = "DirectBufferDumpAction";
  dbRecord.dbHeader = "BUFDUMP";

  DBDump(dbRecord, 0, 0);
}
