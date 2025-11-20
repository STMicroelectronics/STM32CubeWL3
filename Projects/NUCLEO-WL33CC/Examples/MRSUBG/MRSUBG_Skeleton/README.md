::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_Skeleton Example Description</b>

Code demonstrating the basic project structure template with initialization framework to be used for building a sub-1 GHz radio example application.

### <b>Keywords</b>

MRSUBG, radio

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_Skeleton/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_Skeleton/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_Skeleton/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_Skeleton/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_Skeleton/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_Skeleton/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_Skeleton/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_Skeleton/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_Skeleton/Src/main.c                    Main program

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
    Connect the Nucleo Board to your PC with a USB cable type A to USB type C to ST-LINK connector (USB_STLINK). 

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open the projects with your favorite toolchain.
 - Customize the project for implementing the user specific example scenario using the sub-1 GHz radio proprietary driver.
 - Rebuild all files and flash the board with the executable file.

:::
:::