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
The OTA protocol handling and flash programming flow are implemented in radio_ota.c/.h.
When the download is completed and validated, the client jumps to the newly programmed application.

At runtime, OTA transfer is managed by OTA_Init()/OTA_Tick() and radio IRQ callbacks.
The client receives OTA chunks, assembles FLASH pages, and writes them at APP_WITH_OTA_SERVICE_ADDRESS.
Once the new application is running, PB2 is used to request a jump back to the OTA client mode for a new download session.
The firmware image and flash layout are configured with dedicated offsets so that OTA service manager and target application areas do not overlap.

### <b>Keywords</b>

Connectivity, MRSUBG, OTA, Sub-GHz radio

### <b>Directory contents</b>

  - Core/Inc/app_conf.h                     Application configuration file
  - Core/Inc/app_entry.h                    Interface header for application entry
  - Core/Inc/main.h                         Header file for main.c
  - Core/Inc/radio_ota.h                    OTA client protocol and flash layout definitions
  - Core/Inc/stm32_assert.h                 STM32 assert file
  - Core/Inc/stm32wl3x_hal_conf.h           HAL Configuration file
  - Core/Inc/stm32wl3x_it.h                 Header for stm32wl3x_it.c
  - Core/Inc/stm32wl3x_nucleo_conf.h        BSP configuration file
  - Core/Inc/utilities_conf.h               Utilities configuration file
  - Core/Src/app_entry.c                    Application entry implementation
  - Core/Src/main.c                         Main program
  - Core/Src/radio_ota.c                    OTA client protocol and flash programming implementation
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

  - This demonstration runs on STM32WL33CCVx devices
  - This demonstration has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board

### <b>How to use it?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain (IAR EWARM project is provided)
 - Program SFUOTA_ServerYmodem first, so OTA data can be served to clients
 - Rebuild all files and load your image into target memory
 - Run the demonstration and follow OTA host procedure to transfer the image

### <b>Static configuration</b>

The OTA client uses MRSUBG with packet exchange managed in radio_ota.c.

Key configuration points include:

 - OTA protocol headers and sequence handling
 - Flash target layout, tags, and page programming policy
 - OTA service manager jump control through retained RAM variable

These settings can be found in files app_entry.c and radio_ota.c/.h.

### <b>Notes</b>

 1. This demonstration currently provides IAR EWARM project files.

:::
:::
