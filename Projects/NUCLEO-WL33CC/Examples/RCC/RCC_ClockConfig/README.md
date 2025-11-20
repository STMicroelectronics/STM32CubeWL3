## <b>RCC_ClockConfig Example Description</b>

Configuration of the system clock (SYSCLK) and modification of the clock settings in Run mode, using the RCC HAL API.

   - Board: NUCLEO-WL33CC (embeds a STM32WL33CCVx device)
   - Microcontroller Clock Output MCO Pin: PA.05 (Arduino connector CN6 pin 1 D8, Morpho connector CN4 pin 21)

In this example, after startup SYSCLK is configured to the max frequency using the PLL.
the USER push-button (connected to B1_PIN) will be used to change the SYSCLK :

- From PLL to HSI
- From HSI to PLL

Each time the USER push-button is pressed B1_PIN interrupt is generated and in the ISR
the PLL oscillator source is checked using __HAL_RCC_RC64MPLL_READYFLAG() macro:

  - if the HSI is selected as SYSCLK source, the following steps will be followed :
  
      a. Switch the system clock source to HSI to disable the PLL and configure bus clocks dividers
      b. Disable HSE oscillator

  - if the PLL is selected as SYSCLK source, the following steps will be followed :
      a. Enable the HSE oscillator
      b. Switch the system clock source to PLL and configure bus clocks dividers


LD1 is toggled with a timing defined by the HAL_Delay() API.

#### <b>Notes</b>

 1. Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
    based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
    a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
    than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
    To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

 2. The application needs to ensure that the SysTick time base is always set to 1 millisecond
    to have correct HAL operation.

### <b>Keywords</b>

RCC, System, Clock Configuration, HSE, HSI, System clock, Oscillator, PLL

### <b>Directory contents</b>

  - RCC/RCC_ClockConfig/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - RCC/RCC_ClockConfig/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - RCC/RCC_ClockConfig/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - RCC/RCC_ClockConfig/Inc/main.h                  Header for main.c module
  - RCC/RCC_ClockConfig/Src/stm32wl3x_it.c          Interrupt handlers
  - RCC/RCC_ClockConfig/Src/main.c                  Main program
  - RCC/RCC_ClockConfig/Src/system_stm32wl3x.c      STM32WL3x system source file
  - RCC/RCC_ClockConfig/Src/stm32wl3x_hal_msp.c     HAL MSP module

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
