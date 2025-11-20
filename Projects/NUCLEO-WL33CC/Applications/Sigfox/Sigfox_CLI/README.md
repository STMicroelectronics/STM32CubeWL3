::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>Sigfox_CLI Application Description</b>

This application demonstrates how to evaluate the STM32WL3x Sigfox device radio capabilities.  
This software is meant to be used in conjunction with the Sigfox RSA environment.  
For further details, please refer to https://support.sigfox.com/docs/radio-signal-analyser-user-guide

Moreover a Virtual Com stream is open. Both the transmitter and the receiver will write their own buffer on video every time a command is performed.

### <b>Keywords</b>

Sigfox, Skeleton, MRSUBG, CLI

### <b>Directory contents</b>

  - Sigfox/Sigfox_Skeleton/Core/Inc/stm32wl3x_nucleo_conf.h        BSP configuration file
  - Sigfox/Sigfox_Skeleton/Core/Inc/stm32wl3x_hal_conf.h           HAL Configuration file
  - Sigfox/Sigfox_Skeleton/Core/Inc/stm32wl3x_it.h                 Header for stm32wl3x_it.c
  - Sigfox/Sigfox_Skeleton/Core/Inc/main.h                         Header file for main.c
  - Sigfox/Sigfox_Skeleton/Core/Inc/stm32_assert.h                 STM32 assert file
  - Sigfox/Sigfox_Skeleton/Core/Inc/apps_conf.h                    Application configuration file
  - Sigfox/Sigfox_Skeleton/Core/Inc/utilities_conf.h               Utilities configuration file
  - Sigfox/Sigfox_Skeleton/Core/Inc/oob_adc.h                      Header file for oob_adc.c 
  - Sigfox/Sigfox_Skeleton/Core/Src/system_stm32wl3x.c             STM32WL3x system clock configuration file
  - Sigfox/Sigfox_Skeleton/Core/Src/stm32wl3x_it.c                 Interrupt handlers
  - Sigfox/Sigfox_Skeleton/Core/Src/stm32wl3x_hal_msp.c            HAL MSP module
  - Sigfox/Sigfox_Skeleton/Core/Src/main.c                         Main program
  - Sigfox/Sigfox_Skeleton/Core/Inc/oob_adc.c                      ADC configuration file
  - Sigfox/Sigfox_Skeleton/Core/Inc/cli/command-interpreter2.h     Header file for command-interpreter2.c
  - Sigfox/Sigfox_Skeleton/Core/Inc/cli/serial_utils.h             Header file for serial_utils.c
  - Sigfox/Sigfox_Skeleton/Core/Src/cli/command-interpreter2.c     Commands configuration file
  - Sigfox/Sigfox_Skeleton/Core/Src/cli/serial_utils.c             Serial utilities file
  - Sigfox/Sigfox_Skeleton/Core/Src/cli/response.c                 Response configuration file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_credentials.h      Sigfox credentials management file 
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_databuffers.c      RAM data buffers management file 
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_databuffers.h      Header for wl3sfx_databuffers.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_fem.c   	       Front-end modules configuration file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_fem.h              Header for wl3sfx_fem.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_flash_utils.c      Read and write from flash file 
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_flash_utils.h      Header for wl3sfx_flash_utils.h
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_it.c               Interrupt callbacks management file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_it.h               Header for wl3sfx_it.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_log.c   	       Internal debbug messages file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_log.h              Header for wl3sfx_log.c 
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_mcu_api.c          Application configuration file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_nvm.c   	       Nonvolatile memory contents file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_nvm.h              Header for wl3sfx_nvm.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_nvm_boarddata.c    Key-value store file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_nvm_boarddata.h    Header for wl3sfx_nvm_boarddata.c 
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_nvm_records.c      Stores Sigfox NVM records file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_nvm_records.h      Header for wl3sfx_nvm_records.c 
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_oob.c              Utilities for retrieving measurements file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_oob.h              Header for wl3sfx_oob.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_pn9tx.c            Interrupt-based generator for a pseudorandom sequence file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_pn9tx.h            Header for wl3sfx_pn9tx.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_rf_api.c           Application configuration file for STM32WPAN Middleware
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_rf_state.c         State machine's state management file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_rf_state.h         Header for wl3sfx_rf_state.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_timer.c            RTC timers control file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_timer.h            Header for wl3sfx_timer.c
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_txsymbols.c        Generation of DBPSK symbols management file
  - Sigfox/Sigfox_Skeleton/Sigfox/Target/wl3sfx_txsymbols.h        Header for wl3sfx_txsymbols.c
  - Sigfox/Sigfox_Skeleton/Sigfox/App/app_sigfox.c                 Application of the Sigfox Middleware file
  - Sigfox/Sigfox_Skeleton/Sigfox/App/app_sigfox.h                 Header for app_sigfox.c
  - Sigfox/Sigfox_Skeleton/Sigfox/App/cli_commands.c               CLI commands file
  - Sigfox/Sigfox_Skeleton/Sigfox/App/cli_commands.h               Header for cli_commands.c
  - Sigfox/Sigfox_Skeleton/Sigfox/App/ST_Sigfox.c                  Application of the Sigfox Middleware file
  - Sigfox/Sigfox_Skeleton/Sigfox/App/ST_Sigfox.h                  Header for ST_Sigfox.c
  - Sigfox/Sigfox_Skeleton/Sigfox/App/ST_Sigfox_Types.h            Sigfox type definitions file
  - Sigfox/Sigfox_Skeleton/System/Interfaces/stm32_lpm_if.c        Low power mode configuration file
  - Sigfox/Sigfox_Skeleton/System/Interfaces/stm32_lpm_if.h        Header for stm32_lpm_if.c
  - Sigfox/Sigfox_Skeleton/System/Modules/asm.h                    ASM Compiler-dependent macros file
  - Sigfox/Sigfox_Skeleton/System/Modules/osal.c                   OS abstraction layer definition file
  - Sigfox/Sigfox_Skeleton/System/Modules/osal.h                   Header for osal.c
  - Sigfox/Sigfox_Skeleton/System/Startup/cpu_context_switch.s     Context restore file
  - Sigfox/Sigfox_Skeleton/System/Startup/device_context_switch.c  STM32WL3 context switch file
  - Sigfox/Sigfox_Skeleton/System/Startup/device_context_switch.h  Header for device_context_switch.c

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

:::
::: 