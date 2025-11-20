## <b>DAC_SignalsGeneration Example Description</b>

How to use the DAC peripheral to generate several signals using the DMA
controller and the DAC internal wave generator.
For each press on USER push-button B1, a signal has been selected and can be monitored on the DAC channel one :

    - Triangle waveform (Channel 1), amplitute: ~1V, frequency: order of Hz.
    - Escalator waveform (Channel 1) using DMA transfer, amplitute: ~3V, frequency: order of KHz.

NUCLEO-WL33CC board's LEDs can be used to monitor the process status:

  - LD3 is ON and example is stopped (using infinite loop) when there is an error during process.

#### <b>Notes</b>

 1. Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
    based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
    a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
    than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
    To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

 2. The application need to ensure that the SysTick time base is always set to 1 millisecond
    to have correct HAL operation.

### <b>Keywords</b>

Analog, DAC, Digital to Analog, Continuous conversion, DMA, Escalator generation, Timer Trigger

### <b>Directory contents</b>

  - DAC/DAC_SignalsGeneration/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - DAC/DAC_SignalsGeneration/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - DAC/DAC_SignalsGeneration/Inc/stm32wl3x_it.h          DAC interrupt handlers header file
  - DAC/DAC_SignalsGeneration/Inc/main.h                  Header for main.c module  
  - DAC/DAC_SignalsGeneration/Src/stm32wl3x_it.c          DAC interrupt handlers
  - DAC/DAC_SignalsGeneration/Src/main.c                  Main program
  - DAC/DAC_SignalsGeneration/Src/stm32wl3x_hal_msp.c     HAL MSP file
  - DAC/DAC_SignalsGeneration/Src/system_stm32wl3x.c      STM32WL3x system source file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

  - NUCLEO-WL33CC Set-up :
      
      - Connect PA13 (DAC Channel1) (pin 6 in CN8) to an oscilloscope.
      
      - Use USER push-button B1 connected to PA0.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
