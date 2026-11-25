### <b>PWR_ModesSelection Example Description</b>

How to select a power mode through USART1 and configure the system to measure
the MCU current consumption.

In the associated software, the system clock is set to 64 MHz and a menu is
displayed on the virtual COM port. The following configurations are available:

| Selection | Configuration |
| --- | --- |
| 1 | Run at 64 MHz, idle loop executing from Flash |
| 2 | Sleep at 64 MHz, no periodic wakeup |
| 3 | DeepStop, LSI and LSE off, SRAM retained |
| 4 | DeepStop, LSI on, LSE off, SRAM retained, RTC off |
| 5 | Shutdown, BOR off |
| 6 | Shutdown, BOR on |
| 7 | Ultra DeepStop, BOR off |
| 8 | Ultra DeepStop, BOR on |

After a valid selection, the application waits for 5 seconds, then disables
the USART, turns off the LEDs and prepares the GPIOs for current measurement.
SysTick, interrupts, wakeup sources and debug retention are disabled before
entering the selected configuration.

External RESET is required to return to the menu and select another mode.
The USER push-buttons and serial input do not wake up the MCU.

The LEDs are used to monitor the system state as follows:

 - All LEDs off: current measurement configuration active
 - LD3 ON: configuration failed or a reset-only power mode unexpectedly returned;
   the system enters an infinite loop and the measurement is not valid

#### <b>Notes</b>

1. The system clock uses RC64MPLL at 64 MHz, with HSE enabled for Run and Sleep.
   SMPS is enabled using the existing board startup configuration. No radio,
   RTC, watchdog or application timer is initialized. LSI is enabled only for
   selection 4, and LSE remains off. DeepStop uses the startup SRAM retention
   configuration; Shutdown does not retain SRAM. Ultra DeepStop is an additional
   low-power mode supported by STM32WL3R devices.
2. Measure the MCU supply current using the current-measurement connection
   described in the board user manual, not the complete board USB input current.
   Use an instrument with a suitable range and burden voltage for the selected mode.
   Disconnect ST-LINK/SWD, the virtual COM port and external circuitry during
   the 5-second delay, while maintaining a valid MCU supply. Avoid back-powering
   through GPIOs. Measure only after the current has settled.
3. Compare current consumption with the device datasheet only under matching
   supply voltage, temperature, SMPS, clock, BOR, SRAM retention, GPIO and
   wakeup-source conditions. The Run idle loop is not necessarily equivalent
   to the datasheet workload. Typical values are not guaranteed maximum limits.
   Formal electrical validation against the datasheet remains pending.
4. SWD pin functions are disabled during measurement. To reprogram the device,
   connect under reset. After reset, the menu remains active until a valid
   selection is received.
5. HAL_Delay() requires a 1 ms SysTick time base during initialization and the
   preparation delay. SysTick is deliberately stopped for current measurement.
6. The MSP lowPowerIOSetup() helper is called after console shutdown, not from
   HAL_MspInit(), so the menu remains available until measurement preparation.
7. This example is manually maintained; no CubeMX .ioc file is provided.

### <b>Keywords</b>

Power, PWR, Run, Sleep, DeepStop, Shutdown, Ultra DeepStop, Low Power, Current consumption, USART, BOR, External reset

### <b>Directory contents</b>

  - PWR/PWR_ModesSelection/Inc/stm32wl3r_nucleo_conf.h  BSP configuration file
  - PWR/PWR_ModesSelection/Inc/stm32wl3x_hal_conf.h     HAL configuration file
  - PWR/PWR_ModesSelection/Inc/stm32wl3x_it.h           Header for stm32wl3x_it.c
  - PWR/PWR_ModesSelection/Inc/main.h                  Header file for main.c
  - PWR/PWR_ModesSelection/Src/system_stm32wl3x.c       System clock and startup configuration
  - PWR/PWR_ModesSelection/Src/stm32wl3x_it.c           Interrupt handlers
  - PWR/PWR_ModesSelection/Src/main.c                  Main program
  - PWR/PWR_ModesSelection/Src/stm32wl3x_hal_msp.c      HAL MSP module and low-power IO setup

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3RKBVx devices and is provided for the
    STMicroelectronics NUCLEO-WL3RKB2 board.
  - EWARM, MDK-ARM and STM32CubeIDE projects are provided.
  - NUCLEO-WL3RKB2 Set-up
    - USART1 virtual COM port: PA1 (TX) and PB14 (RX).
      Refer to the board user manual for the solder-bridge routing.
    - Terminal configuration: 115200 baud, 8 data bits, no parity, 1 stop bit,
      no flow control.
    - Connect an ammeter in the MCU supply path as described in the board user manual.

### <b>How to use it ?</b>

In order to make the program work, you must do the following:

 - Open your preferred toolchain project for NUCLEO-WL3RKB2.
 - Rebuild all files and load your image into target memory.
 - Open a terminal with the configuration above, then run or reset the example.
 - Send one menu digit. No Enter key is required; CR and LF are ignored.
   Invalid input is reported and selection remains active.
 - Disconnect debug and serial links during the 5-second preparation delay,
   keeping the MCU powered as described in the Notes.
 - Measure the settled MCU supply current.
 - Apply an external RESET to select another configuration.
