---
pagetitle: Readme for STM32CubeWL3R wM-Bus DataLink Concentrator
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>wMBus_DataLink_Concentrator Application Description</b>

This simple application implements a concentrator. It uses the wM-Bus middleware at the data link layer to manage bidirectional communication. 
After initialization, it continuously listens for wM-Bus frames based on the specified configuration, and prints relevant information, including any errors.

### <b>Keywords</b>

wM-Bus, wMBus, Skeleton, MRSUBG

### <b>Directory contents</b>

  - Core/Inc/app_conf.h                     Application configuration file
  - Core/Inc/main.h                         Header file for main.c
  - Core/Inc/stm32_assert.h                 STM32 assert file
  - Core/Inc/stm32wl3x_hal_conf.h           HAL Configuration file
  - Core/Inc/stm32wl3x_it.h                 Header for stm32wl3x_it.c
  - Core/Inc/stm32wl3r_nucleo_conf.h        BSP configuration file
  - Core/Inc/utilities_conf.h               Utilities configuration file
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
  - wMBus/App/app_wMBus.h                   Header of application of the wM-Bus Middleware
  - wMBus/App/app_wMBus.c                   Application of the wM-Bus Middleware

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3RKBVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL3RKB1 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

### <b>Static configuration</b>

The wM-Bus MW is configured as follow:

 - wM-Bus Mode: C-mode
 - wM-Bus Format: Format A
 - wM-Bus Direction: Meter to Other

These settings can be found in file app_wMBus.c.

Furthermore, this project defines the following macros:

| Compilation   switch              | Purpose                                                                                                            |
|-----------------------------------|--------------------------------------------------------------------------------------------------------------------|
| PREAMBLE_AND_SYNC_IRQ_ENABLE      | Enable   Radio event for Preamble detection and HW Sync detection                                                  |
| AUTOMATIC_FORMAT_DETECTION_C_MODE | Activate   dynamic frame format detection (A/B) in C-mode reception                                                |
| WMBUS_RX_PERFORMANCE_ENABLED      | Enable   RX performance optimizations                                                                              |
| PROCESS_FRAME                     | Enable   frame processing to populate RX Frame                                                                     |

::: 
::: 