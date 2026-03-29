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

void fastOutputPin_t::setPin(uint8_t pin, uint8_t mode)
{
    pinMode(pin, mode);
    _port = portOutputRegister(digitalPinToPort(pin));
    _mask = digitalPinToBitMask(pin);
}

/** @brief Set the pin high */
void fastOutputPin_t::setPinHigh(void)
{
    *_port |= _mask;
}

/** @brief Set the pin low */
void fastOutputPin_t::setPinLow(void)
{
    *_port &= ~_mask;
}