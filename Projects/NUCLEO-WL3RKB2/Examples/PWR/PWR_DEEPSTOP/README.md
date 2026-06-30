## <b>PWR_Deepstop Example Description</b>

How to enter the Deepstop mode and wake up from this mode by using an external reset or the WKUP pin.

In the associated software, the system clock is set to 0, an EXTI line
is connected to the pin B1 and configured to generate an
interrupt on falling edge.

The SysTick is programmed to generate an interrupt each 1 ms and in the SysTick
interrupt handler, LD1 is toggled in order to indicate whether the MCU is in Deepstop or Run mode.

After startup, the example remains in Run mode for 5 seconds, then enters Deepstop mode.
When the user presses B1, the wakeup event is generated and the device exits Deepstop mode.
After wake-up, the program restarts and the same sequence is repeated continuously.

When a falling edge is detected on the EXTI line, an interrupt is generated and the system wakes up
the program then checks and clears the Deepstop flag.

After clearing the Deepstop flag, the software enables wake-up pin PWR_WAKEUP_PIN0 connected to PC.05, then
the corresponding flag indicating that a wakeup event was received from the PWR_WAKEUP_PIN0 is cleared.
Finally, the system enters again Deepstop mode causing LD1 to stop toggling.

After wake-up from Deepstop mode, program execution restarts in the same way as after
a RESET and LD1 restarts toggling.



Two leds LD1 and LD3 are used to monitor the system state as follows:

 - LD3 ON: configuration failed (system will go to an infinite loop)
 - LD1 toggling: system in Run mode
 - LD1 off : system in Deepstop mode



#### <b>Notes</b>

1-Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds) based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower) than the peripheral interrupt. Otherwise the caller ISR process will be blocked. To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

2-The application needs to ensure that the SysTick time base is always set to 1 millisecond to have correct HAL operation.

### <b>Keywords</b>

Power, PWR, Deepstop mode, Interrupt, Wakeup, Low Power, External reset

### <b>Directory contents</b>

  - PWR/PWR_DEEPSTOP/Inc/stm32wl3x_conf.h         HAL Configuration file
  - PWR/PWR_DEEPSTOP/Inc/stm32wl3x_it.h           Header for stm32wl3x_it.c
  - PWR/PWR_DEEPSTOP/Inc/main.h                         Header file for main.c
  - PWR/PWR_DEEPSTOP/Src/system_stm32wl3x.c       STM32WL3x system clock configuration file
  - PWR/PWR_DEEPSTOP/Src/stm32wl3x_it.c           Interrupt handlers
  - PWR/PWR_DEEPSTOP/Src/main.c                         Main program
  - PWR/PWR_DEEPSTOP/Src/stm32wl3x_hal_msp.c      HAL MSP module

### <b>Hardware and Software environment</b> 

  - This example runs on STM32WL3RKBVx devices

  - This example has been tested with STMicroelectronics NUCLEO-WL3RKB2
    board and can be easily tailored to any other supported device
    and development board.

  - NUCLEO-WL3RKB2 Set-up
    - Use LD1 and LD3 connected respectively to PB.07 and PB.02 pins
    - WakeUp Pin PWR_WAKEUP_PIN0 connected to B1 pin
    - USER push-button connected to pin PA.00 (B1_PIN)
    - WakeUp Pin PWR_WAKEUP_PIN0 connected to 

### <b>How to use it ?</b> 

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
