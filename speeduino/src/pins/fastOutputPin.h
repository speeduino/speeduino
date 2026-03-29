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
  void setPin(uint8_t pin, uint8_t mode = OUTPUT);

  /** @brief Set the pin high */
  void setPinHigh(void);

  /** @brief Set the pin low */
  void setPinLow(void);

private:
  port_pin_t _port_pin;
};