## <b>LCSC_BasicDemo Example Description</b>

How to use an LC sensor controller (LCSC) peripheral to measure rotation of wheel.

This example is based on the STM32WL3x LCSC HAL API.

Example configuration:

The LCSC control the COMP, DAC, VCMBUFF and PB1, PA14, PB2 in order to implement the LCSC feature.

This example is configured to count the number of wheel rotations in both clockwise and anticlockwise directions
with LCA and LCB.

When the tamper (LCT) detection is activated, it is possible to detect if a metallic surface has interfered with the measurement of the number of revolutions.

Example execution:

This example can be divided in the following parts:

 - initialization of the LCSC IP.

 - initial calibration of the LCSC (for this step the coils must not be exposed to the metal part).

 - infinite while loop, and the LCSC working on its own.
   The wheel can be placed in front of the coils only after the initial calibration, and can eventually start rotating.
   When the wheel performs a number of rotations, clockwise or anticlockwise, equal to the threshold defined in the proper register.
   A message is printed at the terminal.

Connection needed:

Hardware used for the LC network in this example :

  Inductances: La, Lb and Lt = 470uF (Murata 11R474C)

  Capacitors: Ca, Cb and Ct = 560pF Cext_LCSC = 470nF

Note: The configuration of the LCSC must be updated based on the inductors and capacitors used.

### <b>Keywords</b>

LCSC, COMP, DAC, LCA, LCB, LCT, LC network

### <b>Directory contents</b>

  - LCSC/LCSC_BasicDemo/Inc/stm32wl3x_nucleo_conf.h     BSP configuration file
  - LCSC/LCSC_BasicDemo/Inc/stm32wl3x_hal_conf.h    HAL configuration file
  - LCSC/LCSC_BasicDemo/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - LCSC/LCSC_BasicDemo/Inc/main.h                  Header for main.c module
  - LCSC/LCSC_BasicDemo/Src/stm32wl3x_it.c          Interrupt handlers
  - LCSC/LCSC_BasicDemo/Src/stm32wl3x_hal_msp.c     HAL MSP module
  - LCSC/LCSC_BasicDemo/Src/main.c                  Main program
  - LCSC/LCSC_BasicDemo/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with STMicroelectronics NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

