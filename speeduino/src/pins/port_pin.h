#pragma once

#include <Arduino.h>

/// @cond
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
/// @endcond

/// @brief A structure to support direct port manipulation
/// @see https://docs.arduino.cc/retired/hacking/software/PortManipulation/ 
struct port_pin_t
{
  port_pin_t() = default;

  /** @brief Construct from a pin number*/
  port_pin_t(uint8_t pinNum);

  /** @brief Is the port register initialised? */
  bool isValid(void) const {
    return _port!=NULL_PORT;
  }

  /** @brief Check if the pin is set high */
  bool isPinHigh(void) const noexcept {
    return isValid() ? (*_port & _mask) != 0 : false;
  }

  /** @brief Check if the pin is set low */
  bool isPinLow(void) const noexcept {
    return !isPinHigh();
  }

  /** @brief Set the pin high */
  void setPinHigh(void) noexcept;

  /** @brief Set the pin low */
  void setPinLow(void) noexcept;

private:

  /** @brief The return type of a "call" to portOutputRegister() */
  using port_register_t = decltype(type_detection_detail::return_type_of(&type_detection_detail::detectPortRegisterType));

  /** @brief The return type of a "call" to digitalPinToBitMask() */
  using pin_mask_t = decltype(type_detection_detail::return_type_of(&type_detection_detail::detectDigitalPinToBitMask));

  static constexpr port_register_t NULL_PORT = {nullptr};

  port_register_t _port = NULL_PORT;
  pin_mask_t _mask = {0};
};

