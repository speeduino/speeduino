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

#ifndef PLATFORM_X86
/** @brief The return type of a "call" to portOutputRegister() */
using PORT_TYPE = decltype(type_detection_detail::return_type_of(&type_detection_detail::detectPortRegisterType));

/** @brief The return type of a "call" to digitalPinToBitMask() */
using PINMASK_TYPE = decltype(type_detection_detail::return_type_of(&type_detection_detail::detectDigitalPinToBitMask));

#endif