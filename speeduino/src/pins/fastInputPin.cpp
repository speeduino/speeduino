#include "fastInputPin.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void fastInputPin_t::setPin(uint8_t pin, uint8_t mode) 
{
    pinMode(pin, mode);
    _port_pin.port = portInputRegister(digitalPinToPort(pin));
    _port_pin.mask = digitalPinToBitMask(pin);
}

bool fastInputPin_t::isPinHigh(void) const {
    return (*_port_pin.port & _port_pin.mask) != 0;
}

// LCOV_EXCL_STOP
