## <b>UART_Printf Example Description</b>

Re-routing of the C library printf function to the UART.
The UART outputs a message on the HyperTerminal.

Then the SystemClock_Config() function is used to configure the system
clock (SYSCLK) to run at 64 MHz.

- Board : NUCLEO-WL33CC
- Tx Pin: PA.01 : (connected to pin 37 of connector CN3 :available through VCP)
- Rx Pin: PA.15 : (connected to pin 35 of connector CN3 :available through VCP) 

LD3 turns ON when there is an error.

The USART is configured as follows:

    - BaudRate = 115200 baud
    - Word Length = 8 Bits (7 data bit + 1 parity bit)
    - One Stop Bit
    - Odd parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Reception and transmission are enabled in the time

#### <b>Notes</b>

1. When the parity is enabled, the computed parity is inserted at the MSB
   position of the transmitted data.

2. Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
   based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
   a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
   than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
   To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

3. The application needs to ensure that the SysTick time base is always set to 1 millisecond
   to have correct HAL operation.

### <b>Keywords</b>

Connectivity, UART/USART, Printf, baud rate, RS-232, HyperTerminal, full-duplex, Interrupt,
Transmitter, Receiver, Asynchronous

### <b>Directory contents</b>

  - UART/UART_Printf/Inc/stm32wl3x_nucleo_conf.h BSP configuration file
  - UART/UART_Printf/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - UART/UART_Printf/Inc/stm32wl3x_it.h          IT interrupt handlers header file
  - UART/UART_Printf/Inc/main.h                   Header for main.c module
  - UART/UART_Printf/Src/stm32wl3x_it.c          Interrupt handlers
  - UART/UART_Printf/Src/main.c                   Main program
  - UART/UART_Printf/Src/stm32wl3x_hal_msp.c     HAL MSP module
  - UART/UART_Printf/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

  - NUCLEO-WL33CC Set-up
    Connect ST-Link cable to the PC USB port to display data on the HyperTerminal.
      A virtual COM port will then appear in the HyperTerminal.

  - Hyperterminal configuration:
    - Data Length = 7 Bits
    - One Stop Bit
    - Odd parity
    - BaudRate = 115200 baud
    - Flow control: None

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

