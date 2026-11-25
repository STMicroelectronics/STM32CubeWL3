::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>LPM_Button_UDS Example Description</b>

This demonstration illustrates how to operate the STM32 Low Power Manager utility (LPM) to manage device low-power transitions.

At startup, LD1 turns on for 2 seconds. The application then uses the LPM to put the SoC in <b>Ultra-DeepStop</b> mode and configures Button 2 as a wakeup source.

Each time B2 is pressed, the device exits Ultra-DeepStop. No CPU and peripheral contexts save/restore are provided in this mode, so execution restarts from the boot sequence. RAM0 remains powered during Ultra-DeepStop. 
The serial terminal reports the wakeup before the application re-enters Ultra-DeepStop.

### <b>Keywords</b>

PWR, LPM, Low Power Manager, Ultra-DeepStop

### <b>Directory contents</b>

  - LPM_Button_UDS/Core/Inc/app_conf.h                    Application low-power and trace configuration file
  - LPM_Button_UDS/Core/Inc/app_entry.h                   Application interface header
  - LPM_Button_UDS/Core/Inc/main.h                        Main program header
  - LPM_Button_UDS/Core/Inc/stm32_assert.h                Assert description header
  - LPM_Button_UDS/Core/Inc/stm32wl3r_nucleo_conf.h       BSP configuration header
  - LPM_Button_UDS/Core/Inc/stm32wl3x_hal_conf.h          HAL configuration header
  - LPM_Button_UDS/Core/Inc/stm32wl3x_it.h                Interrupt handler header
  - LPM_Button_UDS/Core/Inc/utilities_conf.h              STM32 utilities configuration header
  - LPM_Button_UDS/Core/Src/app_entry.c                   Application entry point
  - LPM_Button_UDS/Core/Src/main.c                        Main program
  - LPM_Button_UDS/Core/Src/stm32wl3x_hal_msp.c           HAL MSP module
  - LPM_Button_UDS/Core/Src/stm32wl3x_it.c                Interrupt handlers
  - LPM_Button_UDS/Core/Src/system_stm32wl3x.c            STM32WL3x system clock configuration
  - LPM_Button_UDS/System/Interfaces/stm32_lpm_if.c       Low Power Manager interface implementation
  - LPM_Button_UDS/System/Interfaces/stm32_lpm_if.h       Low Power Manager interface header
  - LPM_Button_UDS/System/Modules/asm.h                   Assembly macro definitions
  - LPM_Button_UDS/System/Modules/osal.c                  OS abstraction layer implementation
  - LPM_Button_UDS/System/Modules/osal.h                  OS abstraction layer header
  - LPM_Button_UDS/System/Startup/cpu_context_switch.s    CPU context switch implementation
  - LPM_Button_UDS/System/Startup/device_context_switch.c Device context switch implementation
  - LPM_Button_UDS/System/Startup/device_context_switch.h Device context switch header

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL3RKBVx devices

  - This example has been tested with the STMicroelectronics NUCLEO-WL3RKB1 application board and can be tailored to other supported devices and development boards

### <b>How to use it ?</b>

To run the demonstration:

1. Open the complete project in the `EWARM`, `MDK-ARM`, or `STM32CubeIDE` directory for your preferred toolchain.
2. Build the project and program the target board.
3. Connect a serial terminal to COM1 and configure it for 115200 bit/s, 8 data bits, no parity, 1 stop bit, and no flow control.
4. Reset or run the board and verify that the Low Power Manager banner is displayed before the device enters Ultra-DeepStop.
5. Press B2 to wake the device and verify that the serial terminal reports the Ultra-DeepStop wakeup.
:::
:::