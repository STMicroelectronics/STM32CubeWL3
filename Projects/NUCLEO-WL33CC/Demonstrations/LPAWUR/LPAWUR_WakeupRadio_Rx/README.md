::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>LPAWUR_WakeupRadio_Rx Example Description</b>

This example shows how to set the SoC in deepstop and configure the LPAWUR to wakeup the SOC when a frame arrives and it is correctly receveid. This example demonstrates the receiver side and requires another device as a transmitter. The transmitter example is located under the NUCLEO-WL33CC1\Demonstrations\MRSUBG\MRSUBG_WakeupRadio_Tx folder.
Moreover, a Virtual Com stream is open. Both the transmitter and the receiver will write their own buffer on video every time a transmission or a reception is performed.

### <b>Keywords</b>

LPAWUR, radio

### <b>Directory contents</b>

  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/stm32wl3x_hal_conf.h      HAL configuration file
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/main.h                    Header file for main.c
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/stm32_assert.h            Assert description file
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/app_conf.h                STM32WPAN middleware configuration file
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Inc/app_entry.h               Application interface
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Src/stm32wl3x_it.c            Interrupt handlers
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Src/main.c                    Main program
  - LPAWUR/LPAWUR_WakeupRadio_Rx/Core/Src/app_entry.c               Application entry point file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - For the receiver part: since the RF path of LPAWUR is not connected to the SMA connector, to run this example is needed to solder a 0 Ohm resistor on C3. The matching network is assembled and is tuned for the band 826-958 MHz.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CCx application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain and import the .c files in your workspace
 - Rebuild all files and load your image into target memory
 - Run the example
:::
:::