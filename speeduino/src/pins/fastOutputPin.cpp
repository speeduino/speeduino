#include "fastOutputPin.h"
#include "../../atomic.h"
#include "../../board_definition.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void fastOutputPin_t::setPin(uint8_t pin, uint8_t mode) noexcept
{
    if (pin!=NOT_A_PIN)
    {
        pinMode(pin, mode);
        _port_pin.port = portOutputRegister(digitalPinToPort(pin));
        _port_pin.mask = digitalPinToBitMask(pin);
    }
}

/** @brief Set the pin high */
void fastOutputPin_t::setPinHigh(void) noexcept
{
    if (isValid())
    {
        ATOMIC() { *_port_pin.port |= _port_pin.mask; }
    }
}

/** @brief Set the pin low */
void fastOutputPin_t::setPinLow(void) noexcept
{
    if (isValid())
    {
        ATOMIC() { *_port_pin.port &= ~_port_pin.mask; }
    }
}

// LCOV_EXCL_STOP
