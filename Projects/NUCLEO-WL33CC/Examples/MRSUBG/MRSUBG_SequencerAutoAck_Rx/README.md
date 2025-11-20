::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_SequencerAutoACK_Rx Example Description</b>

This example demonstrates how to use the STM32WL3 MRSUBG sequencer feature to automatically transmit and receive packet acknowledgements (ACKs).

The TX has two sequencer ActionConfigurations: WaitMsg and SendACK. When a valid message is received, the sequencer automatically transitions from the WaitMsg state to the SendACK state. After the ACK is transmitted, an interrupt is triggered and the CPU resets the sequencer back into the WaitMsg state.

The RX also has two sequencer ActionConfigurations: SendMsg and WaitACK. After reset, the STM32WL3 first transmits a message containing a counter value.   

When the message transmission is done, the sequencer automatically transitions from the SendMsg state to the WaitACK state, where this device awaits an ACK from the other device.

Once that is received, an interrupt is triggered and the CPU resets the sequencer back into the SendMsg state (after a short 50ms delay).

TX and RX will "ping-pong" messages and ACKs back and forth until any message / ACK is lost, at which point the exchange will stop.

### <b>Keywords</b>

MRSUBG, radio, sequencer

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Inc/stm32wl3x_nucleo_conf.h   BSP configuration file
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Inc/stm32wl3x_hal_conf.h      HAL Configuration file
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Inc/stm32wl3x_it.h            Header for stm32wl3x_it.c
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Inc/main.h                    Header file for main.c
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Inc/stm32_assert.h            Assert description file
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Src/system_stm32wl3x.c        STM32WL3x system clock configuration file
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Src/stm32wl3x_it.c            Interrupt handlers
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Src/stm32wl3x_hal_msp.c       HAL MSP module
  - MRSUBG/MRSUBG_SequencerAutoACK_Rx/Src/main.c                    Main program

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices.
  - This example has been tested with STMicroelectronics NUCLEO-WL33CC1 application board and can be easily tailored to any other supported device and development board.  

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain and load the workspace
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::