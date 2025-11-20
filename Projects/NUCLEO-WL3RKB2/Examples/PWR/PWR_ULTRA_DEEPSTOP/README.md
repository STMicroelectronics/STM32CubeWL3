### <b>PWR_Ultra_Deepstop Example Description</b>

How to enter the UltraDeepStop mode and wake up from this mode by using an external reset or the WKUP pin.

In the associated software, the system clock is set to 64 MHz.

The SysTick is programmed to generate an interrupt each 1 ms and in the SysTick
interrupt handler, LD1 is on in order to indicate whether the MCU is in UltraDeepStop or Run mode.

When a falling edge is detected on the EXTI line, an interrupt is generated and the system wakes up
the program then checks and clears the UltraDeepStop flag.

After clearing the UltraDeepStop flag, the software configures the UltraDeepStop 
mode to enable the I/O wakeup pin PWR_SDWN_WUEN_WUEN, then clears the corresponding 
flag indicating a wakeup event was received from PWR_SDWN_WUF_WUF. 
Finally, the system re-enters UltraDeepStop mode, causing LD1 to turn off.

After waking up from UltraDeepStop mode, program execution restarts as it does after a RESET, 
with LD1 turning back on. Meanwhile, LD2 toggles at different speeds to identify the wakeup source.

Two leds LD1, LD2 and LD3 are used to monitor the system state as follows:

 - LD1 ON: system in Run mode
 - LD1 off : system in UltraDeepStop mode
 - LD2 blinking slow : wakeup from B1
 - LD2 blinking medium : wakeup from B2
 - LD2 blinking fast :wakeup from B3
 - LD3 ON: configuration failed (system will go to an infinite loop)

#### <b>Notes</b>

1-Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds) based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower) than the peripheral interrupt. Otherwise the caller ISR process will be blocked. To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

2-The application needs to ensure that the SysTick time base is always set to 1 millisecond to have correct HAL operation.

### <b>Keywords</b>

Power, PWR, UltraDeepStop mode, Interrupt, Wakeup, Low Power, External reset

### <b>Directory contents</b>

  - PWR/PWR_ULTRA_DEEPSTOP/Inc/stm32wl3r_nucleo_conf.h   BSP configuration file
  - PWR/PWR_ULTRA_DEEPSTOP/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - PWR/PWR_ULTRA_DEEPSTOP/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - PWR/PWR_ULTRA_DEEPSTOP/Inc/main.h                    Header file for main.c
  - PWR/PWR_ULTRA_DEEPSTOP/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - PWR/PWR_ULTRA_DEEPSTOP/Src/stm32wl3x_it.c            Interrupt handlers
  - PWR/PWR_ULTRA_DEEPSTOP/Src/main.c                    Main program
  - PWR/PWR_ULTRA_DEEPSTOP/Src/stm32wl3x_hal_msp.c       HAL MSP module

### <b>Hardware and Software environment</b> 

  - This example runs on STM32WL3RKBVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL3Rx
    board and can be easily tailored to any other supported device
    and development board.

  - NUCLEO-WL3Rx Set-up
    - Use LD1, LD2 and LD3 connected respectively to PB7, PB15 and PB2 pins
    - USER push-buttons connected to pins PA0 (B1_PIN), PA7 (B2_PIN), PB0 (B3_PIN)
    - WakeUp Pin PWR_SDWN_WUEN_WUEN connected to PB0, PA0, PA7, PA8, PA9, PA11,

### <b>How to use it ?</b> 

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
 - If the debugger connection is lost, perform a mass erase using STM32CubeProgrammer to reset the memory and allow the debugger to communicate again.


