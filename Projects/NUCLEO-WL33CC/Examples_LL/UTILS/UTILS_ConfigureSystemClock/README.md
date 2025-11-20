## <b>UTILS_ConfigureSystemClock Example Description</b>

Use of UTILS LL API to configure the system clock using RC64MPLL with HSE as source clock.

The user application just needs to configure the HSE and RC64MPLL parameters using STM32CubeMX and call the UTILS LL API.

Then the SystemClock_Config() function is used to configure the system
clock (SYSCLK) to run at 64MHz.

System starts with clock used after reset.
Then, a system clock switch is done to RC64MPLL with HSE as the clock source. Automatically, FLASH latency
is tuned according to system constraints described in the reference manual.

User can easily set their own RC64MPLL parameters in changing global variable used to store them.

A LD1 toggle of 1sec provides this information that system is well configured to requested frequency.  

Anyway, user can connect on oscilloscope to MCO pin (PA.5) to check requested frequency:
 
 - SYSCLK frequency with frequency value around 32MHz.

### <b>Keywords</b>

Utils, system, Clock, HSE, RC64MPLL, flash latency, SYSCLK, frequency Oscilloscope



### <b>Directory contents</b>

  - UTILS/UTILS_ConfigureSystemClock/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - UTILS/UTILS_ConfigureSystemClock/Inc/main.h                  Header for main.c module
  - UTILS/UTILS_ConfigureSystemClock/Inc/stm32_assert.h          Template file to include assert_failed function
  - UTILS/UTILS_ConfigureSystemClock/Src/stm32wl3x_it.c          Interrupt handlers
  - UTILS/UTILS_ConfigureSystemClock/Src/main.c                  Main program
  - UTILS/UTILS_ConfigureSystemClock/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
    
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

  - NUCLEO-WL33CC Set-up
    - Connect the MCO pin to an oscilloscope to monitor the different waveforms:
      - PA.5: Arduino connector D8 CN6 pin 1, Morpho connector CN4 pin 21 (MB1801)

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example


