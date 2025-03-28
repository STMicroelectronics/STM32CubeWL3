
---
pagetitle: Readme
lang: en
---
::: {.row}
::: {.col-sm-12 .col-lg-8}

## <b>Templates_LL_ Example Description</b>

- This project provides a reference template based on the STM32Cube LL API that can be used
to build any firmware application.
- This project is targeted to run on STM32WL33 device on NUCLEO-WL33CC board from STMicroelectronics.  
- The reference template project configures the maximum system clock frequency at 64Mhz.

#### <b>Description</b>

This project LL template provides:

 - Inclusion of all LL drivers (include files in "main.h" and LL sources files in IDE environment, with option "USE_FULL_LL_DRIVER" in IDE environment)
   Note: If optimization is needed afterwards, user can perform a cleanup by removing unused drivers.
 - Definition of LED and buttons (file: main.h)
   Note: Button names printed on board may differ from naming "buttonx" in code: "key button", ...
 - Clock configuration (file: main.c)

This project LL template does not provide:

 - Functions to initialize and control LED and buttons
 - Functions to manage IRQ handler of buttons

To port a LL example to the targeted board:

1. Select the LL example to port.
   To find the board on which LL examples are deployed, refer to LL examples list in "STM32CubeProjectsList.html", table section "Examples_LL"
   or Application Note: STM32Cube firmware examples for STM32WL3 Series

2. Replace source files of the LL template by the ones of the LL example, except code specific to board.
   Note: Code specific to board is specified between tags:

   - /* ==============   BOARD SPECIFIC CONFIGURATION CODE BEGIN    ============== */
   - /* ==============   BOARD SPECIFIC CONFIGURATION CODE END      ============== */


  - Replace file main.h, with updates:
    - Keep LED and buttons definition of the LL template under tags
  - Replace file main.c, with updates:
    - Keep clock configuration of the LL template: function "SystemClock_Config()"
    - Depending of LED availability, replace LEDx_PIN by another LEDx (number) available in file main.h
  - Replace file stm32wl3x_it.h
  - Replace file stm32wl3x_it.c


### <b>Keywords</b>

Reference, Template

### <b>Directory contents</b>

  - Templates_LL/Legacy/Inc/stm32wl3x_it.h         Interrupt handlers header file
  - Templates_LL/Legacy/Inc/main.h                  Header for main.c module
  - Templates_LL/Legacy/Inc/stm32_assert.h          Template file to include assert_failed function
  - Templates_LL/Legacy/Src/stm32wl3x_it.c         Interrupt handlers
  - Templates_LL/Legacy/Src/main.c                  Main program
  - Templates_LL/Legacy/Src/system_stm32wl3x.c     STM32WL3x system source file

### <b>Hardware and Software environment</b>

  - This template runs on STM32WL33 devices.
  - This template has been tested with STMicroelectronics NUCLEO-WL33CC
    board and can be easily tailored to any other supported device
    and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example


:::
:::

