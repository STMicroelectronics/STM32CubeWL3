---
pagetitle: Readme for STM32CubeWL3 SFUOTA Tx Demonstration
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>SFUOTA_Tx Demonstration Description</b>

How to configure a simple TX application with over-the-air (OTA) upgrade capability.

This code runs a TX scenario that sends packets in sequence using TX_DONE polling.
The last byte of each packet contains a sequential counter value, and after a burst of MAX_NUM_PACKET transmissions, frequency and packet count information are printed on the terminal.
Sleep management is integrated to conserve power.
PB2 is used to request a jump to OTA reset manager mode through a software reset sequence.
The firmware image is built with the correct offset to ensure it is compatible with the OTA client, preventing overwriting of the OTA client area during the firmware upgrade process.

### <b>Keywords</b>

Connectivity, MRSUBG, OTA, Sub-GHz radio

### <b>Directory contents</b>

  - Binary/SFUOTA_Tx.bin                    Prebuilt TX image used as OTA payload candidate
  - Core/Inc/app_conf.h                     Application configuration file
  - Core/Inc/app_entry.h                    Interface header for application entry
  - Core/Inc/main.h                         Header file for main.c
  - Core/Inc/stm32_assert.h                 STM32 assert file
  - Core/Inc/stm32wl3x_hal_conf.h           HAL Configuration file
  - Core/Inc/stm32wl3x_it.h                 Header for stm32wl3x_it.c
  - Core/Inc/stm32wl3x_nucleo_conf.h        BSP configuration file
  - Core/Inc/utilities_conf.h               Utilities configuration file
  - Core/Src/app_entry.c                    Application entry implementation
  - Core/Src/main.c                         Main program
  - Core/Src/stm32wl3x_hal_msp.c            HAL MSP module
  - Core/Src/stm32wl3x_it.c                 Interrupt handlers
  - Core/Src/system_stm32wl3x.c             STM32WL3x system clock configuration file
  - System/Interfaces/stm32_lpm_if.c        Low power mode configuration file
  - System/Interfaces/stm32_lpm_if.h        Header for stm32_lpm_if.c
  - System/Modules/asm.h                    ASM Compiler-dependent macros file
  - System/Modules/osal.c                   OS abstraction layer definition file
  - System/Modules/osal.h                   Header for osal.c
  - System/Startup/cpu_context_switch.s     Context restore file
  - System/Startup/device_context_switch.c  STM32WL3 context switch file
  - System/Startup/device_context_switch.h  Header for device_context_switch.c

### <b>Hardware and Software environment</b>

  - This demonstration runs on STM32WL33CCVx devices.
  - This demonstration has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board.

### <b>How to use it?</b>

This application is configured to be placed at a defined offset in FLASH, through the use of the MEMORY_FLASH_APP_OFFSET linker symbol.
As such, it cannot start right away, but an OTA client is needed to launch it.
Default offset is 0x7000.

A pre-built binary image is provided, this has to be used in conjunction with the SFUOTA_ServerYmodem application (see the corresponding README for further details)
If you want to modify/re-build the application, you must do the following:

 - Open your preferred toolchain (IAR EWARM project is provided)
 - Rebuild all files and generate a new target image

### <b>Static configuration</b>

Key runtime configuration is defined in app_entry.c, including:

 - TX packet format, payload length, and transmission burst settings
 - TX_DONE polling flow for packet transmission completion
 - OTA reset manager jump trigger on PB2
Default runtime values are MAX_NUM_PACKET=100, TX_PAYLOAD_LEN=7, MRSUBG_TX_CHANNEL_HZ=868000000, and TX_INTER_PACKET_DELAY_MS=100.

### <b>Notes</b>

 1. This demonstration currently provides IAR EWARM project files.

:::
:::