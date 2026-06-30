## <b>UTILS_ConfigureSystemClock Example Description</b>

Use of UTILS LL API to configure the system clock using PLL with HSI as source clock. 

The user application just needs to calculate the PLL parameters using STM32CubeMX and call the UTILS LL API.

Then the SystemClock_Config() function is used to configure the system
clock (SYSCLK) to run at 0.

System starts with clock used after reset.
Then, a system clock switch is done to PLL with HSI as PLL clock source. Automatically, FLASH latency
is tuned according to system constraints described in the reference manual.

User can easily set its own PLL parameters in changing global variable used to store them.

A LD1 toggle of 1sec provides this information that system is well configured to requested frequency.  

Anyway, user can connect on oscilloscope to MCO pin (PA.11) to check requested frequency:
 
 - SYSCLK frequency with frequency value around @64MHz.

### <b>Keywords</b>

Utils, system, Clock, HSI, PLL, flash latency, SYSCLK, frequency Oscilloscope



### <b>Directory contents</b>

  - UTILS/UTILS_ConfigureSystemClock/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - UTILS/UTILS_ConfigureSystemClock/Inc/main.h                  Header for main.c module
  - UTILS/UTILS_ConfigureSystemClock/Inc/stm32_assert.h          Template file to include assert_failed function
  - UTILS/UTILS_ConfigureSystemClock/Src/stm32wl3x_it.c          Interrupt handlers
  - UTILS/UTILS_ConfigureSystemClock/Src/main.c                  Main program
  - UTILS/UTILS_ConfigureSystemClock/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3RKBVx devices.
    
  - This example has been tested with NUCLEO-WL3RKB2 board and can be
    easily tailored to any other supported device and development board.

  - NUCLEO-WL3RKB2 Set-up
    - Connect the MCO pin to an oscilloscope to monitor the different waveforms:
      - PA.11: Arduino connector CN6 pin 4 (D11), Morpho connector CN4 pin 15 (MB1801)

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example


