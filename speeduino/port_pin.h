#pragma once

#include <Arduino.h>

// Automatic type deduction 
namespace type_detection_detail {
  // These are used for automatic type detection *at compile time* and are never executed.

  // Get the return type of a function call
  template <typename R, typename... Args> R return_type_of(R(*)(Args...));

  // portOutputRegister() and digitalPinToBitMask() on AVR is a GCC return expression, which needs to
  // be wrapped in a function to be used in this context.
  static inline auto detectPortRegisterType(void) { return portOutputRegister(digitalPinToPort(0)); };
  static inline auto detectDigitalPinToBitMask(void) { return digitalPinToBitMask(0); };
}

/** @brief The return type of a "call" to portOutputRegister() */
using port_register_t = decltype(type_detection_detail::return_type_of(&type_detection_detail::detectPortRegisterType));

/** @brief The return type of a "call" to digitalPinToBitMask() */
using pin_mask_t = decltype(type_detection_detail::return_type_of(&type_detection_detail::detectDigitalPinToBitMask));

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
  port_register_t _port = {0};
  pin_mask_t _mask = {0};
};

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
  port_register_t _port = {0};
  pin_mask_t _mask = {0};
};