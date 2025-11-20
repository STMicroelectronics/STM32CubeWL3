::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>LPAWUR_Skeleton Example Description</b>

Code demonstrating the basic project structure template with initialization framework to be used for building a LPAWUR radio example application.

### <b>Keywords</b>

LPAWUR, radio

### <b>Directory contents</b>

  - LPAWUR/LPAWUR_Skeleton/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - LPAWUR/LPAWUR_Skeleton/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - LPAWUR/LPAWUR_Skeleton/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - LPAWUR/LPAWUR_Skeleton/Inc/main.h                    Header file for main.c
  - LPAWUR/LPAWUR_Skeleton/Inc/stm32_assert.h            Assert description file
  - LPAWUR/LPAWUR_Skeleton/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - LPAWUR/LPAWUR_Skeleton/Src/stm32wl3x_it.c            Interrupt handlers
  - LPAWUR/LPAWUR_Skeleton/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - LPAWUR/LPAWUR_Skeleton/Src/main.c                    Main program


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
    Connect the Nucleo Board to your PC with a USB cable type A to USB type C to ST-LINK connector (USB_STLINK). 

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open the projects with your favorite toolchain.
 - Customize the project for implementing the user specific example scenario using the LPAWUR radio proprietary driver.
 - Rebuild all files and flash the board with the executable file.

:::
:::