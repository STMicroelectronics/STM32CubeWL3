### <b>PWR_Shutdown Example Description</b>

How to enter the Shutdown  mode and wake up from this mode by using an external reset or one of several wakeup pins..

In the associated software, the system clock is set to 64 MHz, an EXTI line
is connected to the pin B1 and configured to generate an
interrupt on falling edge.

The SysTick is programmed to generate an interrupt every 1 ms. In the SysTick interrupt handler, LD1 is on to indicate that the MCU is in Run mode.
The blinking frequency of LD2 changes depending on which user button (B1, B2, or B3) was last pressed.

When a falling edge is detected on the EXTI line, an interrupt is generated and the system wakes up the program then checks and clears the Shutdown flag.

After clearing the Shutdown flag, the software configures the Shutdown mode to enable the I/O wakeup pin, 
then clears the corresponding flag indicating a wakeup event was received. Finally, the system re-enters Shutdown mode, causing LD1 to turn off.

After waking up from Shutdown mode, program execution restarts as it does after a RESET, with LD1 turning back on. Meanwhile, LD2 toggles at different speeds to identify the wakeup source.


Startup and LED Behavior:

After a reset, the default behavior is that LD1 is on.

  - LD1 ON: system in Run mode
  - LD1 off : system in Shutdown mode
  - LD2 blinking slow : wakeup from B1
  - LD2 blinking medium : wakeup from B2
  - LD2 blinking fast :wakeup from B3
  - LD3 ON: configuration failed (system will go to an infinite loop)


#### <b>Notes</b>

1-Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds) based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower) than the peripheral interrupt. Otherwise the caller ISR process will be blocked. To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

2-The application needs to ensure that the SysTick time base is always set to 1 millisecond to have correct HAL operation.

### <b>Keywords</b>

Power, PWR, Deepstop mode, Interrupt, Wakeup, Low Power, External reset

### <b>Directory contents</b>

  - PWR/PWR_Shutdown/Inc/stm32wl3x_conf.h         HAL Configuration file
  - PWR/PWR_Shutdown/Inc/stm32wl3x_it.h           Header for stm32wl3x_it.c
  - PWR/PWR_Shutdown/Inc/main.h                   Header file for main.c
  - PWR/PWR_Shutdown/Src/system_stm32wl3x.c       STM32WL3x system clock configuration file
  - PWR/PWR_Shutdown/Src/stm32wl3x_it.c           Interrupt handlers
  - PWR/PWR_Shutdown/Src/main.c                   Main program
  - PWR/PWR_Shutdown/Src/stm32wl3x_hal_msp.c      HAL MSP module

### <b>Hardware and Software environment</b> 

  - This example runs on STM32WL3RKBVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL3RKB1
    board and can be easily tailored to any other supported device
    and development board.

  - NUCLEO-WL3RKB1 Set-up
    - Use LD1, LD2 and LD3 connected respectively to PB7, PB15 and PB2 pins
    - USER push-buttons connected to pins PA0 (B1_PIN), PA7 (B2_PIN), PB0 (B3_PIN)
    - WakeUp Pins connected to PB0, PA0, PA7, PA8, PA9, PA11

### <b>How to use it ?</b> 

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
