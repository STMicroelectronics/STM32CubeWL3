﻿## <b>IWDG_RefreshUntilUserEvent_Init Example Description</b>

How to configure the IWDG peripheral to ensure periodical counter update and
generate an MCU IWDG reset when a USER push-button is pressed. The peripheral
is initialized with LL unitary service functions to optimize
for performance and size.

Example Configuration:

Configure the IWDG (prescaler, counter) and enable it.

Infinite refresh of the IWDG down-counter done in the main loop.
LD1 is blinking fast & continuously.

Example Execution:

When USER push-button is pressed, the down-counter automatic refresh mechanism is
disable and thus, reset will occur. After a reset, when re-entering in the main,
RCC IWDG Reset Flag will be checked and if we are back from a IWDG reset the LD1
will be switch ON.

Waiting a new USER push-button pressed to re-activate the IWDG

#### <b>Notes</b>

 1. This example must be tested in standalone mode (not in debug).

### <b>Keywords</b>

System, IWDG, reload counter, MCU Reset

### <b>Directory contents</b>

  - IWDG/IWDG_RefreshUntilUserEvent_Init/Inc/stm32wl3x_it.h          Interrupt handlers header file
  - IWDG/IWDG_RefreshUntilUserEvent_Init/Inc/main.h                        Header for main.c module
  - IWDG/IWDG_RefreshUntilUserEvent_Init/Inc/stm32_assert.h                Template file to include assert_failed function
  - IWDG/IWDG_RefreshUntilUserEvent_Init/Src/stm32wl3x_it.c          Interrupt handlers
  - IWDG/IWDG_RefreshUntilUserEvent_Init/Src/main.c                        Main program
  - IWDG/IWDG_RefreshUntilUserEvent_Init/Src/system_stm32wl3x.c      STM32WL3x system source file


### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.

  - This example has been tested with NUCLEO-WL33CC board and can be
    easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example


