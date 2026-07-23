#pragma once
#include "port_pin.h"

/** 
 * @brief A class for output pin operations that is faster than standard Arduino digitalWrite() 
 * 
 * Call setPin() to initialize the pin, then call setPinHigh() to set the pin high.
 */
class fastOutputPin_t 
{
public:
  /** @brief Set the output pin */
  void setPin(uint8_t pin, uint8_t mode = OUTPUT) noexcept {
    _pin = port_pin_t(pin, mode);
  }

  /** @brief Set the pin high */
  void setPinHigh(void) noexcept {
    _pin.setPinHigh();
  }

  /** @brief Set the pin low */
  void setPinLow(void) noexcept {
    _pin.setPinLow();
  }

  /** @brief Is the pin set? */
  bool isValid(void) const noexcept {
    return _pin.isValid();
  }

#if !defined(UNIT_TEST)
private:
#endif
  port_pin_t _pin;
};