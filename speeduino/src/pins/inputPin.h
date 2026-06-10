#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "../../board_definition.h"

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
  void setPin(uint8_t pin, uint8_t mode = INPUT);

  /** @brief Check if the pin is set high */
  bool isPinHigh(void) const;

  /** @brief Check if the pin is set low */
  bool isPinLow(void) const
  {
    return !isPinHigh();
  }

  /** @brief Is the pin set? */
  bool isValid(void) const
  {
    return _pin != NOT_A_PIN;
  }

private:
  uint8_t _pin = NOT_A_PIN;
};
