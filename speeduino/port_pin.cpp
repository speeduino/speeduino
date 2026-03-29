#include "port_pin.h"

void fastInputPin_t::setPin(uint8_t pin, uint8_t mode) 
{
    pinMode(pin, mode);
    _port = portInputRegister(digitalPinToPort(pin));
    _mask = digitalPinToBitMask(pin);
}

bool fastInputPin_t::isPinHigh(void) const {
    return (*_port & _mask) != 0;
}