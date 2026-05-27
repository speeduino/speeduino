#include <Arduino.h>
#include "pinNumbers_t.h"

void __attribute__((optimize("Os"))) injector_pins_t::clampToNumInjectors(const config2 &page2)
{
    for (uint8_t i=page2.nInjectors; i<INJ_CHANNELS; ++i)
    {
        this->operator[](i) = NOT_A_PIN;
    }
}

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
