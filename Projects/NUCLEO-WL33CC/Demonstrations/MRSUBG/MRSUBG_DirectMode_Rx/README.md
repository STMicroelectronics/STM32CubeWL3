::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_DirectMode_Rx Example Description</b>

This example shows how to operate the MRSUBG Radio in "Direct through GPIO" Rx to receive and decode packets with OOK modulation.

When in direct through GPIO mode, MRSUBG outputs RX clock on PB2 and RX data on PB1.
RX data is looped to PB10 (TIM2 CH2) to capture RX data rising and falling edges and decode the packet (sync word + payload).
The default configuration will listen to OOK packet with a 0x88888888 sync word, a 16-byte payload, and a 2400 bps data rate.
If CRC is enabled at build time (CRC_SIZE = 1), the firmware performs a CRC check on the decoded packet and prints the result on the virtual COM port.
LED2 blinks on when a valid packet is detected.

#### <b>Notes</b>

 Transmitter RF and PHY parameters must be compatible with the receiver radio settings.
 To test this demonstration, the following MRSUBG configuration was used on the transmitter board:
 
  - MRSUBG_PacketSettingsStruct.PreambleLength = 16;
  - MRSUBG_PacketSettingsStruct.PostambleLength = 0;
  - MRSUBG_PacketSettingsStruct.SyncLength = 31;
  - MRSUBG_PacketSettingsStruct.SyncWord = 0x88888888;
  - MRSUBG_PacketSettingsStruct.FixVarLength = FIXED;
  - MRSUBG_PacketSettingsStruct.PreambleSequence = PRE_SEQ_0101;
  - MRSUBG_PacketSettingsStruct.PostambleSequence = POST_SEQ_0101;
  - MRSUBG_PacketSettingsStruct.CrcMode = PKT_CRC_MODE_8BITS;
  - MRSUBG_PacketSettingsStruct.DataWhitening = DISABLE;
  - MRSUBG_PacketSettingsStruct.LengthWidth = BYTE_LEN_1;
  - MRSUBG_PacketSettingsStruct.SyncPresent = ENABLE;

### <b>Keywords</b>

MRSUBG, Radio, OOK modulation

### <b>Directory contents</b>

  - .extSettings/                         STM32CubeMX settings directory
  - .mxproject                            STM32CubeMX project file
  - readme.html                           HTML version of this README
  - README.md                             This file
  - Core/Inc/app_entry.h                  Application entry header
  - Core/Inc/main.h                       Header for main.c module
  - Core/Inc/stm32wl3x_hal_conf.h         HAL configuration file
  - Core/Inc/stm32wl3x_it.h               Interrupt handlers header file
  - Core/Inc/stm32wl3x_nucleo_conf.h      BSP configuration file
  - Core/Src/app_entry.c                  Application entry point and OOK decoder
  - Core/Src/main.c                       Main program
  - Core/Src/stm32wl3x_hal_msp.c          HAL MSP file
  - Core/Src/stm32wl3x_it.c               Interrupt handlers
  - Core/Src/system_stm32wl3x.c           STM32WL3x system source file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices

  - This example has been tested with NUCLEO-WL33CC1 board and can be
    easily tailored to any other supported device and development board
  
  - Test environment setup
     - MRSUBG RX data output on PB1 must be connected to TIM2 Channel2 input on PB10
       On a NUCLEO-WL3CCx, this is achieved connecting CN7-A2 to CN6-D13 on the Arduino board
     - MRSUBG radio settings have been tested OK with a transmitter output power of 0 dBm, at about 20-30 cm from the receiver Nucleo (wireless communication)
       If test conditions vary, it may be required to adapt the RSSI threshold of the receiver and/or adjust transmitter output power

### <b>How to use it?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
:::
:::