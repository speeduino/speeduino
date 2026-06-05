#pragma once
#include "port_pin.h"

/** 
 * @brief A class for input pin operations that is faster than standard Arduino digitalRead() 
 * 
 * Call setPin() to initialize the pin, then call isPinHigh() to check if the pin is set high.
 */
class fastInputPin_t 
{
public:
  /** @brief Set the input pin */
  void setPin(uint8_t pin, uint8_t mode = INPUT);

  /** @brief Check if the pin is set high */
  bool isPinHigh(void) const;

  /** @brief Check if the pin is set low */
  bool isPinLow(void) const
  {
    return !isPinHigh();
  }

private:
  port_pin_t _port_pin;
};
