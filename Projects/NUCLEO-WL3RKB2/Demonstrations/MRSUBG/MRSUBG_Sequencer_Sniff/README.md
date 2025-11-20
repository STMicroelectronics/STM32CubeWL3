::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_Sequencer_Sniff Example Description</b>

This example shows how to set the Sequencer to operate in sniff mode. This example demonstrates the receiver side and requires another device as a TX. 
Moreover, a Virtual Com stream is open. The receiver will write its own buffer on video every time a reception is performed.

### <b>Keywords</b>

MRSUBG, radio

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/stm32wl3r_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/app_conf.h                STM32WPAN middleware configuration file
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Inc/app_entry.h               Application interface
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Src/main.c                    Main program
  - MRSUBG/MRSUBG_Sequencer_Sniff/Core/Src/app_entry.c               Application entry point file

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