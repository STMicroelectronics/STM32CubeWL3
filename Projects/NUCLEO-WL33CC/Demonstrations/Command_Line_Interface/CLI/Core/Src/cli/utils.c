#include <stdint.h>
#include "utils.h"

IrqQueue s_xIrqQueue = { .cIndexPush = 0, .cIndexPop = 0, .cIrqQueueSize = 0 };

uint32_t BufferToUint32(uint8_t* ucBuffer)
{
  uint32_t nTmp = 0;

  for(int8_t cI=3; cI>=0; cI--)
    nTmp |= (uint8_t)((uint32_t)ucBuffer[3-cI])<<(8*cI);

  return nTmp;
}
