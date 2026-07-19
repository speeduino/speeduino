#include "port_pin.h"
#include "../../atomic.h"

port_pin_t::port_pin_t(uint8_t pinNum)
{
    if (pinNum!=NOT_A_PIN)
    {
        _port = portOutputRegister(digitalPinToPort(pinNum));
        _mask = digitalPinToBitMask(pinNum);
    }
}

// LCOV_EXCL_START

/** @brief Set the pin high */
void port_pin_t::setPinHigh(void) noexcept
{
    if (isValid())
    {
        ATOMIC() { *_port |= _mask; }
    }
}

/** @brief Set the pin low */
void port_pin_t::setPinLow(void) noexcept
{
    if (isValid())
    {
        ATOMIC() { *_port &= ~_mask; }
    }
}

// LCOV_EXCL_STOP