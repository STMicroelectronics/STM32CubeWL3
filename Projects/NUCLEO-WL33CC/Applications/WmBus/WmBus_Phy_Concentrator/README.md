::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>WmBus_Phy_Concentrator Application Description</b>

This application allows to evaluate the STM32WL33 WmBus Phy device radio capabilities as Concentrator.
The application, after initialization, listen for WmBus packets, according to the configuration specified below and prints some relevant information or the error source of the received packet.

### <b>Keywords</b>

WmBus, Skeleton, MRSUBG

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
  - WmBus/App/app_wmbus.h                   Header of application of the WmBus Phy Middleware
  - WmBus/App/app_wmbus.c                   Application of the WmBus Phy Middleware

### <b>Hardware and Software environment</b>

  - This example runs on NUCLEO-WL33CCx application board.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CCx application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

### <b>Static configuration</b>

The WmBus Phy is configured as follow:
 - WmBus Mode: C-mode
 - WmBus Format: Format A
 - WmBus Direction: Meter to Other
These settings can be found in file main.c.

Furthermore, this project defines the following macros:

| Compilation   switch              | Purpose                                                                                                            |
|-----------------------------------|--------------------------------------------------------------------------------------------------------------------|
| WMBUS_CRC_IN_HAL                  | Activate CRC   computing/checking for TX/RX in HAL                                                                 |
| WMBUS_NO_BLOCKING_HAL             | Enable   non blocking calls for TX and RX      Radio Event bitmap updated by HAL                                   |
| T1C1_ACTIVATED_C1_MODE            | Activate   single protocol (Radio + packet) RX configuration for T1 + C1 gateway   reception only if C-mode is set |
| AUTOMATIC_FORMAT_DETECTION_C_MODE | Activate   dynamic frame format detection (A/B) in C-mode reception                                                |
| PREAMBLE_AND_SYNC_IRQ_ENABLE      | Enable   Radio event for Preamble detection and HW Sync detection                                                  |
| WMBUS_RX_PERFORMANCE_ENABLED      | Enable   RX performance optimizations                                                                              |
| WMBUS_ACTIVE_POWER_MODE_ENABLED   | Enable   active power mode to optimize RX, TX performances and power consumption                                   |
| WFI_ENABLE                        | Enable   WFI (RX application) – waiting for Radio events                                                           |

::: 
::: 