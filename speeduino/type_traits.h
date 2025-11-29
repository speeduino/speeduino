/**
 * @file
 * @brief Minimal polyfill for the standard library type traits, since it isn't
 * available with avr-gcc.
 */
#pragma once

namespace type_traits {

  // std::remove_reference. See https://en.cppreference.com/w/cpp/types/remove_reference.html
  template <typename T>
  struct remove_reference {
    using type = T;
  };
  template <typename T>
  struct remove_reference<T&> {
    using type = T;
  };
  template <typename T>
  struct remove_reference<T&&> {
    using type = T;
  };

}
