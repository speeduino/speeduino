#include <Arduino.h>
#include "pinNumbers_t.h"

bool __attribute__((optimize("Os"))) sensor_pins_t::isPinUsed(uint8_t pin) const
{
    const uint8_t *pStart = (const uint8_t *)this;
    const uint8_t *pEnd = pStart + sizeof(*this);
    while ((pStart!=pEnd) && (*pStart!=pin))
    {
        ++pStart;
    }
    return pStart!=pEnd;
}
