#include "digitalPin.h"
#include "../../atomic.h"

// LCOV_EXCL_START
// Exclude low level pin manipulation from coverage as it's not testable in a meaningful way

void digitalPin_t::setPin(uint8_t pin, uint8_t mode) noexcept 
{
    _pin = pin;
    if (_pin!=NOT_A_PIN)
    {
        pinMode(_pin, mode);
    }
}

/** @brief Set the pin high */
void digitalPin_t::setPinHigh(void) noexcept
{
  if (isValid())
  {
#if defined(UNIT_TEST)
    _pinState = HIGH;
#endif    
    ATOMIC() { digitalWrite(_pin, HIGH); }
  }
}

/** @brief Set the pin low */
void digitalPin_t::setPinLow(void) noexcept
{
  if (isValid())
  {
#if defined(UNIT_TEST)
    _pinState = LOW;
#endif    
    ATOMIC() { digitalWrite(_pin, LOW); }
  }
}

bool digitalPin_t::isPinHigh(void) const noexcept {
    return isValid() ? 
#if defined(UNIT_TEST)
    _pinState
#else
    digitalRead(_pin) == HIGH 
#endif
    : false;
}

// LCOV_EXCL_STOP
