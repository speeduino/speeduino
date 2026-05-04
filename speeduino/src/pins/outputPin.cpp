#include "outputPin.h"
#include "../../atomic.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void outputPin_t::setPin(uint8_t pin, uint8_t mode)
{
    pinMode(pin, mode);
    _pin = pin;
}

/** @brief Set the pin high */
void outputPin_t::setPinHigh(void)
{
   ATOMIC() { digitalWrite(_pin, HIGH); }
}

/** @brief Set the pin low */
void outputPin_t::setPinLow(void)
{
  ATOMIC() { digitalWrite(_pin, LOW); }
}

// LCOV_EXCL_STOP
