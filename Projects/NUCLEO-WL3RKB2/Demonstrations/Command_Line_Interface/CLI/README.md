::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>CLI Example Description</b>

The STM32WL3 CLI demo application allows to use the STM32WL3 WiSE-RadioExplorer PC application by simply connecting a STM32WL3 development kit to PC USB port.

Moreover a Virtual Com stream is open. Both the transmitter and the receiver will write their own buffer on video every time a transmission or a reception is performed.

### <b>Keywords</b>

MRSUBG, radio, CLI, WiSE-RadioExplorer

### <b>Directory contents</b>

  - Command_Line_Interface/CLI/Core/Inc/stm32wl3r_nucleo_conf.h    BSP configuration file
  - Command_Line_Interface/CLI/Core/Inc/stm32wl3x_hal_conf.h       HAL Configuration file
  - Command_Line_Interface/CLI/Core/Inc/stm32wl3x_it.h             Header for stm32wl3x_it.c
  - Command_Line_Interface/CLI/Core/Inc/main.h                     Header file for main.c
  - Command_Line_Interface/CLI/Core/Inc/stm32_assert.h             STM32 assert file
  - Command_Line_Interface/CLI/Core/Inc/stm32wl3x_cli_commands.h   Header file for stm32wl3x_cli_commands.c
  - Command_Line_Interface/CLI/Core/Inc/app_entry.h                Application interface
  - Command_Line_Interface/CLI/Core/Inc/timer_utils.h              Header file for timer_utils.c
  - Command_Line_Interface/CLI/Core/Inc/binary_iqsampling.h        Header file for binary_iqsampling.c
  - Command_Line_Interface/CLI/Core/Inc/binary_iqsampling_spi.h    Header file for binary_iqsampling_spi.c
  - Command_Line_Interface/CLI/Core/Src/system_stm32wl3x.c         STM32WL3x system clock configuration file
  - Command_Line_Interface/CLI/Core/Src/stm32wl3x_it.c             Interrupt handlers
  - Command_Line_Interface/CLI/Core/Src/stm32wl3x_hal_msp.c        HAL MSP module
  - Command_Line_Interface/CLI/Core/Src/main.c                     Main program
  - Command_Line_Interface/CLI/Core/Src/app_entry.c                Application entry point 
  - Command_Line_Interface/CLI/Core/Src/binary_iqsampling.c        Binary IQ sampling file
  - Command_Line_Interface/CLI/Core/Src/binary_iqsampling_spi.c    Binary IQ sampling file for SPI
  - Command_Line_Interface/CLI/Core/Src/stm32wl3x_cli_commands.c   CLI commands file
  - Command_Line_Interface/CLI/Core/Src/timer_utils.c              Timer utilities file
  - Command_Line_Interface/CLI/Core/Src/cli/utils.c                Utilities file
  - Command_Line_Interface/CLI/Core/Src/cli/serial_utils.c         Serial utilities file
  - Command_Line_Interface/CLI/Core/Src/cli/response.c             Display response file
  - Command_Line_Interface/CLI/Core/Src/cli/command-interpreter2.c Command interpreter file
  - Command_Line_Interface/CLI/Core/Inc/cli/utils.h                Header for utils.c
  - Command_Line_Interface/CLI/Core/Inc/cli/serial_utils.h         Header for serial_utils.c         
  - Command_Line_Interface/CLI/Core/Inc/cli/command-interpreter2.h Header for command-interpreter2.c
  - Command_Line_Interface/CLI/System/Interface/hw_mrsubg.h        Header for hw_mrsubg.c
  - Command_Line_Interface/CLI/System/Interface/hw_mrsubg.c        MRSUBG Manager functions file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3RKBVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL3RKB2 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example

::: 
::: 