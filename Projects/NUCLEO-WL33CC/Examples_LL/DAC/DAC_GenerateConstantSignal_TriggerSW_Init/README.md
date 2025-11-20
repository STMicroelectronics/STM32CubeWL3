## <b>DAC_GenerateConstantSignal_TriggerSW_Init Example Description</b>


How to use the DAC peripheral to generate a constant voltage signal. This
example is based on the STM32WL3x DAC LL API. The peripheral 
initialization uses LL unitary service functions for optimization purposes
(performance and size).

Example configuration:
One DAC channel (DAC channel1) is configured to connect DAC channel output on GPIO pin
to get the samples from SW (no DMA transfer) and to get conversion trigger from SW.

Example execution:

From the main program execution, LD1 is toggling quickly while waiting for
user button press.

Then, the DAC is configured and activated: constant signal is generated on DAC output
indefinitely.

DAC channel output value is provided by SW, a new value is loaded
at each press on push button: LD1 toggles and the signal starts from 0, increased by a quarter of Vdda and
finishing at Vdda voltage.

Finally, LD1 is turned-on.

Connection needed:
None. 

Oscilloscope for monitoring DAC channel output (cf pin below).

Other peripheral used:

  1 GPIO for push button
  
  1 GPIO for DAC channel output PA13 (Arduino connector CN8 pin 6, Morpho connector CN4 pin 27)

### <b>Keywords</b>

Analog, DAC, Digital to Analog, Continuous conversion,  DMA, Sine-wave generation, Software Trigger


### <b>Directory contents</b> 

  - DAC/DAC_GenerateConstantSignal_TriggerSW_Init/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - DAC/DAC_GenerateConstantSignal_TriggerSW_Init/Inc/main.h                  Header for main.c module
  - DAC/DAC_GenerateConstantSignal_TriggerSW_Init/Inc/stm32_assert.h          Template file to include assert_failed function
  - DAC/DAC_GenerateConstantSignal_TriggerSW_Init/Src/stm32wl3x_it.c          Interrupt handlers
  - DAC/DAC_GenerateConstantSignal_TriggerSW_Init/Src/main.c                  Main program
  - DAC/DAC_GenerateConstantSignal_TriggerSW_Init/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
    
  - This example has been tested with STM32WL33CCVx board and can be
    easily tailored to any other supported device and development board.


### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
