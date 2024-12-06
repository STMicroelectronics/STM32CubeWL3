#include "timer_utils.h"

volatile uint32_t lSystickCounter = 0;

__weak void SysTickUserAction(uint32_t counter) { }

/**
* @brief  This function handles SysTick Handler.
* @param  None
* @retval None
*/
void SysTick_Handler(void)
{
  lSystickCounter++;

  SysTickUserAction(lSystickCounter);
}

uint32_t TIMER_UTILS_GetTimerValue(void)
{
  SysTick->CTRL;
  uint32_t reload, ticks;

  do {
    reload = lSystickCounter;
    ticks = SysTick->VAL;
  } while (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk);

  return (reload*1000+(SysTick->LOAD-ticks)/CLOCK_FREQUENCY_MHZ);
}

void TIMER_UTILS_TimestampReset(void)
{
  lSystickCounter = 0;
  SysTick->VAL = 0;
}
