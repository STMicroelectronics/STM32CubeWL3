---
pagetitle: Readme for STM32CubeWL3 SFUOTA Server Ymodem Demonstration
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>SFUOTA_ServerYmodem Demonstration Description</b>

How to configure a simple Sub-GHz SFUOTA server application with Ymodem image input.

This code runs a Ymodem + SFUOTA server flow:
- firmware chunks are received over COM1 (Ymodem),
- chunks are served to SFUOTA client over MRSUBG packet exchange.
The code is split in two layers: SFUOTA/Protocol holds the server state machine, the over-the-air frame contract and the Ymodem receiver, while SFUOTA/Target holds the MRSUBG driver and the RAM staging window that buffers one chunk at a time.


The application supports three build-time modes, selected through SFUOTA_SERVER_MODE in app_conf.h: Ymodem input plus over-the-air transfer, Ymodem input only with the radio left idle, and over-the-air transfer only with a simulated image.

### <b>Keywords</b>

Connectivity, MRSUBG, OTA, Sub-GHz radio

### <b>Directory contents</b>

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
  - SFUOTA/Protocol/sfuota_server.c         SFUOTA server state machine and Ymodem arbitration
  - SFUOTA/Protocol/sfuota_server.h         SFUOTA server state machine interface
  - SFUOTA/Protocol/sfuota_frames.h         Over-the-air frame headers and payload size
  - SFUOTA/Protocol/ymodem.c                Ymodem protocol implementation
  - SFUOTA/Protocol/ymodem.h                Ymodem protocol interface
  - SFUOTA/Target/sfuota_radio.c            MRSUBG transmit, receive and IRQ event handling
  - SFUOTA/Target/sfuota_radio.h            Header for sfuota_radio.c
  - SFUOTA/Target/sfuota_image.c            RAM staging window holding one Ymodem chunk at a time
  - SFUOTA/Target/sfuota_image.h            Header for sfuota_image.c
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

 - Open the project with your preferred toolchain (EWARM, MDK-ARM and STM32CubeIDE projects are provided)
 - Rebuild all files and load your image into target memory
 - Start SFUOTA_Client on the target node to pull image chunks over MRSUBG
 - Run the Server app, use a Ymodem terminal to select an application image to send to the client.
 
   E.g. if using "Tera Term" go to File > Transfer > YMODEM > Send... and select the binary to use
   - Sample application images are provided (SFUOTA_Tx/Binary/SFUOTA_Tx.bin and SFUOTA_Rx/Binary/SFUOTA_Rx.bin)
 

### <b>Static configuration</b>

The SFUOTA server uses MRSUBG with packet exchange managed in sfuota_radio.c.

Key configuration points include:

 - SFUOTA frame headers, sequence handling, and data request flow
 - Ymodem packet reception and buffering over COM1
 - Server mode selected at build time with SFUOTA_SERVER_MODE (SFUOTA_MODE_FULL, SFUOTA_MODE_YMODEM, SFUOTA_MODE_RADIO)
 - Optional diagnostics in sfuota_server.c: SFUOTA_TRACE_TX_FRAMES dumps every transmitted frame, SFUOTA_CAPTURE_PAYLOAD mirrors the beginning of the image in RAM

These settings can be found in files app_conf.h, sfuota_frames.h, sfuota_server.c and ymodem.c/.h.

### <b>Notes</b>

 1. SFUOTA_Client and SFUOTA_ServerYmodem are linked at the beginning of the user flash (offset 0), while SFUOTA_Rx and SFUOTA_Tx are linked at offset 0x7000, right after the SFUOTA service manager.

 2. All the provided projects are built with size optimization in every build configuration (on STM32CubeIDE the Debug configuration is set to size optimization as well, instead of the default no-optimization level). This is required because SFUOTA_Client and SFUOTA_ServerYmodem must fit in the 0x7000 bytes reserved to the SFUOTA service manager: with a lower optimization level the image exceeds that limit and overwrites the beginning of the application area.

 3. Whatever toolchain is used, the application binary is generated in the Binary folder of the project, so that the image to be transferred with Ymodem is always available.

:::
:::
