#pragma once
#include "digitalPin.h"
/** 
 * @brief A class for output pin operations. Must have same signature as fastOutputPin_t to allow for interchangeable use.
 * 
 * Call setPin() to initialize the pin, then call setPinHigh() to set the pin high.
 */
class outputPin_t 
{
public:
  /** @brief Set the output pin */
  void setPin(uint8_t pin, uint8_t mode = OUTPUT) noexcept {
    _pin.setPin(pin, mode);
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
  bool isValid(void) const noexcept
  {
    return _pin.isValid();
  }

#if !defined(UNIT_TEST)
private:
#endif
  digitalPin_t _pin;
};