## <b>COMP_CompareGpioVsVrefInt_OutputGpio_Init Example Description</b>

How to use a comparator peripheral to compare a voltage level applied on
a GPIO pin to the internal voltage reference (VrefInt)
with comparator output connected to a GPIO pin.
This example is based on the STM32WL3x COMP LL API.
The peripheral initialization is done using LL unitary service functions
for optimization purposes (performance and size).

Example configuration:

Comparator instance COMP1 is configured to compare:

- input plus set to a GPIO pin (cf pin below) in analog mode.
- input minus set to 3.3V.
  
A GPIO pin a configured in alternate function to be connected directly
to comparator output (cf pin below).

Example execution:

From the main program execution, comparator is enabled.
Each time the voltage level applied on GPIO pin (comparator input plus) 
is crossing VrefInt voltage threshold (comparator input minus),
the comparator output set the logical level on its affected GPIO.
The program reads this GPIO state and reflects it on LED state.

LD1 is used to monitor program execution status:

- Normal operation:

  - LED turned on if comparator voltage level on input plus is higher than input minus
  - LED turned off if comparator voltage level on input plus is lower than input minus
  
- Error: LED remaining turned on

Connection needed:

Use an external power supply to generate a voltage in range [0V; 3.3V]
and connect it to analog input pin (cf pin below).

Note: If no power supply available, this pin can be let floating (in this case
      comparator output state will be undetermined).

Other peripherals used:

  1 GPIO for LD1
  
  1 GPIO for analog input: pin PB.01 (Arduino connector CN7 pin 2 A1, Morpho connector CN3 pin 30)
  
  1 GPIO for digital output of comparator: PA.04 (Arduino connector CN8 pin 8 D7, Morpho connector CN4 pin 23)

#### <b>Notes</b> 

-The COMP negative input can use an internal generated voltage. This voltage is generated as a fraction of the battery voltage instead of 1.2 V internal voltage.This causes the internal COMP generated voltage to be battery-level dependent.(refer to device Errata sheet).

### <b>Keywords</b>

Comparator, COMP, analog, voltage, analog input, threshold, VrefInt

### <b>Directory contents</b>

  - COMP/COMP_CompareGpioVsVrefInt_OutputGpio_Init/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - COMP/COMP_CompareGpioVsVrefInt_OutputGpio_Init/Inc/main.h                        Header for main.c module
  - COMP/COMP_CompareGpioVsVrefInt_OutputGpio_Init/Inc/stm32_assert.h                Template file to include assert_failed function
  - COMP/COMP_CompareGpioVsVrefInt_OutputGpio_Init/Src/stm32wl3x_it.c          Interrupt handlers
  - COMP/COMP_CompareGpioVsVrefInt_OutputGpio_Init/Src/main.c                        Main program
  - COMP/COMP_CompareGpioVsVrefInt_OutputGpio_Init/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
    
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.


### <b>How to use it ?</b> 

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

