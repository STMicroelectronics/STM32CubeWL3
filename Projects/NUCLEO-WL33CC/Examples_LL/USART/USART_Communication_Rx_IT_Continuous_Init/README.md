## <b>USART_Communication_Rx_IT_Continuous_Init Example Description</b>

This example shows how to configure GPIO and USART peripheral for continuously receiving characters
from HyperTerminal (PC) in Asynchronous mode using Interrupt mode. Peripheral initialization is
done using LL unitary services functions for optimization purpose (performance and size).

USART Peripheral is configured in asynchronous mode (115200 bauds, 8 data bit, 1 start bit, 1 stop bit, no parity).
No HW flow control is used.
GPIO associated to User push-button is linked with EXTI.

Example execution:

After startup from reset and system configuration, LD1 is blinking quickly.

On press on push button, USART RX Not Empty interrupt is enabled.

When character is received on USART Rx line, a RXNE interrupt occurs.

Received data is read from USART RDR register and stored in user buffer.

A double buffer is available for received data, allowing continuous reception.

User could process data received in Buffer A, while buffer B is used for reception.

When buffer B is full, it could be handled by user, while buffer A becomes active buffer for next reception, and so on.

Each time a reception buffer is full, user data process callback is called.

Data processing consists in echoing data buffer content on PC Com port, and making LD1 toggle.

In case of errors, LD1 is blinking.


### <b>Keywords</b>

Connectivity, UART/USART, Asynchronous, RS-232, baud rate, Interrupt, HyperTerminal, Continuous Reception,
Receiver

### <b>Directory contents</b>

  - USART/USART_Communication_Rx_IT_Continuous_Init/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - USART/USART_Communication_Rx_IT_Continuous_Init/Inc/main.h                  Header for main.c module
  - USART/USART_Communication_Rx_IT_Continuous_Init/Inc/stm32_assert.h          Template file to include assert_failed function
  - USART/USART_Communication_Rx_IT_Continuous_Init/Src/stm32wl3x_it.c          Interrupt handlers
  - USART/USART_Communication_Rx_IT_Continuous_Init/Src/main.c                  Main program
  - USART/USART_Communication_Rx_IT_Continuous_Init/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

  - NUCLEO-WL33CC Set-up
    Connect USART1 TX/RX to respectively RX and TX pins of PC UART (could be done through a USB to UART adapter) :
    - Connect STM32 MCU board USART1 TX pin (GPIO PA.01 connected to pin 47 of connector CN1)
      to PC COM port RX signal
    - Connect STM32 MCU board USART1 RX pin (GPIO PA.15 connected to pin 45 of connector CN1)
      to PC COM port TX signal
    - Connect STM32 MCU board GND to PC COM port GND signal

  - Launch serial communication SW on PC (as HyperTerminal or TeraTerm) with proper configuration
    (115200 bauds, 8 bits data, 1 stop bit, no parity, no HW flow control).

  - Launch the program. Enter characters on PC communication SW side.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

