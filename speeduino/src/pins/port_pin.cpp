#include "port_pin.h"
#include "../../atomic.h"

port_pin_t::port_pin_t(uint8_t pinNum, uint8_t mode)
{
    if (pinNum!=NOT_A_PIN)
    {
        _port = portOutputRegister(digitalPinToPort(pinNum));
        _mask = digitalPinToBitMask(pinNum);
        pinMode(pinNum, mode);
    }
}

// LCOV_EXCL_START

/** @brief Set the pin high */
void port_pin_t::setPinHigh(void) noexcept
{
    if (isValid())
    {
        ATOMIC() { *_port |= _mask; }
#if defined(UNIT_TEST)
        _pinState = HIGH;
#endif
    }
}

/** @brief Set the pin low */
void port_pin_t::setPinLow(void) noexcept
{
    if (isValid())
    {
        ATOMIC() { *_port &= ~_mask; }
#if defined(UNIT_TEST)
        _pinState = LOW;
#endif
    }
}

// LCOV_EXCL_STOP