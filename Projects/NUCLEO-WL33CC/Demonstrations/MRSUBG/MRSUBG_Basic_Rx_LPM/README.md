::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_Basic_Rx_LPM Example Description</b>

This example configures the NUCLEO-WL33CC1 as a basic MRSUBG packet receiver with low-power support. It receives variable-length 2-FSK packets at 868 MHz with a 38.4 kbps data rate, a 20-byte maximum payload, an 8-bit CRC, and the sync word `0x88888888`.

The MRSUBG interrupt is enabled before the receiver is started. After starting the receiver, the application waits for interrupts using `WFI`. The RX interrupt handler processes received packets through `RX_IRQHandler()`.

### <b>Low-power behavior</b>

When no processing is pending, the device enters **Sleep mode**. It is woken by the MRSUBG `RX_OK` interrupt when a valid packet is received.

Use `Examples/MRSUBG/MRSUBG_BasicGeneric_Tx` on a second NUCLEO-WL33CC1 board as the transmitter. Its radio and packet settings are compatible with this receiver.

### <b>Keywords</b>

MRSUBG, radio, RX, low power, WFI

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Inc/app_entry.h               Application entry interface
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Src/main.c                    Main program
  - MRSUBG/MRSUBG_Basic_Rx_LPM/Core/Src/app_entry.c               RX application and low-power processing
  - MRSUBG/MRSUBG_Basic_Rx_LPM/System/Interfaces             Low-power mode interface
  - MRSUBG/MRSUBG_Basic_Rx_LPM/System/Modules                OS abstraction layer
  - MRSUBG/MRSUBG_Basic_Rx_LPM/System/Startup                Context switching support


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open the project for your preferred toolchain: `EWARM/Project.eww`, `MDK-ARM/MRSUBG_Basic_Rx_LPM.uvprojx`, or import the `STM32CubeIDE` directory.
 - Rebuild all files and load your image into target memory
 - Run the example and start `Examples/MRSUBG/MRSUBG_BasicGeneric_Tx` on a second NUCLEO-WL33CC1 board to transmit compatible packets

:::
:::