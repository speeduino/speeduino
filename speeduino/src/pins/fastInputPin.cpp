#include "fastInputPin.h"
#include "../../board_definition.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void fastInputPin_t::setPin(uint8_t pin, uint8_t mode) 
{
    if (pin!=NOT_A_PIN)
    {
        pinMode(pin, mode);
        _port_pin.port = portInputRegister(digitalPinToPort(pin));
        _port_pin.mask = digitalPinToBitMask(pin);
    }
    else
    {
        _port_pin = port_pin_t();
    }
}

bool fastInputPin_t::isPinHigh(void) const noexcept{
    return isValid() ? (*_port_pin.port & _port_pin.mask) != 0 : false;
}

// LCOV_EXCL_STOP
