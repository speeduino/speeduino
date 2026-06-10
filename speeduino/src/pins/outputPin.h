#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "../../board_definition.h"

/** 
 * @brief A class for output pin operations. Must have same signature as fastOutputPin_t to allow for interchangeable use.
 * 
 * Call setPin() to initialize the pin, then call setPinHigh() to set the pin high.
 */
class outputPin_t 
{
public:
  /** @brief Set the output pin */
  void setPin(uint8_t pin, uint8_t mode = OUTPUT);

  /** @brief Set the pin high */
  void setPinHigh(void);

  /** @brief Set the pin low */
  void setPinLow(void);
  
  /** @brief Is the pin set? */
  bool isValid(void) const
  {
    return _pin != NOT_A_PIN;
  }

private:
  uint8_t _pin = NOT_A_PIN;
};