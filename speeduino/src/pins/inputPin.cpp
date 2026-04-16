#include "inputPin.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void inputPin_t::setPin(uint8_t pin, uint8_t mode) 
{
    pinMode(pin, mode);
    _pin = pin;
}

bool inputPin_t::isPinHigh(void) const {
    return digitalRead(_pin) == HIGH;
}

// LCOV_EXCL_STOP
