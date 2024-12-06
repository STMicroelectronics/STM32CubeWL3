## <b>GPIO_IOToggle Example Description</b>

How to configure and use GPIOs through the HAL API.

PA.14 and PB.04 IOs (configured in output pushpull mode) toggle in a forever loop.
On NUCLEO-WL33CC board these IOs are connected to LD1 and LD2.

In this example, HCLK is configured at 64 MHz.

#### <b>Notes</b>

 1. Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
    based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
    a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
    than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
    To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

 2. The example needs to ensure that the SysTick time base is always set to 1 millisecond
    to have correct HAL operation.

### <b>Keywords</b>

System, GPIO, Input, Output, Alternate function, Push-pull, Toggle

### <b>Directory contents</b>

  - GPIO/GPIO_IOToggle/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - GPIO/GPIO_IOToggle/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - GPIO/GPIO_IOToggle/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - GPIO/GPIO_IOToggle/Inc/main.h                  Header for main.c module  
  - GPIO/GPIO_IOToggle/Src/stm32wl3x_it.c          Interrupt handlers
  - GPIO/GPIO_IOToggle/Src/stm32wl3x_hal_msp.c     HAL MSP file
  - GPIO/GPIO_IOToggle/Src/main.c                  Main program
  - GPIO/GPIO_IOToggle/Src/system_stm32wl3x.c      STM32WL3x system source file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

