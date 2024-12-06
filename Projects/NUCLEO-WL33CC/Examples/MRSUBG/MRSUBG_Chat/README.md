::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_Chat Example Description</b>

This example explains how to configure a node both as a transmitter and as a receiver in order to perform a simple basic packet transmission.  
There are two devices: at the beginning each device is in RX state. Open a terminal and write some text and press enter. At this point the device will commute to TX  transmit the message and then go again in RX.   
Moreover, each device will write its own buffer on video every time a Tx or an Rx is performed.

### <b>Keywords</b>

MRSUBG, radio

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_Chat/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_Chat/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_Chat/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_Chat/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_Chat/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_Chat/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_Chat/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_Chat/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_Chat/Src/main.c                    Main program

### <b>Hardware and Software environment</b>

  - This example runs on NUCLEO-WL33CC1 application board.
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain and load the workspace
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::