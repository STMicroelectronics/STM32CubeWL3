::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_WMBusSTD_Tx Example Description</b>

This is a demo implementation of the standard WMBUS protocol. It takes in account the physical and data link layers defined by the standard.   The example shows how to configure the packets in order to transmit or receive MBUS packets (format A only).

The data-link layer all the fields defined by the protocol are inserted into the payload of the packets and the CRC blocks computation is managed by firmware.   
Moreover, on the RX side the algorithm uses the L-field of the wM-BUS packet to correctly stop the reception making use of the Databuffer threshold IRQs.

The radio configuration according to the standards has to be totally managed by the user.

A Virtual Com stream is open. Both the transmitter and the receiver will write their encrypted and decrypted buffers on video every time a Tx or an Rx is performed.

### <b>Keywords</b>

MRSUBG, radio

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_WMBusSTD_Tx/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_WMBusSTD/Inc/stm32_assert.h               Assert description file
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_WMBusSTD_Tx/Src/main.c                    Main program

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CCx application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::