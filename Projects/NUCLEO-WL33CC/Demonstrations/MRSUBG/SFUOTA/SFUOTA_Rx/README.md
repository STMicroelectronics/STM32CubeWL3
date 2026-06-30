---
pagetitle: Readme for STM32CubeWL3 SFUOTA Rx Demonstration
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>SFUOTA_Rx Demonstration Description</b>

How to configure a simple RX application with over-the-air (OTA) upgrade capability.

This code runs an RX scenario where the payload of each received frame is printed on the terminal.
Sleep management is integrated to conserve power.
RX data and RF error events are handled through IRQ flags and processed in the main loop.
PB2 is used to request a jump to OTA reset manager mode through a software reset sequence.
The firmware image is built with the correct offset to ensure it is compatible with the OTA client, preventing overwriting of the OTA client area during the firmware upgrade process.

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
 - Program SFUOTA_Client before this image if OTA return path via PB2 is required
 - Rebuild all files and load your image into target memory
 - Run the demonstration

### <b>Static configuration</b>

Key runtime configuration is defined in app_entry.c, including:

 - RX packet buffer length and IRQ-driven reception flow
 - OTA reset manager jump trigger on PB2

### <b>Notes</b>

 1. This demonstration currently provides IAR EWARM project files.

:::
:::