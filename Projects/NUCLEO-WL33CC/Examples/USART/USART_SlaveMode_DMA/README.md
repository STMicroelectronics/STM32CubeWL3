## <b>USART_SlaveMode_DMA Example Description</b>

This example describes an USART-SPI communication (transmit/receive) with DMA between two
boards where the USART is configured as a slave.

Boards: NUCLEO-WL33CC (embeds a STM32WL33CCVx device)


    Two identical boards are connected as described below.
    Board 1: SPI Master board
    Board 2: USART Slave board

In a first step, after the user press on the User push-button, Master (SPI) starts
the communication by sending aTxBuffer and receiving aRxBuffer through
HAL_SPI_TransmitReceive(), at the same time Slave (USART) transmits aTxBuffer
and receives aRxBuffer through HAL_USART_TransmitReceive().
Finally, aRxBuffer and aTxBuffer are compared through Buffercmp() in order to
check buffers correctness.

WARNING: as both boards do not behave the same way, "SLAVE_BOARD" compilation
switch is defined in Inc/main.h and must be enabled at compilation time before
loading the executable in the USART slave board.
The SPI master board needs to be loaded with an executable software obtained
with SLAVE_BOARD disabled.
if you want to repeat the execution of the example, you should reset the master board
first else the execution will fail and a red led will be on.

NUCLEO-WL33CC board LEDs are used to monitor the transfer status:

- While master board is waiting for the user to press the User push-button, its
  LD2 is blinking rapidly (100 ms period).
- While slave board is waiting for the reception of the buffer, its
  LD2 is blinking rapidly (100 ms period).
- When the transmission/reception processes starts, LD2 is turned off.
- When the test passes, LD1 on both boards is turned on, otherwise the
  test has failed.
- If there is an initialization or transfer error, LD3 is turned on.

At the beginning of the main program the HAL_Init() function is called to reset
all the peripherals, initialize the Flash interface and the systick.

The SystemClock_Config() function is used to configure the system clock for STM32WL33CCVx Devices :
The CPU at 64 MHz.

The HCLK for AHB peripherals to run at 64 MHz.
The APB clock dividers APB1, APB2, APB3 peripherals to run at 64 MHz.

**Note:** USARTx/SPIx instances used and associated resources can be updated in
"main.h" file depending hardware configuration used.

#### <b>Notes</b>

 1. Care must be taken when using HAL_Delay(), this function provides accurate
    delay (in milliseconds) based on variable incremented in SysTick ISR. This
    implies that if HAL_Delay() is called from a peripheral ISR process, then
    the SysTick interrupt must have higher priority (numerically lower) than
    the peripheral interrupt. Otherwise the caller ISR process will be blocked.
    To change the SysTick interrupt priority you have to use
    HAL_NVIC_SetPriority() function.

 2. The example needs to ensure that the SysTick time base is always set to
    1 millisecond to have correct HAL operation.

 3. It is highly recommended to set the SPI parameter "MasterKeepIOState" to "SPI_MASTER_KEEP_IO_STATE_ENABLE"
    at init time. This configuration allows to avoid glitches on SPI lines especially the clock.
    Glitches that could lead to Master/Slave desynchronization. This config allows SPI Master to keep
    control on output state even if it is disabled.

### <b>Keywords</b>

Connectivity, UART/USART, SPI interface, full-duplex, Polling, Receiver, Transmitter, Synchronous, Slave, DMA

### <b>Directory contents</b> 

  - USART/USART_SlaveMode_DMA/Inc/stm32wl3x_hal_conf.h     HAL configuration file
  - USART/USART_SlaveMode_DMA/Inc/stm32wl3x_it.h           interrupt handlers header file
  - USART/USART_SlaveMode_DMA/Inc/main.h                         Header for main.c module
  - USART/USART_SlaveMode_DMA/Inc/stm32wl3x_nucleo_conf.h  STM32WL3x_Nucleo board configuration file
  - USART/USART_SlaveMode_DMA/Src/stm32wl3x_it.c           interrupt handlers
  - USART/USART_SlaveMode_DMA/Src/main.c                         Main program
  - USART/USART_SlaveMode_DMA/Src/stm32wl3x_hal_msp.c      HAL MSP module
  - USART/USART_SlaveMode_DMA/Src/system_stm32wl3x.c       STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with two NUCLEO-WL33CC boards embedding
    a STM32WL33CCVx device and can be easily tailored to any other supported device
    and development board.

  - NUCLEO-WL33CC set-up
    - Connect Master board PB.06 (SCK)(35 CN4) to Slave Board PB8(USART CK)(5 CN6)
    - Connect Master board PA.08 (MISO)(29 CN4) to Slave Board PA1 (USART TX)(37 CN3)
    - Connect Master board PB.07 (MOSI)(37 CN4) to Slave Board PA15 (USART RX)(35 CN3)
    - Connect Master board GND to Slave Board GND

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

  - Open your preferred toolchain
  - Rebuild all files and load your image into target memory :
    - Uncomment "#define SLAVE_BOARD" and load the project in Slave Board
    - Comment "#define SLAVE_BOARD" and load the project in Master Board
  - Run the example

