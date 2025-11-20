---
pagetitle: Readme for STM32CubeWL3 wM-Bus Phy Meter
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="../../../../../_htmresc/favicon.png"/>
---

::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>wMBus_Phy_Meter Application Description</b>

This simple application implements a meter. It uses the wM-Bus middleware at the physical layer to manage unidirectional communication. 
After initialization, it continuously transmits sample wM-Bus SND-NR frames based on the specified configuration, while printing relevant information.

### <b>Keywords</b>

wM-Bus, wMBus, Skeleton, MRSUBG

### <b>Directory contents</b>

  - Core/Inc/app_conf.h                     Application configuration file
  - Core/Inc/main.h                         Header file for main.c
  - Core/Inc/stm32_assert.h                 STM32 assert file
  - Core/Inc/stm32wl3x_hal_conf.h           HAL Configuration file
  - Core/Inc/stm32wl3x_it.h                 Header for stm32wl3x_it.c
  - Core/Inc/stm32wl3x_nucleo_conf.h        BSP configuration file
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
  - wMBus/App/app_wMBus.h                   Header of application of the wM-Bus Phy Middleware
  - wMBus/App/app_wMBus.c                   Application of the wM-Bus Phy Middleware

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CCx application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

### <b>Static configuration</b>

The wM-Bus Phy MW is configured as follow:

 - wM-Bus Mode: C-mode
 - wM-Bus Format: Format A
 - wM-Bus Direction: Meter to Other

These settings can be found in file app_wMBus.c.

Furthermore, this project defines the following macros:

| Compilation   switch  | Purpose                                                                          |
|-----------------------|----------------------------------------------------------------------------------|
| SENSE_RSSI            | Prints   the RSSI                                                                |

::: 
::: 