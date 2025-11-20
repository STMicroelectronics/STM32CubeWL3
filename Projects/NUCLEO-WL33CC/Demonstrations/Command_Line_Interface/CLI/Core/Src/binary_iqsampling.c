/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    binary_iqsampling.c
  * @author  GPM WBL Application Team
  * @brief   This file provides firmware functions for raw I/Q sampling
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

#include "main.h"

#define DATABUF_SIZE 3000
#define PACKET_RINGBUFFER_COUNT 5
#define S3_IQSAMPLING_SYNC_WORD 0x413f35d9
#define S3_IQSAMPLING_ESCAPESEQ 0x2e7e2e7e
#define USART_PERIPHCLK 16000000 
#define _current_databuffer() ((READ_BIT(MR_SUBG_GLOB_STATUS->DATABUFFER_INFO, MR_SUBG_GLOB_STATUS_DATABUFFER_INFO_CURRENT_DATABUFFER) ? 1 : 0))

/*
 * STM32WL3-to-HOST binary protocol:
 * The STM32WL3 continuously transmits databuffer samples to the host.
 * To minimize bandwidth and to eliminate number-to-string conversions, the samples are transmitted
 * using a binary protocol which is made up of one single packet type.
 * Two packets are pre-allocated in RAM so that the SubGHz hardware can directly fill the payloads.
 */

/**
 * @brief Layout of raw I/Q sample packet header
 */
typedef struct
{
  /** Synchronization sequence: Used by decoder to detect start of packet in raw data stream */
  uint32_t sync_word;

  /** Number of raw samples in this particular packet */
  uint32_t samplecount;

  /** Number of packets since binary sample streaming command was issued, can be used to detect losses */
  uint32_t packetcounter;

  /** Number of packets suspected lost */
  uint32_t lost_packets;

  /** Configuration revision counter */
  uint32_t conf_revision;

  /** The true achieved sample rate, as calculated on STM32WL3 based on desired bandwidth */
  uint32_t sample_rate;

  /** Received signal strength indicator */
  uint32_t rssi;

  /** Indicator for error while setting override register */
  uint32_t register_override_error;

  /** Fields reserved for future use */
  uint32_t reserved[3];
} s3_iqsampling_header;

/**
 * @brief Layout of data packet used to convey raw I/Q samples from STM32WL3 to streaming application
 */
typedef struct
{
  /** This "skip" buffer is necessary to make everything work in SPI mode in case an SPI transaction is terminated too early */
  uint32_t skip;

  /** Header of packet */
  s3_iqsampling_header header;

  /** Raw samples. Note that the MR_SubGHz databuffer mechanisms will directly write to this field to avoid memory copies. */
  uint8_t databuf[DATABUF_SIZE];
} s3_iqsampling_packet;

/*
 * HOST-to-STM32WL3 binary protocol:
 * While the STM32WL3 is transmitting raw databuffer samples to the host, the host may also change
 * the radio's configuration, e.g. bandwidth, sample rate, frequency, ...
 * This protocol also consists of just one packet type that contains all configuration attributes.
 * All attributes are transmitted at once, there is no way to partially transmit them.
 */

/**
 * @brief Register address-value pair for manual register overrides
 */
typedef struct
{
  uint32_t address;
  uint32_t value;
} s3_register_override;

/**
 * @brief HOST-to-STM32WL3 binary configuration protocol packet format
 * "On the wire", all words are in little-endian order.
 * If the host is a big-endian system, it will need to adapt its byte order.
 */
typedef struct
{
  /** Whether databuffer streaming is active or not. Used upon entering sampling mode to start transmission. */
  uint32_t running;

  /** Low-pass filter bandwidth in Hz */
  uint32_t bandwidth;

  /** Local oscillator (LO) frequency */
  uint32_t frequency;

  /** Automatic Gain Control (AGC) enable / disable flag */
  uint8_t agc_enable;

  /** I/Q mismatch compensation enable / disable flag */
  uint8_t iqc_enable;

  /** DC block filter bandwidth / gain according to datasheet, range 0-15 */
  uint8_t dcrem_gain;

  /** Dummy for word-alignment */
  uint8_t dummy;

  /** Frequency of IF stage, both analog and digital (typically 300kHz / 600kHz), in Hz */
  uint32_t intermediate_frequency;

  /** True XTAL oscillator frequency, 48MHz by default */
  uint32_t xtal_freq;

  /** Manual configuration register overrides */
  s3_register_override register_overrides[16];

  /** Fields reserved for future use */
  uint32_t reserved[4];
} s3_iqsampling_conf;

typedef struct
{
  /** Synchronization sequence: Used by decoder to detect start of packet in raw data stream. */
  uint32_t sync_word;

  /** Configuration counter: Used to identify configuration revisions to check if STM32WL3 is using latest one */
  uint32_t revision_counter;

  /** Configuration data */
  s3_iqsampling_conf configuration;

  /** CRC-32 over all previous struct entries */
  uint32_t crc;
} s3_iqsampling_conf_packet;

typedef enum
{
  WAIT_FOR_SYNC,
  READ_CONFIG
} s3_iqsampling_conf_parser_state;

static s3_iqsampling_conf_packet _configuration_packet;
static s3_iqsampling_conf _configuration_active;
static s3_iqsampling_conf_parser_state _parser_state;
static uint32_t _parser_offset;
static uint32_t _conf_revision_counter;
static uint8_t _terminate_samplingmode;
static uint32_t _register_override_error;

/* This computes the standard CRC-32 checksum */
#define CRC32_POLYNOMIAL 0xEDB88320

static uint32_t _crc32(uint8_t const data[], size_t length)
{
  if (length == 0)
    return 0;

  uint32_t remainder = 0xffffffff;

  for (size_t i = 0; i < length; ++i)
  {
    remainder ^= data[i];

    for (uint8_t bit = 8; bit > 0; --bit)
    {
      if (remainder & 1)
        remainder = ((remainder >> 1) ^ CRC32_POLYNOMIAL);
      else
        remainder = (remainder >> 1);
    }
  }

  return ~remainder;
}

/**
 * @brief  Apply current RF configuration
 * @retval None
 */
static void _iqsampling_conf_apply(void)
{
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);
  HAL_MRSubG_SetChannelBW(_configuration_active.bandwidth);
  HAL_MRSubG_SetFrequencyBase(_configuration_active.frequency);

  MODIFY_REG_FIELD(MR_SUBG_RADIO->AGC0_CTRL, MR_SUBG_RADIO_AGC0_CTRL_AGC_EN, _configuration_active.agc_enable);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->IQC_CONFIG, MR_SUBG_GLOB_STATIC_IQC_CONFIG_IQC_ENABLE, _configuration_active.iqc_enable);
  MODIFY_REG_FIELD(MR_SUBG_RADIO->DCREM_CTRL0, MR_SUBG_RADIO_DCREM_CTRL0_START_GAIN, _configuration_active.dcrem_gain);
  MODIFY_REG_FIELD(MR_SUBG_RADIO->DCREM_CTRL0, MR_SUBG_RADIO_DCREM_CTRL0_TRACK_GAIN, _configuration_active.dcrem_gain);

  uint32_t f_dig = LL_GetXTALFreq() / 3;
  uint32_t if_offset = (((_configuration_active.intermediate_frequency / 10) * 65536) / f_dig) * 10;
  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->IF_CTRL, MR_SUBG_GLOB_STATIC_IF_CTRL_IF_OFFSET_ANA, if_offset);
  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->IF_CTRL, MR_SUBG_GLOB_STATIC_IF_CTRL_IF_OFFSET_DIG, if_offset);

  LL_SetXTALFreq(_configuration_active.xtal_freq);

  /* Apply manual register overrides */
  _register_override_error = 0;
  for (uint32_t i = 0; i < sizeof(_configuration_active.register_overrides) / sizeof(_configuration_active.register_overrides[0]); ++i)
  {
    if (_configuration_active.register_overrides[i].address != 0)
    {
      /* Ensure that register address is within bounds for SubGHz peripheral */
      if (_configuration_active.register_overrides[i].address < MR_SUBG_BASE || _configuration_active.register_overrides[i].address >= LPAWUR_BASE)
      {
        _register_override_error = 1;
      }
      else
      {
        *((__IOM uint32_t *)_configuration_active.register_overrides[i].address) = _configuration_active.register_overrides[i].value;
      }
    }
  }

  __HAL_MRSUBG_STROBE_CMD(CMD_RX);
}

/**
 * @brief  Reset binary HOST-to-STM32WL3 configuration protocol parser to its initial state.
 * @retval None
 */
static void _iqsampling_conf_parser_reset(void)
{
  _parser_state = WAIT_FOR_SYNC;
  _configuration_packet.sync_word = 0;
  _parser_offset = 0;
  _conf_revision_counter = 0;
}

/**
 * @brief  Parse and apply binary HOST-to-STM32WL3 configuration protocol.
 * @param  c Byte that was previously received through the host interface.
 * @retval None
 */
static void _iqsampling_conf_parser_parse(char c)
{
  if (_parser_state == WAIT_FOR_SYNC)
  {
    uint8_t *sync_candidate = (uint8_t *)&_configuration_packet.sync_word;
    sync_candidate[0] = sync_candidate[1];
    sync_candidate[1] = sync_candidate[2];
    sync_candidate[2] = sync_candidate[3];
    sync_candidate[3] = c;
  }

  if (_parser_state == WAIT_FOR_SYNC)
  {
    if (_configuration_packet.sync_word == S3_IQSAMPLING_SYNC_WORD)
    {
      _parser_offset = 4;
      _parser_state = READ_CONFIG;
    }
    else if (_configuration_packet.sync_word == S3_IQSAMPLING_ESCAPESEQ)
    {
      _terminate_samplingmode = 1;
    }
  }
  else if (_parser_state == READ_CONFIG)
  {
    ((uint8_t *)&_configuration_packet)[_parser_offset] = c;

    _parser_offset++;

    if (_parser_offset >= sizeof(_configuration_packet))
    {
      _parser_state = WAIT_FOR_SYNC;

      /* Ensure that computed CRC-32 is equivalent to CRC-32 in configuration packet */
      uint32_t computed_crc = _crc32((uint8_t *)&_configuration_packet, sizeof(_configuration_packet) - sizeof(_configuration_packet.crc));
      if (computed_crc == _configuration_packet.crc)
      {
        /* Copy received configuration to active configuration and apply */
        _conf_revision_counter = _configuration_packet.revision_counter;
        memcpy(&_configuration_active, &(_configuration_packet.configuration), sizeof(_configuration_active));
        _iqsampling_conf_apply();
      }
    }
  }
}

/**
 * @brief  Binary I/Q sampling action; streams binary I/Q samples to host via UART.
 * @param  None
 * @retval None
 */
void BinaryIQSampling(void)
{
  /* Read command line parameters and print some debug output */
  uint32_t spimode = unsignedCommandArgument(0);
  uint32_t baudrate = unsignedCommandArgument(1);
  printf("Entering binary sampling mode. In this mode, the STM32WL3 communicates with\r\n");
  printf("a host device using a *binary* protocol over UART.\r\n");
  printf("Stream will only start after first binary configuration message is received.\r\n");
  printf("Use escape sequence ~.~. to exit! Switching to requested baudrate: %d\r\n", baudrate);
  if (spimode)
    printf("SPI mode has been activated. UART will only be used for configuration data.\r\n");

  /* Switch to different (usually higher) baudrate configured through command line parameter */
  uint32_t baudrate_backup = LL_USART_GetBaudRate(COM1_UART, USART_PERIPHCLK, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16);
  while (LL_USART_IsActiveFlag_TC(COM1_UART) == 0)
  {
  };
  LL_USART_SetBaudRate(COM1_UART, USART_PERIPHCLK, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16, baudrate);

  /* Initialize SPI interface if SPI mode is selected and allocate output buffers */
  if (spimode)
    BinaryIQSampling_SPI_Init();

  /* back up DATABUFFER0_PTR, DATABUFFER1_PTR and DATABUFFER_SIZE */
  uint32_t databuffer0_ptr_backup = MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR;
  uint32_t databuffer1_ptr_backup = MR_SUBG_GLOB_STATIC->DATABUFFER1_PTR;
  uint32_t databuffer_size_backup = MR_SUBG_GLOB_STATIC->DATABUFFER_SIZE;

  /* Perform VCO calibration */
  SET_BIT(MR_SUBG_GLOB_DYNAMIC->VCO_CAL_CONFIG, MR_SUBG_GLOB_DYNAMIC_VCO_CAL_CONFIG_VCO_CALIB_REQ);

  /* Reset HOST-to-STM32WL3 configuration channel parser */
  _iqsampling_conf_parser_reset();

  /* Allocate packets and make MR_SubG hardware write to it by setting their payloads as databuffers */
  volatile s3_iqsampling_packet packet_ringbuffer[PACKET_RINGBUFFER_COUNT];
  MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR = (uint32_t)&packet_ringbuffer[0].databuf[0];
  MR_SUBG_GLOB_STATIC->DATABUFFER1_PTR = (uint32_t)&packet_ringbuffer[1].databuf[0];
  MODIFY_REG_FIELD(MR_SUBG_GLOB_STATIC->DATABUFFER_SIZE, MR_SUBG_GLOB_STATIC_DATABUFFER_SIZE_DATABUFFER_SIZE, DATABUF_SIZE);
  uint8_t packet_ringbuffer_head = 0;
  uint8_t packet_ringbuffer_tail = 0;
  uint8_t packet_ringbuffer_full = 0;

  /* Prepare packets */
  for (uint8_t i = 0; i < PACKET_RINGBUFFER_COUNT; ++i)
  {
    packet_ringbuffer[i].header.sync_word = S3_IQSAMPLING_SYNC_WORD;
    packet_ringbuffer[i].header.samplecount = DATABUF_SIZE / 4;
  }

  /* Initialize raw I/Q sampling mode */
  LL_MRSubG_SetRXMode(RX_IQ_SAMPLING);
  __HAL_MRSUBG_STROBE_CMD(CMD_RX);

  /* Streaming loop */
  char c;
  uint64_t packetcounter = 0;
  uint8_t current = _current_databuffer();
  uint32_t lost_packets = 0;
  _terminate_samplingmode = 0;

  while (!_terminate_samplingmode)
  {
    /* Wait for next databuffer to get ready and do various actions in the meantime  */
    while (current == _current_databuffer())
    {
      /* Parse HOST-to-STM32WL3 binary configuration protocol */
      if (serialReadByte((uint8_t *)&c))
      {
        _iqsampling_conf_parser_parse(c);
      }

      /* In SPI mode, make sure that DMA hardware always transfers latest buffer */
      if (_configuration_active.running && spimode && BinaryIQSampling_SPI_TXIsComplete())
      {
        BinaryIQSampling_SPI_Finalize();
        if ((packet_ringbuffer_head != packet_ringbuffer_tail) || packet_ringbuffer_full)
        {
          if (packet_ringbuffer_full)
            printf("Warning: BinaryIQSampling on-device buffer overflow.\r\n");

          BinaryIQSampling_SPI_Transmit((uint8_t *)&packet_ringbuffer[packet_ringbuffer_tail], sizeof(s3_iqsampling_packet));
          packet_ringbuffer_tail = (packet_ringbuffer_tail + 1) % PACKET_RINGBUFFER_COUNT;
          packet_ringbuffer_full = 0;
        }
      }
    }

    /* Mark current packet as finished, move ringbuffer head to next packet */
    uint8_t finished_packet_idx = packet_ringbuffer_head;
    packet_ringbuffer_head = (packet_ringbuffer_head + 1) % PACKET_RINGBUFFER_COUNT;
    if (packet_ringbuffer_head == packet_ringbuffer_tail)
      packet_ringbuffer_full = 1;

    /* Prepare for packet *after* the next one: Make DATABUFFER0/1_PTR point to next empty slot in ring buffer */
    uint8_t next_head = (packet_ringbuffer_head + 1) % PACKET_RINGBUFFER_COUNT;
    current = _current_databuffer();
    if (current == 1)
      MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR = (uint32_t)&packet_ringbuffer[next_head].databuf[0];
    else
      MR_SUBG_GLOB_STATIC->DATABUFFER1_PTR = (uint32_t)&packet_ringbuffer[next_head].databuf[0];

    /* Fill in metadata for next (currently being written by SubGHz hardware) packet */
    packet_ringbuffer[packet_ringbuffer_head].header.packetcounter = packetcounter++;
    packet_ringbuffer[packet_ringbuffer_head].header.conf_revision = _conf_revision_counter;
    packet_ringbuffer[packet_ringbuffer_head].header.sample_rate = 16000000 / (8 * (1 << READ_REG_FIELD(MR_SUBG_GLOB_DYNAMIC->MOD1_CONFIG, MR_SUBG_GLOB_DYNAMIC_MOD1_CONFIG_CHFLT_E)));
    packet_ringbuffer[packet_ringbuffer_head].header.rssi = READ_REG_FIELD(MR_SUBG_GLOB_STATUS->RX_INDICATOR, MR_SUBG_GLOB_STATUS_RX_INDICATOR_RSSI_LEVEL_RUN);
    packet_ringbuffer[packet_ringbuffer_head].header.register_override_error = _register_override_error;
    packet_ringbuffer[packet_ringbuffer_head].header.lost_packets = lost_packets;
    lost_packets = 0;

    /* If sample transmission is active, transmit packet that the current databuffer belongs to */
    if (_configuration_active.running && !spimode)
    {
      /*
       * UART mode: Send packet that was just read directly via UART, *bypassing* ring buffer (only used in SPI mode)
       * While waiting until UART is ready, count number of (potentially) lost packets
       */
      for (size_t i = 0; i < sizeof(s3_iqsampling_packet); ++i)
      {
        while (LL_USART_IsActiveFlag_TC(COM1_UART) == 0)
        {
          if (current != _current_databuffer())
          {
            lost_packets++;
            current = _current_databuffer();
          }
        };
        LL_USART_TransmitData8(COM1_UART, *((uint8_t *)&packet_ringbuffer[finished_packet_idx] + i));
      }
    }
  }

  /* Stop Receiver */
  __HAL_MRSUBG_STROBE_CMD(CMD_SABORT);

  /* Revert baudrate to configuration before entering I/Q sampling mode */
  while (LL_USART_IsActiveFlag_TC(COM1_UART) == 0)
  {
  };
  LL_USART_SetBaudRate(COM1_UART, USART_PERIPHCLK, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16, baudrate_backup);

  /* de-initialize SPI interface */
  if (spimode)
    BinaryIQSampling_SPI_DeInit();

  /* restore databuffer-related backups */
  MR_SUBG_GLOB_STATIC->DATABUFFER0_PTR = databuffer0_ptr_backup;
  MR_SUBG_GLOB_STATIC->DATABUFFER1_PTR = databuffer1_ptr_backup;
  MR_SUBG_GLOB_STATIC->DATABUFFER_SIZE = databuffer_size_backup;
}
