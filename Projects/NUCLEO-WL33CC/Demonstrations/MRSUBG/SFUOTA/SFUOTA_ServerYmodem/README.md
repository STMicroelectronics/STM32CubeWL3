---
pagetitle: Readme for STM32CubeWL3 SFUOTA Server Ymodem Demonstration
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>SFUOTA_ServerYmodem Demonstration Description</b>

How to configure a simple Sub-GHz OTA server application with Ymodem image input.

This code runs a Ymodem + OTA server flow:
- firmware chunks are received over COM1 (Ymodem),
- chunks are served to OTA client over MRSUBG packet exchange.
The OTA/Ymodem protocol handling is implemented in radio_ota.c/.h and ymodem.c/.h.


The application supports YMODEM + OTA mode and a YMODEM debug-only mode controlled at build time.

### <b>Keywords</b>

Connectivity, MRSUBG, OTA, Sub-GHz radio

### <b>Directory contents</b>

  - Core/Inc/app_conf.h                     Application configuration file
  - Core/Inc/app_entry.h                    Interface header for application entry
  - Core/Inc/main.h                         Header file for main.c
  - Core/Inc/radio_ota.h                    OTA server protocol definitions and server state machine interface
  - Core/Inc/ymodem.h                       Ymodem protocol interface
  - Core/Inc/stm32_assert.h                 STM32 assert file
  - Core/Inc/stm32wl3x_hal_conf.h           HAL Configuration file
  - Core/Inc/stm32wl3x_it.h                 Header for stm32wl3x_it.c
  - Core/Inc/stm32wl3x_nucleo_conf.h        BSP configuration file
  - Core/Inc/utilities_conf.h               Utilities configuration file
  - Core/Src/app_entry.c                    Application entry implementation
  - Core/Src/main.c                         Main program
  - Core/Src/radio_ota.c                    OTA server protocol + Ymodem/OTA arbitration implementation
  - Core/Src/ymodem.c                       Ymodem protocol implementation
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

In order to make the program work, you must do the following:

 - Open your preferred toolchain (IAR EWARM project is provided)
 - Rebuild all files and load your image into target memory
 - Start SFUOTA_Client on the target node to pull image chunks over MRSUBG
 - Run the Server app, use a Ymodem terminal to select an application image to send to the client.
 
   E.g. if using "Tera Term" go to File > Transfer > YMODEM > Send... aand select the binary to use
   - Sample application images are provided (SFUOTA_Tx/Binary/SFUOTA_Tx.bin and SFUOTA_Rx/Binary/SFUOTA_Rx.bin)
 

### <b>Static configuration</b>

The OTA server uses MRSUBG with packet exchange managed in radio_ota.c.

Key configuration points include:

 - OTA protocol headers, sequence handling, and data request flow
 - Ymodem packet reception and buffering over COM1
 - Optional YMODEM_DEBUG_ONLY and OTA_DEBUG_ONLY build-time modes

These settings can be found in files app_entry.c, radio_ota.c/.h, and ymodem.c/.h.

### <b>Notes</b>

 1. This demonstration currently provides IAR EWARM project files.

:::
:::
