::: {.row}
::: {.col-sm-12 .col-lg-4}
## <b>MRSUBG_Timer_RTC Example Description</b>

This demonstration shows how to operate the MRSUBG embedded timer and the RTC concurrently on an STM32WL33CC device, while the STM32 Low Power Manager
controls the low-power transitions.

The application starts three virtual timers on the MRSUBG radio timer and an
independent RTC wake-up timer. Each virtual radio timer is re-armed with a
1.05 s period. The RTC is configured from the LSE clock with a 1 s wake-up
period.

The two sources are deliberately given close periods, so that the four possible
combinations are exercised over a long run: an RTC wake-up alone, a radio timer
wake-up alone, and either source expiring while the device is already awake
because of the other one.

B2 is also configured as an asynchronous wake-up source.

To minimise consumption, the application runs its active phase on the internal
high speed oscillator and enables the external crystal only for the duration of
a transmission.
Thus, On a radio-timer wake-up, the application enables HSE, transmits a sub-1 GHz
packet, increments the last byte of the payload, and then switches HSE off.

Between two wake-ups the device enters **DeepStop** (with Low-speed clock active).

### <b>Keywords</b>

MRSUBG, radio timer, virtual timer, RTC, Low Power Manager, DeepStop, Tiny LPM

### <b>Timer operation</b>

The MRSUBG timer driver virtualizes the radio hardware timer. The application
uses three static timer handles, whose callbacks re-arm their own timers and
set application flags. `HAL_MRSUBG_TIMER_Tick()` is called on every main-loop
iteration to serve expired virtual timers and re-arm the hardware timer.

The callbacks do not run in interrupt context. The radio timer wake-up interrupt
records the event, and the timer driver dispatches callbacks when the main loop
calls `HAL_MRSUBG_TIMER_Tick()`. Application callbacks can therefore restart
timers without running radio or serial operations from interrupt context.

### <b>How the radio timer driver works</b>

The device provides one hardware timer. The driver virtualises it into any
number of virtual timers, kept in a list sorted by expiry time, and always
programs the hardware for the head of that list. Two consequences apply
throughout the example:

  - **Callbacks do not run in interrupt context.** The CPU wake-up interrupt
    only records the expiry. The callbacks are dispatched from the main loop,
    when the application calls the driver tick function. Printing, re-arming a
    timer or starting a transmission from a callback is therefore allowed.
  - **Virtual timers are one-shot.** A periodic timer is obtained by re-arming
    the timer from its own callback, which is what the three callbacks of this
    example do.

The time base is expressed in machine time units (MTU). One MTU is
1 / (16 x 32768) s, approximately 1.9 us, and the 32-bit counter wraps
approximately every 2 hours and 17 minutes. The virtual timer API accepts
microseconds or milliseconds and performs the conversion internally.

### <b>Running the radio timer on the internal oscillator</b>

The active phase of this example runs on the internal high speed oscillator
(HSI), so that the external crystal (HSE) can stay off while no radio activity
is required. The crystal is started only around a transmission and stopped again
immediately afterwards.

This has a direct consequence on the radio timer. The absolute time counter of
the wake-up timer block is an *interpolated* value: the upper bits are counted
from the low speed clock, while the four least significant bits are produced by
the high speed clock. When the crystal is stopped, those four bits stop
changing. As the wake-up comparison uses the complete interpolated value, a
wake-up instant whose four least significant bits are not zero would never be
reached, and the radio timer would never expire.

The example resolves this by declaring the clock tree as accurate to the radio
block even while the device runs on the internal oscillator, which keeps the
least significant bits of the absolute time counter running. The radio timer
therefore continues to expire normally during an active phase that does not use
the crystal.

The trade-off is accuracy. During those phases the interpolation is derived from
the internal oscillator instead of the crystal, so the effective resolution and
the accuracy of the programmed wake-up instant are lower than in a
crystal-based configuration. This is acceptable for the periodic wake-up pattern
of this example, where the crystal is started again before any radio operation,
but an application with tighter timing requirements should keep the crystal
running, or take the reduced accuracy into account when choosing its periods.

### <b>Application flow</b>

```
main()
  HAL_Init
  SystemClock_Config_HSE
  PeriphCommonClock_Config
  MX_GPIO_Init
  MX_MRSUBG_Init
  MX_MRSUBG_TIMER_Init
  MX_RTC_Init
  BSP_COM_Init                 virtual COM port, 115200 8N1
  MX_APPE_Init
    initialize LEDs and B2
    configure B2 as a wake-up source
    configure and start three MRSUBG virtual timers
  SystemClock_Config_HSI
  while (1)
    MX_APPE_Process
      HAL_MRSUBG_TIMER_Tick
      read and clear the internal wake-up flags
      on a MRSUBG timer wake-up: enable HSE, transmit one packet, disable HSE
      on an RTC wake-up: turn LD1 on for 10 ms
      combine application and MRSUBG timer low-power constraints
      enter the selected low-power mode
```

### <b>Low-power behavior</b>

The application does not select the low power mode directly. Two independent
users publish their own constraint to the low power manager:

| User                | Constraint published by                            |
|---------------------|----------------------------------------------------|
| Application         | the application power save level check             |
| MRSUBG timer driver | the driver power save level check                  |

Each user publishes the deepest mode that it can afford at that moment.

**The low power manager then arbitrates and always enters the shallowest mode
among all the requests.** 

An application is therefore free to request a deep
mode; it can never force the device deeper than another user allows.

The selected DeepStop mode keeps the low-speed clock active. The low-power
interface saves and restores the CPU and peripheral context around DeepStop, so
execution resumes after the low-power call when a wake-up source occurs.

The RTC is an independent wake-up source. It is not a Low Power Manager user,
but it remains configured while the device is in DeepStop. The application reads
and clears the internal wake-up flags once on each main-loop iteration, then
handles both the radio-timer and RTC flags when they are present.

### <b>Usage guidelines</b>

<details>
<summary>Click to expand</summary>

The following points apply to any application built on this driver, not only to
this example.

**1. Call the driver tick function on every main loop iteration.**
Not only when a timer wake-up is detected. A wake-up caused by the push-button,
by a radio interrupt or by any other source must still give the driver the
opportunity to serve a timer that expired in the meantime and to re-arm the
hardware for the next one. Omitting it can stall the whole timer chain.

**2. Keep the radio timer wake-up interrupt at a high priority and keep it
short.**
The wake-up path is time critical: if the handler is delayed beyond the next
expiry, that expiry is lost. The virtual timer callbacks must never be executed
from the interrupt handler; they are dispatched by the driver tick function.
When a second source such as the RTC is used, its handler must be kept equally
short, so that it cannot delay the radio timer wake-up.

**3. Do not insert a long task between the power save level check and the low
power entry.**
The driver answers the question "how deep can the device go now". Any slow
operation inserted afterwards, such as a console print, a transmission or a
flash write, invalidates that answer and the device may enter a low power mode
past a wake-up instant that has already elapsed. Perform the work first, then
check, then enter the low power mode.

**4. A running timer cannot be restarted.**
Starting a virtual timer that is still in the list returns an error. Stop it
first. Restarting a timer from its own callback is always safe, because the
timer has already been removed from the list at that point. This example treats
the error as fatal on purpose, so that an incorrect usage is visible immediately
instead of degrading silently.

**5. A virtual timer handle is a list node.**
It must remain valid for as long as the timer runs, so it has to be a static or
global object and never a local variable. The driver stores the pointer, not a
copy of the handle.

**6. Callbacks are executed on expiry only.**
Starting a timer never invokes its callback. A timer armed with a zero delay
still goes through one driver tick.

**7. The timer list always contains one more timer than the application
started.**
The driver maintains an internal anti-wrapping timer. It is not visible to the
application, but it is the reason why the list is never empty and why the device
wakes up periodically even when no application timer is close.

**8. Do not leave the device with no application timer running.**
If the internal timer is the only one left in the list, the driver considers
that the time base is no longer needed, disables it and allows the deepest low
power mode, which retains nothing. Starting a timer afterwards shifts the
cumulative time and produces one spurious immediate expiry. In this example the
three timers re-arm themselves from their callbacks, so the situation does not
occur; an application that stops all of its timers must take it into account.

**9. Wake-up source flags are cleared when they are read.**
The functions that report the wake-up source also clear the flags that they
return. Call each of them once per iteration, store the value and test the bits
on that copy; a second call returns zero.

**10. The radio timer wake-up line must be enabled.**
Without it the timer cannot wake the device from Stop or Standby, and the timers
are only served when another source happens to wake the CPU.

**11. Both wake-up sources must be evaluated on every wake-up.**
The device can be woken by the radio timer while an RTC period is still running,
and the opposite. Each iteration of the main loop therefore has to test both
sources and serve whichever of them is pending, rather than assuming that the
source which caused the wake-up is the only one to handle.

**12. The domain that supplies the RTC must be retained.**
Standby with retention only preserves the domains that were explicitly
requested. If the RTC domain is not among them, the RTC is lost on the first low
power cycle and never wakes the device again.

**13. Do not override the reset handler.**
The wake-up from Standby with retention is detected in C, early in the system
initialisation, so the context switch assembly only provides the context save
and restore routines. The reset handler supplied with the toolchain must keep
running: it arms the main stack pointer limit, which provides the Cortex-M33
hardware stack overflow detection. A private reset handler that omits it
silently disables that protection for the whole application.

**14. The system initialisation must stay shallow on the wake-up path.**
The reset handler, the system initialisation and the wake-up handler run on the
top of the C stack before the context is restored, so they overwrite words that
may belong to a live application frame. Those words are saved and restored by
the context switch module, and the number of words it preserves is a compile
time constant. Adding stack usage to that path without raising that constant
corrupts the resumed frame.

</details>

### <b>Build options</b>

| Define | Effect |
|---|---|
| `CFG_FULL_LOW_POWER` | Enables full low-power configuration when set to `1` |
| `CFG_LPM_SUPPORTED` | Enables Low Power Manager support when set to `1` |
| `NO_DEEPSTOP` | Optional application define that disable transitions to low power |

### <b>Directory contents</b>

  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/app_conf.h                    Application low-power and trace configuration file
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/app_entry.h                   Application interface header
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/main.h                        Main program header and RTC wake-up period
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/stm32_assert.h                Assert description header
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/stm32wl3x_hal_conf.h          HAL configuration header
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/stm32wl3x_it.h                Interrupt handler header
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/stm32wl3x_nucleo_conf.h       BSP configuration header
  - MRSUBG/MRSUBG_Timer_RTC/Core/Inc/utilities_conf.h              STM32 utilities configuration header
  - MRSUBG/MRSUBG_Timer_RTC/Core/Src/app_entry.c                   Application entry point and timer callbacks
  - MRSUBG/MRSUBG_Timer_RTC/Core/Src/main.c                        Main program, radio, timer, and RTC initialization
  - MRSUBG/MRSUBG_Timer_RTC/Core/Src/stm32wl3x_hal_msp.c           HAL MSP module
  - MRSUBG/MRSUBG_Timer_RTC/Core/Src/stm32wl3x_it.c                Interrupt handlers
  - MRSUBG/MRSUBG_Timer_RTC/Core/Src/system_stm32wl3x.c            STM32WL3x system clock configuration
  - MRSUBG/MRSUBG_Timer_RTC/System/Interfaces/stm32_lpm_if.c       Low Power Manager interface implementation
  - MRSUBG/MRSUBG_Timer_RTC/System/Interfaces/stm32_lpm_if.h       Low Power Manager interface header
  - MRSUBG/MRSUBG_Timer_RTC/System/Modules/asm.h                   Assembly macro definitions
  - MRSUBG/MRSUBG_Timer_RTC/System/Modules/osal.c                  OS abstraction layer implementation
  - MRSUBG/MRSUBG_Timer_RTC/System/Modules/osal.h                  OS abstraction layer header
  - MRSUBG/MRSUBG_Timer_RTC/System/Startup/cpu_context_switch.s    CPU context switch implementation
  - MRSUBG/MRSUBG_Timer_RTC/System/Startup/device_context_switch.c Device context switch implementation
  - MRSUBG/MRSUBG_Timer_RTC/System/Startup/device_context_switch.h Device context switch header

### <b>Hardware and Software environment</b>

  - This example runs on STM32WL33CCVx devices

  - This example has been tested with the STMicroelectronics NUCLEO-WL33CCx application board and can be tailored to other supported devices and development boards

  - NUCLEO-WL33CCx setup
    - Connect the board to the host through the ST-LINK USB connector
    - The console output is available on the ST-LINK virtual COM port, configured for 115200 bit/s, 8 data bits, no parity, 1 stop bit, and no flow control
    - LD2 turns on while a radio-timer wake-up is processed and LD1 turns on while an RTC wake-up is processed
    - The user push-button B2, connected to PA11, is configured as an asynchronous wake-up source

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

:::
:::