#include <Arduino.h>
#include "preprocessor.h"
#include "pinNumbers_t.h"

void injector_pins_t::copy_P(const uint8_t *pSrc, uint8_t length, const config2 &page2)
{
    uint8_t elementsToCopy = min(page2.nInjectors, min(_countof(injector_pins_t::pins), length));
    memcpy_P(pins, pSrc, elementsToCopy*sizeof(pins[0]));
    // Fill remainder of array with zeroes
    for (; elementsToCopy<_countof(injector_pins_t::pins); ++elementsToCopy)
    {
        pins[elementsToCopy] = 0U;
    }
}