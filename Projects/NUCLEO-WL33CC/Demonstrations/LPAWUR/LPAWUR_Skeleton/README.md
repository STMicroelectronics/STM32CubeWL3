::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>LPAWUR_Skeleton Example Description</b>

Code demonstrating the basic project structure template with initialization framework to be used for building a LPAWUR radio demonstration application.

### <b>Keywords</b>

LPAWUR, radio, wakeup

### <b>Directory contents</b>

  - LPAWUR/LPAWUR_Skeleton/Core/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - LPAWUR/LPAWUR_Skeleton/Core/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - LPAWUR/LPAWUR_Skeleton/Core/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - LPAWUR/LPAWUR_Skeleton/Core/Inc/main.h                    Header file for main.c
  - LPAWUR/LPAWUR_Skeleton/Core/Inc/stm32_assert.h            Assert description file
  - LPAWUR/LPAWUR_Skeleton/Core/Inc/app_conf.h                STM32WPAN middleware configuration file
  - LPAWUR/LPAWUR_Skeleton/Core/Inc/app_entry.h               Application interface
  - LPAWUR/LPAWUR_Skeleton/Core/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - LPAWUR/LPAWUR_Skeleton/Core/Src/stm32wl3x_it.c            Interrupt handlers
  - LPAWUR/LPAWUR_Skeleton/Core/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - LPAWUR/LPAWUR_Skeleton/Core/Src/main.c                    Main program
  - LPAWUR/LPAWUR_Skeleton/Core/Src/app_entry.c               Application entry point file

### <b>Hardware and Software environment</b>

  - This example runs on NUCLEO-WL33CCx application board.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CCx application board and can be easily tailored to any other supported device and development board.   

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open the project with your preferred toolchain
 - Customize the project for implementing the user specific demonstration scenario using the LPAWUR proprietary driver
 - Rebuild all files and flash the board with the executable file 

:::
:::