#include "fastInputPin.h"
#include "../../board_definition.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void fastInputPin_t::setPin(uint8_t pin, uint8_t mode) 
{
    _port_pin = port_pin_t(pin);
    if (pin!=NOT_A_PIN)
    {
        pinMode(pin, mode);
    }
}

// LCOV_EXCL_STOP
