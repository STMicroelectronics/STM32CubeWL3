## <b>GPIO_EXTI Example Description</b>

How to configure external interrupt lines.

Then the *SystemClock_Config()* function is used to configure the system clock (SYSCLK) to run at 64 MHz.

In this example, one EXTI line (B1_PIN) is configured to generate
an interrupt on each falling edge.
In the interrupt routine a led connected to a specific GPIO pin is toggled.

In this example:

    - B1_PIN is connected to PA.00 pin
    - when falling edge is detected on B1_PIN by pressing USER push-button, LD1 toggles once

On NUCLEO-WL33CC:

    - B1_PIN is connected to USER push-button

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

System, GPIO, EXTI, Output, Alternate function, Push-pull, Toggle

### <b>Directory contents</b>

  - GPIO/GPIO_EXTI/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - GPIO/GPIO_EXTI/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - GPIO/GPIO_EXTI/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - GPIO/GPIO_EXTI/Inc/main.h                  Header for main.c module  
  - GPIO/GPIO_EXTI/Src/stm32wl3x_it.c          Interrupt handlers
  - GPIO/GPIO_EXTI/Src/stm32wl3x_hal_msp.c     HAL MSP file
  - GPIO/GPIO_EXTI/Src/main.c                  Main program
  - GPIO/GPIO_EXTI/Src/system_stm32wl3x.c      STM32WL3x system source file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example


