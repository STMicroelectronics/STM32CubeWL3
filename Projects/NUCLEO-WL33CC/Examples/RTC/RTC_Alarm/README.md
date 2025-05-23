## <b>RTC_Alarm Example Description</b>

Configuration and generation of an RTC alarm using the RTC HAL API.

At the beginning of the main program the HAL_Init() function is called to reset 
all the peripherals, initialize the Flash interface and the systick.

Then the *SystemClock_Config()* function is used to configure the system clock (SYSCLK) to run at 64Mhz.

The RTC peripheral configuration is ensured by the HAL_RTC_Init() function.
This later is calling the HAL_RTC_MspInit()function which core is implementing
the configuration of the needed RTC resources according to the used hardware (CLOCK, 
PWR, RTC clock source and BackUp). You may update this function to change RTC configuration.

    LSI oscillator clock is used as RTC clock source by default.
    The user can use also LSE as RTC clock source.
    - The user uncomment the adequate line on the main.h file.
      @code
        #define RTC_CLOCK_SOURCE_LSI
        /* #define RTC_CLOCK_SOURCE_LSE */
      @endcode
    - Open the ioc file with STM32CubeMX and select :
      - LSE as "Crystal/Ceramic Resonator" in RCC configuration.
      - LSE as RTC clock source in Clock configuration.
    - Generate code
    LSI oscillator clock is delivered by a 32 kHz RC.
    LSE (when available on board) is delivered by a 32.768 kHz crystal.

**HAL_RTC_SetDate()** and **HAL_RTC_SetTime()** functions are called to initialize the time and the date.
**HAL_RTC_SetAlarm_IT()** function is then called to initialize the Alarm feature with interrupt mode.

In this example, the Time is set to 02:20:00 and the Alarm must be generated after 
30 seconds on 02:20:30.

LD1 is turned ON when the RTC Alarm is generated correctly.
The current time is updated and displayed on the debugger in aShowTime variable.
In case of error, LD3 is toggled with a period of one second.

#### <b>Notes</b>

 1. Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
    based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
    a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
    than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
    To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
 2. The application need to ensure that the SysTick time base is always set to 1 millisecond
    to have correct HAL operation.

### <b>Keywords</b>

System, RTC, Alarm, wakeup timer, Backup domain, Counter, LSE, LSI

### <b>Directory contents</b>

  - RTC/RTC_Alarm/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - RTC/RTC_Alarm/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - RTC/RTC_Alarm/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - RTC/RTC_Alarm/Inc/main.h                  Header for main.c module  
  - RTC/RTC_Alarm/Src/stm32wl3x_it.c          Interrupt handlers
  - RTC/RTC_Alarm/Src/main.c                  Main program
  - RTC/RTC_Alarm/Src/stm32wl3x_hal_msp.c     HAL MSP module
  - RTC/RTC_Alarm/Src/system_stm32wl3x.c      STM32WL3x system source file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC 
    board and can be easily tailored to any other supported device 
    and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

