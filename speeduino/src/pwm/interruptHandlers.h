#pragma once

#include <stdint.h>

using setTimerCallback_t = void(*)(uint16_t tickDelta);

template <typename TPwmChannel>
static inline void pwmISR(TPwmChannel &pwmChannel, setTimerCallback_t setTimerCallback)
{
  if (pwmChannel.isPartialDuty())
  {
    if (pwmChannel.pin.isPinHigh())
    {
      pwmChannel.pin.setPinLow();
      setTimerCallback(pwmChannel.maxDuty - pwmChannel.targetDuty);
    }
    else
    {
      pwmChannel.pin.setPinHigh();
      setTimerCallback(pwmChannel.targetDuty);
    }
  }
}