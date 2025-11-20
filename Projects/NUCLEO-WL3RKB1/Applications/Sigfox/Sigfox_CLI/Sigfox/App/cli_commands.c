/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    cli_commands.c
  * @author  GPM WBL Application Team
  * @brief   Application of the Sigfox Middleware
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
#include <stdio.h>
#include "main.h"
#include "command-interpreter2.h"
#include "cli_commands.h"

sfx_error_t _printVersion(sfx_version_type_t versionType);

extern uint8_t responsePrintf(const char* formatString, ...);

static uint32_t id;
static uint8_t pac[8];
static uint8_t rc = 1;
static uint32_t config_words[3];
static uint32_t timer_enable;
static uint8_t _isPublicKey = 0;
static uint8_t _testModeActive = 0;

void set_id(uint32_t id_new)
{
  id = id_new;
}

void set_pac(uint8_t* pac_new)
{
  for (uint8_t i = 0; i < 8; i++) {
    pac[i] = pac_new[i];
  }
}

void set_rc(uint8_t rc_new)
{
  rc = rc_new;
}

/*
 * Set _testModeActive for non-registered boards
 */
void set_testMode(uint8_t enableTestMode)
{
  _testModeActive = enableTestMode;

  ST_SFX_SetTestCredentials(_testModeActive);
}

uint8_t get_testMode()
{
  return _testModeActive;
}

void ST_get_id_Action(void)
{
  responsePrintf("{&N API call...&t4x}\r\n", "get_id", "id", id);
}

void ST_get_pac_Action(void)
{
  responsePrintf("{&N API call...\r\n", "get_pac");
  responsePrintf("{%t: ", "pac");

  for (uint16_t i = 0; i < sizeof(pac); i++) {
    printf("%.2X", pac[i]);
  }
  responsePrintf("}}\r\n");
}

void ST_get_rc_Action(void)
{
  responsePrintf("{&N API call...&tx}\r\n", "get_rcz", "rcz", rc);
}

void ST_set_rc_Action(void)
{
  uint8_t rc_new = unsignedCommandArgument(0);

  if ((rc_new == 2 || rc_new == 4) && (rc == 2 || rc == 4)) {
    rc = rc_new;
  }

  responsePrintf("{&N API call...&tx}\r\n", "set_rc", "rc", rc);
}

void node_close_Action(void)
{
  sfx_error_t err;

  err = SIGFOX_API_close();
  responsePrintf("{&N API call...&tx}\r\n", "node_close", "sfx_error", err);
}

void node_reset_Action(void)
{
  sfx_error_t err = 0;

  responsePrintf("{&N API call...&tx}\r\n", "node_reset", "sfx_error", err);
}

void node_open_Action(void)
{
  uint32_t err = 0x80000001;

  ST_SFX_SetPAEnabled(0);

  if (rc == 1) {
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC1);
  } else if (rc == 2) {
    ST_SFX_SetPAEnabled(1);
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC2);
  } else if (rc == 3) {
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC3C);
  } else if (rc == 4) {
    ST_SFX_SetPAEnabled(1);
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC4);
  } else if (rc == 5) {
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC5);
  } else if (rc == 6) {
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC6);
  } else if (rc == 7) {
    err = (uint32_t)SIGFOX_API_open(&(sfx_rc_t)RC7);
  }

  responsePrintf("{&N API call...&t4x}\r\n", "node_open", "sfx_error", err);
}

void node_open_with_zone_Action(void)
{
  uint8_t user_rc = unsignedCommandArgument(0);
  uint8_t rc_backup = rc;

  rc = user_rc;
  node_open_Action();
  rc = rc_backup;
}

void node_get_info_Action(void)
{
  sfx_error_t err;
  uint8_t info;

  err = SIGFOX_API_get_info(&info);
  responsePrintf("{&N API call...&tx&tx}\r\n", "node_get_info", "info", info, "sfx_error", err);
}

void node_get_version_Action(void)
{
  responsePrintf("{&N API call...\r\n", "node_get_version");
  _printVersion(VERSION_SIGFOX);
}

void node_send_frame_Action(void)
{
  sfx_error_t err;
  uint8_t customer_data[256];
  uint8_t customer_data_len = copyStringArgument(0, customer_data, 255, 0);
  uint8_t tx_repeat = unsignedCommandArgument(1);
  uint8_t initiate_downlink_flag = (uint8_t)unsignedCommandArgument(2);
  uint8_t customer_resp[8] = { 0 };

  err = SIGFOX_API_send_frame(customer_data, customer_data_len, customer_resp, tx_repeat, initiate_downlink_flag);
  responsePrintf("{&N API call...&tx\r\n", "node_send_frame", "sfx_error", err);

  if (initiate_downlink_flag && !err) {
    responsePrintf("{%t: ", "customer_resp");
    for (uint16_t i = 0; i < 7; i++) {
      responsePrintf("0x%x,", customer_resp[i]);
    }
    responsePrintf("0x%x}\n\r", customer_resp[7]);
  }
  responsePrintf("}\n\r");
}

void node_test_mode_Action(void)
{
  sfx_error_t err = 0;
  sfx_rc_enum_t rc = (sfx_rc_enum_t)unsignedCommandArgument(0);
  sfx_test_mode_t test_mode = (sfx_test_mode_t)unsignedCommandArgument(1);
  sfx_u8 rolloverCounter = 0;

  /* PA management */
  ST_SFX_SetPAEnabled(rc == SFX_RC2 || rc == SFX_RC4);

  ST_SFX_GetRolloverCounter(&rolloverCounter);
  ST_SFX_SetTestCredentials(1);

  if (_isPublicKey)
    ST_SFX_SetPublicKey(1);

  err = ADDON_SIGFOX_RF_PROTOCOL_API_test_mode(rc, test_mode);

  // Revert to pre-test key only for registered device
  // Non-registered boards will keep the test id and key
  if (!_testModeActive) {
    ST_SFX_SetTestCredentials(0);

    if (_isPublicKey)
      ST_SFX_SetPublicKey(1);
  }

  responsePrintf("{&N API call...&td &t2x}\r\n", "node_test_mode", "rollover counter", rolloverCounter, "sfx_error", err);
}

void node_monarch_test_mode_Action(void)
{
  sfx_error_t err = 0;

  sfx_rc_enum_t rc = (sfx_rc_enum_t)unsignedCommandArgument(0);
  sfx_test_mode_t test_mode = (sfx_test_mode_t)unsignedCommandArgument(1);
  sfx_u8 rc_capabilities = (sfx_u8)unsignedCommandArgument(2); // 31 up to RC5,  63 up to RC6

  ST_SFX_SetPAEnabled(unsignedCommandArgument(0) == 1 || unsignedCommandArgument(0) == 4);

#ifdef MONARCH_CLI_TESTS
  if (unsignedCommandArgument(1) > 6)
    ST_SFX_ReduceOutputPower(80);
  else if (unsignedCommandArgument(1) == 4)
    ST_SFX_ReduceOutputPower(80);
#endif
  ST_SFX_SetTestCredentials(1);

  if (_isPublicKey)
    ST_SFX_SetPublicKey(1);

  err = ADDON_SIGFOX_RF_PROTOCOL_API_monarch_test_mode(rc, test_mode, rc_capabilities);

  ST_SFX_SetTestCredentials(0);

  if (_isPublicKey)
    ST_SFX_SetPublicKey(1);

  responsePrintf("{&N API call...&tx}\r\n", "ADDON_SIGFOX_RF_PROTOCOL_API_monarch_test_mode", "sfx_error", err);
}

void get_lib_version_Action(void)
{
  sfx_version_type_t libType = (sfx_version_type_t)unsignedCommandArgument(0);

  responsePrintf("{&N API call...\r\n", "get_lib_version");
  _printVersion(libType);
}

void ST_set_low_power_Action(void)
{
  /* Not implemented */

  responsePrintf("{&N API call...}\r\n", "set_low_power");
}

void EepromWritePageAction(void)
{
  /* Not implemented */
  responsePrintf("{&N API call...}\r\n", "_e2prom_write_page");
}

void EepromReadPageAction(void)
{
  uint8_t cNbBytes = unsignedCommandArgument(2);

  /* Not implemented - pretend EEPROM is all 0x00 */
  responsePrintf("{&N API callback...\r\n", "_e2prom_read_page");
  responsePrintf("{%t: ", "Data");

  for (uint8_t i = 0; i < cNbBytes - 1; i++)
    responsePrintf("0x%x,", 0x00);

  responsePrintf("0x%x", 0x00);
  responsePrintf("}}\r\n");
}

void node_set_std_config_Action(void)
{
  sfx_error_t err;

  for (uint32_t i = 0; i < 3; i++)
    config_words[i] = unsignedCommandArgument(i);

  timer_enable = (uint16_t)unsignedCommandArgument(3);

  err = SIGFOX_API_set_std_config((sfx_u32*)config_words, timer_enable);

  responsePrintf("{&N API call...&tx}\r\n", "node_set_std_config", "sfx_error", err);
}

void node_get_std_config_Action(void)
{

  responsePrintf("{&N API call...\r\n", "node_get_std_config");

  responsePrintf("{%t: ", "config_words");

  for (uint8_t i = 0; i < 2; i++)
    responsePrintf("0x%4x,", config_words[i]);
  responsePrintf("0x%4x", config_words[2]);

  responsePrintf("}\r\n");
  responsePrintf("&t2x", "timer_enable", timer_enable);
  responsePrintf("&t4x}\r\n", "sfx_error", 0);
}

void node_set_public_key_Action(void)
{
  uint8_t en = unsignedCommandArgument(0);
  sfx_error_t err = 0;

  ST_SFX_SetPublicKey(en);
  _isPublicKey = en;

  responsePrintf("{&N API call...&t2x}\r\n", "switch_public_key", "sfx_error", err);
}

void node_set_test_credentials_Action(void)
{
  uint8_t val = unsignedCommandArgument(0);
  sfx_error_t err = 0;

  set_testMode(val);

  responsePrintf("{&N API call...&t2x}\r\n", "switch_test_credentials", "sfx_error", err);
}

/*****************************************************************************/
/*		               XTAL FREQUENCY OFFSET 																 */
/*****************************************************************************/
void set_xtal_frequency_offset_Action(void)
{
  sfx_error_t err = 0;
  sfx_s32 xtalCompValue = (int32_t)signedCommandArgument(0); /* The Xtal drift */
  sfx_u32 xtal_freq;

  /* Retrieve base, uncompensated XTal frequency */
  xtal_freq = LL_GetXTALFreq();

  /*xtalCompValue is a value measured during manufacturing as follows:
  xtalCompValue=fnominal-fmeasured. To compensate such value it should
  be reported to xtal freq and then subtracted*/
  err = ST_SFX_SetXtalFreq(xtal_freq + xtalCompValue); /* Override RF_API Xtal value */

  responsePrintf("{&N API call...&tx}\r\n", "set_xtal_frequency_offset", "sfx_error", err);
}

void get_xtal_frequency_Action(void)
{
  sfx_error_t err = 0;

  uint32_t xtal_freq = ST_SFX_GetXtalFreq();

  responsePrintf("{&N API call...&t4x &tx}\r\n", "get_xtal_frequency", "xtal_freq", xtal_freq, "sfx_error", err);
}

/*****************************************************************************/
/*						RSSI OFFSET						*/
/*****************************************************************************/
void VENDOR_set_rssi_offset_Action(void)
{
  sfx_s8 rssi_offset = (sfx_s8)signedCommandArgument(0);
  sfx_error_t err;

  err = ST_SFX_SetRSSIOffset(rssi_offset);

  responsePrintf("{&N API call...&tx}\r\n", "ST_MANUF_API_set_rssi_offset", "sfx_error", err);
}

void VENDOR_get_rssi_offset_Action(void)
{
  sfx_error_t err;
  sfx_s8 rssi_offset;

  err = ST_SFX_GetRSSIOffset(&rssi_offset);

  responsePrintf("{&N API call...&td &tx}\r\n", "ST_MANUF_API_get_rssi_offset", "rssi_offset", rssi_offset, "sfx_error", err);
}
/*****************************************************************************/
/*                            LBT THRESHOLD OFFSET                           */
/*****************************************************************************/
void set_lbt_thr_offset_Action(void)
{
  sfx_s8 lbt_thr = (sfx_s8)signedCommandArgument(0);
  sfx_s8 err = ST_SFX_SetLBTThresholdOffset(lbt_thr);

  responsePrintf("{&N API call...&tx}\r\n", "set_lbt_thr_offset", "sfx_error", err);
}

void get_lbt_thr_offset_Action(void)
{
  sfx_error_t err;
  sfx_s8 lbt_thr;

  err = ST_SFX_GetLBTThresholdOffset(&lbt_thr);

  responsePrintf("{&N API call...&td &tx}\r\n", "ST_MANUF_API_get_rssi_offset", "lbt_thr", lbt_thr, "sfx_error", err);
}

void VENDOR_PayloadEncription(void)
{
  sfx_s8 enable_encryption = (sfx_s8)unsignedCommandArgument(0);
  sfx_error_t err = 0;

  ST_SFX_SetPayloadEncryption(enable_encryption);

  responsePrintf("{&N API call...&tx}\r\n", "VENDOR_set_payload_encryption", "sfx_error", err);
}

void SIGFOX_API_start_continuous_transmission_Action(void)
{
  sfx_u32 frequency = (uint32_t)unsignedCommandArgument(0);
  sfx_modulation_type_t mode = (sfx_modulation_type_t)unsignedCommandArgument(1);
  sfx_error_t err;

  err = SIGFOX_API_start_continuous_transmission(frequency, mode);

  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_API_start_continuous_transmission", "sfx_error", err);
}

void SIGFOX_API_stop_continuous_transmission_Action(void)
{
  sfx_error_t err;

  err = SIGFOX_API_stop_continuous_transmission();

  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_API_stop_continuous_transmission", "sfx_error", err);
}

void reduce_output_power_Action(void)
{
  sfx_error_t err = 0;
  sfx_s16 reduction = (int16_t)unsignedCommandArgument(0);

  ST_SFX_ReduceOutputPower(reduction);

  responsePrintf("{&N API call...&t2x}\r\n", "reduce_output_power", "sfx_error", err);
}

void switch_pa_Action(void)
{

  ST_SFX_SetPAEnabled((int8_t)unsignedCommandArgument(0));

  responsePrintf("{&N API call...}\r\n", "switch_pa");
}

void SIGFOX_API_send_out_of_band_Action()
{
  sfx_error_t err;

  err = SIGFOX_API_send_outofband(SFX_OOB_RC_SYNC);

  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_API_send_out_of_band", "sfx_error", err);
}

void SIGFOX_API_set_rc_sync_period_Action()
{
  sfx_error_t err;

  sfx_u16 period = (int16_t)unsignedCommandArgument(0);

  err = SIGFOX_API_set_rc_sync_period(period);

  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_API_set_rc_sync_period", "sfx_error", err);
}

sfx_error_t _printVersion(sfx_version_type_t versionType)
{
  sfx_error_t err;
  uint8_t* version_p;
  uint8_t size;

  err = SIGFOX_API_get_version(&version_p, &size, versionType);

  responsePrintf("{%t: ", "version");

  if (!err) {
    for (uint8_t i = 0; i < size; i++)
      responsePrintf("%c", version_p[i]);

    responsePrintf("}\r\n");
  } else
    responsePrintf("--- }\r\n");

  responsePrintf("&t4x\r\n", "sfx_error", err);
  responsePrintf("}\r\n");

  return err;
}

void SIGFOX_API_send_bit_Action(void)
{
  sfx_error_t err;
  sfx_u8 bit_value = unsignedCommandArgument(0);
  sfx_u8 tx_repeat = unsignedCommandArgument(1);
  sfx_u8 customer_resp[100];

  sfx_bool initiate_downlink_flag = (sfx_bool)unsignedCommandArgument(2);
  err = SIGFOX_API_send_bit(bit_value, customer_resp, tx_repeat, initiate_downlink_flag);
  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_API_send_bit", "sfx_error", err);
}

/*****************************************************************************/
/*                     MONARCH SCAN FUNCTIONS                                */
/*****************************************************************************/
/* Callback called from monarch rc scan */
sfx_u8 callback_for_found(sfx_u8 rc_bit_mask, sfx_s16 rssi)
{
  printf("return rc_bit_mask %d\r\n", rc_bit_mask);
  printf("return rssi %d\r\n", rssi);

  switch (rc_bit_mask) {
  case 0x01: // RC1
  {
    printf("Detected RC1!!!:\r\n");
  } break;
  case 0x02: // RC2
  {
    printf("Detected RC2!!!:\r\n");
  } break;
  case 0x04: // RC3a
  {
    printf("Detected RC3!!!:\r\n");
  } break;
  case 0x08: // RC4
  {
    printf("Detected RC4!!!:\r\n");
  } break;
  case 0x10: // RC5
  {
    printf("Detected RC5!!!:\r\n");
  } break;
  case 0x20: // RC6
  {
    printf("Detected RC6!!!:\r\n");
  } break;
  case 0x40: // RC7
  {
    printf("Detected RC7!!!:\r\n");
  } break;
  }

  return 1;
}

void node_execute_monarch_scan_Action(void)
{
  sfx_error_t err;
  err = SIGFOX_MONARCH_API_execute_rc_scan((sfx_u8)unsignedCommandArgument(0), (sfx_u16)unsignedCommandArgument(1),
    (sfx_timer_unit_enum_t)unsignedCommandArgument(2), callback_for_found);
  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_MONARCH_API_execute_rc_scan", "sfx_error", err);
}

void node_stop_monarch_scan_Action(void)
{
  sfx_error_t err;
  err = SIGFOX_MONARCH_API_stop_rc_scan();
  responsePrintf("{&N API call...&tx}\r\n", "SIGFOX_MONARCH_API_stop_rc_scan_Action", "sfx_error", err);
}
