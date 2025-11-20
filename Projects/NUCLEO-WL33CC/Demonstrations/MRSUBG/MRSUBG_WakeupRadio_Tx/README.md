::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_WakeupRadio_Tx Example Description</b>

This example explains shows how to set the SoC in deepstop and configure the MRSUBG to wakeup the SOC pressing PB2 to send a frame.  
This example demonstrates the transmitter side and requires another device as a LPAWUR receiver. The receiver example is located under the NUCLEO-WL33CC\Demonstrations\LPAWUR\LPAWUR_WakeupRadio_Rx folder.
Moreover, a Virtual Com stream is open. Both the transmitter and the receiver will write their own buffer on video every time a transmission or a reception is performed.

### <b>Keywords</b>

MRSUBG, LPAWUR, deepstop, wakeup, radio

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/app_conf.h                STM32WPAN middleware configuration file
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Inc/app_entry.h               Application interface
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Src/main.c                    Main program
  - MRSUBG/MRSUBG_WakeupRadio_Tx/Core/Src/app_entry.c               Application entry point file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CCx application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::