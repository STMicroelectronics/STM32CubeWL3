::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_802_15_4_Rx Example Description</b>

This is a demo implementation of the standard 802.15.4 protocol. It takes in account the physical defined by the standard. The example shows how to configure the packets in order to transmit or receive 802.15.4 packets.

A Virtual COM stream is open. Both the transmitter and the receiver will write their buffers on video every time a Tx or an Rx is performed.

### <b>Keywords</b>

MRSUBG, radio, TX, RX

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_802_15_4_Rx/Inc/stm32wl3r_nucleo_conf.h       BSP configuration file
  - MRSUBG/MRSUBG_802_15_4_Rx/Inc/stm32wl3x_hal_conf.h          HAL Configuration file
  - MRSUBG/MRSUBG_802_15_4_Rx/Inc/stm32wl3x_it.h                Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_802_15_4_Rx/Inc/main.h                        Header file for main.c
  - MRSUBG/MRSUBG_802_15_4_Rx/Inc/stm32_assert.h                Assert description file
  - MRSUBG/MRSUBG_802_15_4_Rx/Src/system_stm32wl3x.c            STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_802_15_4_Rx/Src/stm32wl3x_it.c                Interrupt handlers
  - MRSUBG/MRSUBG_802_15_4_Rx/Src/stm32wl3x_hal_msp.c           HAL MSP module
  - MRSUBG/MRSUBG_802_15_4_Rx/Src/main.c                        Main program

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3RKBVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL3Rx application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::