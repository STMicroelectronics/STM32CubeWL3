/**
  ******************************************************************************
  * @file      : startup_stm32wl3x.s
  * @author    : GPM WBL Application Team
  * @brief     : STM32WL3x Ultra Low Power Devices vector
  *              This module performs:
  *              - Set the initial SP
  *              - Set the initial PC == Reset_Handler,
  *              - Set the vector table entries with the exceptions ISR address
  *              - Branches to main in the C library (which eventually
  *                calls main()).
  *              After Reset the Cortex-M0+ processor is in Thread mode,
  *              priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m0plus
  .fpu softvfp
  .thumb

.global  __vector_table
.global  Default_Handler

.word  _sidata
.word  _sdata
.word  _edata
.word  _sbss
.word  _ebss

	.section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
   ldr   r0, =_estack
   mov   sp, r0
  bl  SystemInit

  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4

LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss

FillZerobss:
  movs  r3, #0
  str  r3, [r2]
  adds r2, r2, #4


LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

  bl  main

LoopForever:
    b LoopForever


.size  Reset_Handler, .-Reset_Handler

    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

   .section  .intvec,"a",%progbits
  .type  __vector_table, %object
  .size  __vector_table, .-__vector_table


__vector_table:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  0
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler
  .word  FLASH_IRQHandler
  .word  RCC_IRQHandler
  .word  PVD_IRQHandler
  .word I2C1_IRQHandler
  .word I2C2_IRQHandler
  .word SPI1_IRQHandler
  .word 0x00000000
  .word SPI3_IRQHandler
  .word USART1_IRQHandler
  .word LPUART1_IRQHandler
  .word TIM2_IRQHandler
  .word RTC_IRQHandler
  .word ADC_IRQHandler
  .word AES_IRQHandler
  .word 0x00000000
  .word GPIOA_IRQHandler
  .word GPIOB_IRQHandler
  .word DMA_IRQHandler
  .word LPAWUR_IRQHandler
  .word COMP1_IRQHandler
  .word MRSUBG_BUSY_IRQHandler
  .word MRSUBG_IRQHandler
  .word MRSUBG_TX_RX_SEQUENCE_IRQHandler
  .word MRSUBG_TIMER_CPU_WKUP_IRQHandler
  .word MRSUBG_WKUP_IRQHandler
  .word DAC_IRQHandler
  .word TIM16_IRQHandler
  .word LCD_IRQHandler
  .word LCSC_IRQHandler
  .word LCSC_LC_ACTIVITY_IRQHandler
  .word 0x00000000
  .word 0x00000000

   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler

   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler

   .weak      FLASH_IRQHandler
   .thumb_set FLASH_IRQHandler,Default_Handler

   .weak      RCC_IRQHandler
   .thumb_set RCC_IRQHandler,Default_Handler

   .weak      PVD_IRQHandler
   .thumb_set PVD_IRQHandler,Default_Handler

   .weak      I2C1_IRQHandler
   .thumb_set I2C1_IRQHandler,Default_Handler

   .weak      I2C2_IRQHandler
   .thumb_set I2C2_IRQHandler,Default_Handler

   .weak      SPI1_IRQHandler
   .thumb_set SPI1_IRQHandler,Default_Handler

   .weak      SPI3_IRQHandler
   .thumb_set SPI3_IRQHandler,Default_Handler

   .weak      USART1_IRQHandler
   .thumb_set USART1_IRQHandler,Default_Handler

   .weak      LPUART1_IRQHandler
   .thumb_set LPUART1_IRQHandler,Default_Handler

   .weak      TIM2_IRQHandler
   .thumb_set TIM2_IRQHandler,Default_Handler

   .weak      RTC_IRQHandler
   .thumb_set RTC_IRQHandler,Default_Handler

   .weak      ADC_IRQHandler
   .thumb_set ADC_IRQHandler,Default_Handler

   .weak      AES_IRQHandler
   .thumb_set AES_IRQHandler,Default_Handler

   .weak      GPIOA_IRQHandler
   .thumb_set GPIOA_IRQHandler,Default_Handler

   .weak      GPIOB_IRQHandler
   .thumb_set GPIOB_IRQHandler,Default_Handler

   .weak      DMA_IRQHandler
   .thumb_set DMA_IRQHandler,Default_Handler

   .weak      LPAWUR_IRQHandler
   .thumb_set LPAWUR_IRQHandler,Default_Handler

   .weak      COMP1_IRQHandler
   .thumb_set COMP1_IRQHandler,Default_Handler

   .weak      MRSUBG_BUSY_IRQHandler
   .thumb_set MRSUBG_BUSY_IRQHandler,Default_Handler

   .weak      MRSUBG_IRQHandler
   .thumb_set MRSUBG_IRQHandler,Default_Handler

   .weak      MRSUBG_TX_RX_SEQUENCE_IRQHandler
   .thumb_set MRSUBG_TX_RX_SEQUENCE_IRQHandler,Default_Handler

   .weak      MRSUBG_TIMER_CPU_WKUP_IRQHandler
   .thumb_set MRSUBG_TIMER_CPU_WKUP_IRQHandler,Default_Handler

   .weak      MRSUBG_WKUP_IRQHandler
   .thumb_set MRSUBG_WKUP_IRQHandler,Default_Handler

   .weak      DAC_IRQHandler
   .thumb_set DAC_IRQHandler,Default_Handler

   .weak      TIM16_IRQHandler
   .thumb_set TIM16_IRQHandler,Default_Handler

   .weak      LCD_IRQHandler
   .thumb_set LCD_IRQHandler,Default_Handler

   .weak      LCSC_IRQHandler
   .thumb_set LCSC_IRQHandler,Default_Handler

   .weak      LCSC_LC_ACTIVITY_IRQHandler
   .thumb_set LCSC_LC_ACTIVITY_IRQHandler,Default_Handler