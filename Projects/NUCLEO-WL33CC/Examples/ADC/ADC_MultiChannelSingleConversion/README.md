## <b>ADC_MultiChannelSingleConversion Example Description</b>

How to use an ADC peripheral to convert several channels. ADC conversions are 
performed successively in a scan sequence.
This example is based on the STM32WL3x ADC HAL API.

Example configuration:

ADC is configured in single conversion mode, from SW trigger.
Sequencer of ADC group regular (default group available on ADC of all STM32 devices)
is configured to convert 2 channels: 1 channel from GPIO, 1 internal channel: temperature sensor.
DMA is configured to transfer conversion data in an array of size three elements
(one array address for conversion data of each channel) in RAM memory,
in circular mode.

Example execution:

Every second, ADC performs conversion of a channel among the 2 channels
of the scan sequence, successively at each conversion start (discontinuous mode enabled).

Note: In case of discontinuous mode is disabled, then the entire scan sequence
      is converted in burst from one conversion start.

ADC conversion data of each sequence rank is transferred by DMA into the array
"uhADCxConvertedData":

- uhADCxConvertedData[0]: ADC channel set on sequence rank 1 (Temperature sensor)
- uhADCxConvertedData[1]: ADC channel set on sequence rank 2 (GPIO as analog input)

When sequence is completed, it restarts from the beginning: first channel 
in the sequence, data transfer in first array address (rollback).

ADC conversions raw data are computed to physical values
using LL ADC driver helper macro:


- Value of voltage on GPIO pin (on which is mapped ADC channel, cf pin below) (unit: mV)
- Value of temperature (unit: degree Celsius)

LD1 is used to monitor program execution status:

- Normal operation: Activity of ADC scan sequence

  can be observed with LED toggle:
  
  - At each ADC conversion: LED toggle once (every 1sec)
  - At each scan conversion completed: LED toggle 4 times quickly (10Hz)
  
- Error: LED remaining turned on

Debug: variables to monitor with debugger:

- "ubDmaTransferStatus": status of DMA transfer of ADC group regular conversions
- "uhADCxConvertedData":                          ADC group regular conversion data
- "uhADCxConvertedData_VoltageGPIO_mVolt":        Value of voltage on GPIO pin (on which is mapped ADC channel) (unit: mV)
- "hADCxConvertedData_Temperature_DegreeCelsius": Value of temperature (unit: degree Celsius)

Connection needed:

Use an external power supply to generate a voltage in range [0V; 3.3V]
and connect it to analog input pin (cf pin below).

Note: If no power supply available, this pin can be let floating (in this case
      ADC conversion data will be undetermined).

Other peripherals used:

  1 GPIO for LD1

  1 GPIO for analog input: ADC channel P0 on pin PB.03 (Arduino connector CN7 pin 4 A3, Morpho connector CN3 pin 34)

### <b>Keywords</b>

ADC, analog digital converter, analog, conversion, voltage, channel, analog input, trigger, analog watchdog, threshold, window

### <b>Directory contents</b>

  - ADC/ADC_AnalogWatchdog/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - ADC/ADC_AnalogWatchdog/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - ADC/ADC_AnalogWatchdog/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - ADC/ADC_AnalogWatchdog/Inc/main.h                  Header for main.c module
  - ADC/ADC_AnalogWatchdog/Src/stm32wl3x_it.c          Interrupt handlers
  - ADC/ADC_AnalogWatchdog/Src/stm32wl3x_hal_msp.c     HAL MSP module
  - ADC/ADC_AnalogWatchdog/Src/main.c                  Main program
  - ADC/ADC_AnalogWatchdog/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

