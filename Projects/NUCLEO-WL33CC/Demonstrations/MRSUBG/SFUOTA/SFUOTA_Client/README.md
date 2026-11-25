---
pagetitle: Readme for STM32CubeWL3 SFUOTA Client Demonstration
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>SFUOTA_Client Demonstration Description</b>

How to configure a simple client application for Over-The-Air firmware update (FUOTA) using SubGHz communication.

This code runs a FUOTA client state machine and communicates over MRSUBG to receive and program a new application image in the device FLASH.
The code is split in two layers: SFUOTA/Protocol holds the transport-independent state machine and the over-the-air frame contract, while SFUOTA/Target holds the MRSUBG driver, the flash programming, the flash map and the boot decision.
When the download is completed and validated, the client jumps to the newly programmed application.

At runtime, SFUOTA transfer is managed by SFUOTA_Init()/SFUOTA_Tick() and radio IRQ callbacks.
The client receives SFUOTA data chunks, assembles FLASH pages, and writes them at SFUOTA_MAP_APP_ADDR.
Once the new application is running, PB2 is used to request a jump back to the SFUOTA client mode for a new download session.
The firmware image and flash layout are configured with dedicated offsets so that SFUOTA service manager and target application areas do not overlap.

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
  - SFUOTA/Protocol/sfuota_client.c         SFUOTA client state machine implementation
  - SFUOTA/Protocol/sfuota_client.h         SFUOTA client state machine interface
  - SFUOTA/Protocol/sfuota_frames.h         Over-the-air frame headers and payload size
  - SFUOTA/Target/sfuota_radio.c            MRSUBG transmit, receive and IRQ event handling
  - SFUOTA/Target/sfuota_radio.h            Header for sfuota_radio.c
  - SFUOTA/Target/sfuota_image.c            Flash erase and page programming of the received image
  - SFUOTA/Target/sfuota_image.h            Header for sfuota_image.c
  - SFUOTA/Target/sfuota_boot.c             Image tagging, image selection and jump to application
  - SFUOTA/Target/sfuota_boot.h             Header for sfuota_boot.c
  - SFUOTA/Target/sfuota_flashmap.h         Flash layout of manager, NVM and application areas
  - System/Interfaces/stm32_lpm_if.c        Low power mode configuration file
  - System/Interfaces/stm32_lpm_if.h        Header for stm32_lpm_if.c
  - System/Modules/asm.h                    ASM Compiler-dependent macros file
  - System/Modules/osal.c                   OS abstraction layer definition file
  - System/Modules/osal.h                   Header for osal.c
  - System/Startup/cpu_context_switch.s     Context restore file
  - System/Startup/device_context_switch.c  STM32WL3 context switch file
  - System/Startup/device_context_switch.h  Header for device_context_switch.c

### <b>Hardware and Software environment</b>

  - This demonstration runs on STM32WL33CCVx devices
  - This demonstration has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board

### <b>How to use it?</b>

In order to make the program work, you must do the following:

 - Open the project with your preferred toolchain (EWARM, MDK-ARM and STM32CubeIDE projects are provided)
 - Program SFUOTA_ServerYmodem first, so SFUOTA data can be served to clients
 - Rebuild all files and load your image into target memory
 - Run the demonstration and follow the Ymodem host procedure to transfer the image

### <b>Static configuration</b>

The SFUOTA client uses MRSUBG with packet exchange managed in sfuota_radio.c.

Key configuration points include:

 - SFUOTA frame headers and sequence handling
 - Flash target layout, tags, and page programming policy
 - SFUOTA service manager jump control through retained RAM variable

These settings can be found in files sfuota_frames.h, sfuota_flashmap.h and sfuota_client.c.

### <b>Notes</b>

 1. SFUOTA_Client and SFUOTA_ServerYmodem are linked at the beginning of the user flash (offset 0), while SFUOTA_Rx and SFUOTA_Tx are linked at offset 0x7000, right after the SFUOTA service manager.

 2. All the provided projects are built with size optimization in every build configuration (on STM32CubeIDE the Debug configuration is set to size optimization as well, instead of the default no-optimization level). This is required because SFUOTA_Client and SFUOTA_ServerYmodem must fit in the 0x7000 bytes reserved to the SFUOTA service manager: with a lower optimization level the image exceeds that limit and overwrites the beginning of the application area.

 3. Whatever toolchain is used, the application binary is generated in the Binary folder of the project, so that the image to be transferred with Ymodem is always available.

:::
:::
