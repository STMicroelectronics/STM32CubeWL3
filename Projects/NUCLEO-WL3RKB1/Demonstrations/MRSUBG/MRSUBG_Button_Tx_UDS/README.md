::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_Button_Tx_UDS Example Description</b>

This example shows how to set the SoC in Ultra-DeepStop using by using the Low-Power Manager (LPM) and configure the MRSUBG to wakeup the SOC pressing PB2. Execution will then restart and a frame will be sent.
This example demonstrates the transmitter side and requires another device as a receiver. 

The receiver example is located under NUCLEO-WL3RKB1\Demonstrations\MRSUBG\MRSUBG_Sequencer_Sniff.
Moreover, a Virtual Com stream is open. Both the transmitter and the receiver will write their own buffer on video every time a transmission or a reception is performed.

### <b>Keywords</b>

MRSUBG, radio, Ultra-DeepStop

### <b>Directory contents</b>

      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/stm32wl3r_nucleo_conf.h   BSP configuration file
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/main.h                    Header file for main.c
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/stm32_assert.h            Assert description file
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/app_conf.h                STM32WPAN middleware configuration file
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Inc/app_entry.h               Application interface
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Src/stm32wl3x_it.c            Interrupt handlers
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Src/stm32wl3x_hal_msp.c       HAL MSP module
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Src/main.c                    Main program
      - MRSUBG/MRSUBG_Button_Tx_UDS/Core/Src/app_entry.c               Application entry point file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3Rx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL3RKB1 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example
:::
:::