#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32wl3x.h"
#include "stm32wl3x_hal_def.h"
#include "stm32wl3x_ll_rcc.h"

#define CLOCK_FREQUENCY					(SystemCoreClock/1000)
#define CLOCK_FREQUENCY_MHZ				(CLOCK_FREQUENCY/1000)

void SysTick_Handler(void);
uint32_t TIMER_UTILS_GetTimerValue(void);
void TIMER_UTILS_TimestampReset(void);

void SysTickUserAction(uint32_t counter);

#ifdef __cplusplus
}
#endif

#endif
