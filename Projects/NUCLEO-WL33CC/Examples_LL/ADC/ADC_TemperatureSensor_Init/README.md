## <b>ADC_TemperatureSensor_Init Example Description</b>

This example shows how to use the ADC peripheral to perform a single
conversion of the internal temperature sensor and calculate the temperature
in degrees Celsius. It uses the polling programming model.

The example is based on the STM32WL3x ADC LL API. Peripheral initialization
uses LL unitary service functions for optimization purposes (performance and
size).

#### <b>Example Configuration</b>

The ADC is configured to convert a single channel in single-conversion mode,
using a software trigger. The regular group sequencer converts the internal
temperature sensor channel. ADC down-sampling is configured for a 16-bit
output with an oversampling ratio of 128.

USART1 is configured on PA1 (TX) and PA15 (RX) at 115200 baud, 8 data bits, no
parity, and one stop bit (8N1).

###### <b>Example Execution</b>

The program configures the ADC, GPIO, and USART1, then repeats these steps in
an infinite loop:

- Turn LED2 off and wait 200 ms.
- Start an ADC conversion and poll for conversion completion.
- Read the conversion data and calculate calibrated and typical temperature
  values in degrees Celsius.
- Send the raw ADC result and both temperature values over USART1.
- Turn LED2 on for 500 ms.

For debugging, monitor these variables in the debugger watch window:

| Variable | Description |
| --- | --- |
| `uhADCxConvertedData` | ADC conversion data |
| `hADCxConvertedData_Temperature_DegreeCelsius` | Calibrated temperature in degrees Celsius |
| `hADCxConvertedData_TemperatureTypical_DegreeCelsius` | Typical temperature in degrees Celsius |

The UART output reports the raw conversion value (`raw`), calibrated
temperature (`temp_cal`), and typical temperature (`temp_typ`). In case of an
error, LED2 toggles every second.

#### <b>Connections Needed</b>

Connect a serial terminal to USART1 at 115200 baud, 8N1, using PA1 for TX and
PA15 for RX, to view conversion and temperature readings.

#### <b>Other Peripherals Used</b>

- GPIO for LED2
- USART1 for serial output

#### <b>Keywords</b>

ADC, analog digital converter, analog, conversion, voltage, channel, analog
input, trigger, temperature sensor

#### <b>Directory Contents</b>

| Path | Description |
| --- | --- |
| `ADC_TemperatureSensor_Init/Inc/main.h` | Header for the `main.c` module |
| `ADC_TemperatureSensor_Init/Inc/stm32wl3x_it.h` | Interrupt handlers header file |
| `ADC_TemperatureSensor_Init/Src/main.c` | Main program |
| `ADC_TemperatureSensor_Init/Src/stm32wl3x_it.c` | Interrupt handlers |
| `ADC_TemperatureSensor_Init/Src/system_stm32wl3x.c` | STM32WL3x system source file |
| `ADC_TemperatureSensor_Init/EWARM/` | IAR Embedded Workbench project files |
| `ADC_TemperatureSensor_Init/MDK-ARM/` | Keil MDK-ARM project files |
| `ADC_TemperatureSensor_Init/STM32CubeIDE/` | STM32CubeIDE linker and application startup files |

#### <b>Hardware and Software Environment</b>

- This example runs on STM32WL33CCVx devices.
- It has been tested with the NUCLEO-WL33CC board and can be tailored to any
  other supported device and development board.

#### <b>How to Use </b>

1. Open the project in your preferred toolchain.
2. Rebuild all files and load the image into target memory.
3. Run the example.
