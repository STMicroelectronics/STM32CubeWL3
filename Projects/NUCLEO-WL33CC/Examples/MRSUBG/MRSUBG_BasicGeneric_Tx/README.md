::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_BasicGeneric_Tx Example Description</b>

This example explains how to configure one node as a transmitter and the other as a receiver
in order to perform a simple basic packet transmission.  
There are two devices: the device TX is configured as a transmitter and the device RX as a receiver. The program consists to transmit a fixed length sequence from TX and to control if RX has received them correctly.

So, the user should see:

- LED1 of TX at the end of each transmission.
- LED1 of RX every time the packet has been received.  

Moreover a Virtual Com stream is open. Both the transmitter and the receiver will write their own buffer on video every time a transmission or a reception is performed.

### <b>Keywords</b>

MRSUBG, radio, TX, RX

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_BasicGeneric_Tx/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_BasicGeneric_Tx/Src/main.c                    Main program


### <b>Hardware and Software environment</b>

  - This example runs on NUCLEO-WL33CC1 application board.
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::