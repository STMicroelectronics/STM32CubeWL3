## <b>LPUART_WakeUpFromDeepStop_Init Example Description</b>

Configuration of GPIO and LPUART peripherals to allow characters
received on LPUART_RX pin to wake up the MCU from low-power mode. This example is based
on the LPUART LL API. The peripheral initialization uses LL
initialization function to demonstrate LL init usage.

    Board: NUCLEO-WL33CC (embeds a STM32WL33CCVx device)
    LPUART1 Tx Pin: PB.03 (pin 34 on CN3)
    LPUART1 Rx Pin: PA.05 (pin 21 on CN4)

LPUART Peripheral is configured in asynchronous mode (9600 bauds, 8 data bit, 1 start bit, 1 stop bit, no parity).
No HW flow control is used.
LPUART Clock is based on LSE.

Example execution:

After startup from reset and system configuration, LD1 is blinking quickly during 3 sec,
then MCU  enters "Deep Stop" mode (LD1 off).
On first  character reception by the LPUART from PC Com port (ex: using HyperTerminal), MCU wakes up
from "Deep Stop" Mode. Please note that this first character is used to wake up MCU but is not retrieved by LPUART.

After wake up, MCU keeps awake during 3s (LD1 is blinking).
During this period, all received characters are echoed on the LPUART TX line.
Received character value is checked :

- On a specific value ('S' or 's'), LD1 is turned On and program ends.
- If no 'S' or 's' is received during this period, program enters "Deep Stop" mode again,
  waiting for next character to wake up.

In case of errors, LD1 is slowly blinking (1 sec period).

#### <b>Notes</b>

In order to connect LPUART1 Tx and Rx pins to a PC, you may use a "USB-TTL Converter" .

Set-Up :

  - Tx converter connected with Rx LPUART
  - Rx converter connected with Tx LPUART
  - GND converter connected with GND Board

### <b>Keywords</b>

Connectivity, LPUART, baud rate, RS-232, HyperTerminal, full-duplex,
Transmitter, Receiver, Asynchronous, Low Power

### <b>Directory contents</b>

  - LPUART/LPUART_WakeUpFromDeepStop_Init/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - LPUART/LPUART_WakeUpFromDeepStop_Init/Inc/main.h                  Header for main.c module
  - LPUART/LPUART_WakeUpFromDeepStop_Init/Inc/stm32_assert.h          Template file to include assert_failed function
  - LPUART/LPUART_WakeUpFromDeepStop_Init/Src/stm32wl3x_it.c          Interrupt handlers
  - LPUART/LPUART_WakeUpFromDeepStop_Init/Src/main.c                  Main program
  - LPUART/LPUART_WakeUpFromDeepStop_Init/Src/system_stm32wl3x.c      STM32Wl3x system source file

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

  - NUCLEO-WL33CC Set-up
    Example is delivered to use a connection between NUCLEO-WL33CC and PC,
    GPIOs connected to LPUART1 TX/RX (PB3 and PA5) should be connected to PC UART Com port selected on PC side
    thanks to a UART-USB converter.

  - Launch serial communication SW on PC (as HyperTerminal or TeraTerm) with proper configuration
    (9600 bauds, 8 bits data, 1 stop bit, no parity, no HW flow control).


### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example
