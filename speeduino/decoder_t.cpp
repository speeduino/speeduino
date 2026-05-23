#include "decoder_t.h"
#include "src/pins/pinNumbers_t.h"

decoder_t& decoder_t::attachInterrupts(const decoder_pins_t &pins)
{
    primary.attach(pins.primary);
    secondary.attach(pins.secondary);
    tertiary.attach(pins.tertiary);

    return *this;
}