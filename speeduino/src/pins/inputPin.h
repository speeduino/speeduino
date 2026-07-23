#pragma once
#include "digitalPin.h"

/** 
 * @brief A class for input pin operations. 
 * 
 * Call setPin() to initialize the pin, then call isPinHigh() to check if the pin is set high.
 * 
 * @note Must have same signature as fastInputPin_t to allow for interchangeable use.
 */
class inputPin_t 
{
public:
  /** @brief Set the input pin */
  void setPin(uint8_t pin, uint8_t mode = INPUT) noexcept {
    _pin.setPin(pin, mode);
  }

  /** @brief Check if the pin is set high */
  bool isPinHigh(void) const noexcept {
    return _pin.isPinHigh();
  }

  /** @brief Check if the pin is set low */
  bool isPinLow(void) const noexcept {
    return _pin.isPinLow();
  }

  /** @brief Is the pin set? */
  bool isValid(void) const {
    return _pin.isValid();
  }

#if !defined(UNIT_TEST)
private:
#endif
  digitalPin_t _pin;
};
