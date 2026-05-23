#include <Arduino.h>
#include "pinNumbers_t.h"

void injector_pins_t::copy_P(const uint8_t *pSrc, uint8_t length, const config2 &page2)
{
    pin_array_t<INJ_CHANNELS>::copy_P(pSrc, min(page2.nInjectors, length));
}

bool sensor_pins_t::isPinUsed(uint8_t pin) const
{
    const uint8_t *pStart = (const uint8_t *)this;
    const uint8_t *pEnd = pStart + sizeof(*this);
    while ((pStart!=pEnd) && (*pStart!=pin))
    {
        ++pStart;
    }
    return pStart!=pEnd;
}
