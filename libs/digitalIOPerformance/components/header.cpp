/*
 *
 * Header for high performance Arduino Digital I/O
 *
 * Automatically generated from the Arduino library setup (boards.txt & pins_arduino.h)
 *
 * See the accompanying file README.md for documentation.
 *
 * ****
 *
 * This header is a derived work of the Arduino microcontroller libraries, which are
 * licensed under LGPL. Although as a header file it is not bound by the same usage
 * clauses as the library itself (see "3. Object Code Incorporating Material from
 * Library Header Files.)"
 *
 * Note that although the code generated functions below here look horrific,
 * they're written to inline only very small subsets of themselves at compile
 * time (they generate single port-register instructions when the parameters
 * are constant.)
 *
 *
 */

#ifdef __AVR__
#ifndef _DIGITALIO_PERFORMANCE
#define _DIGITALIO_PERFORMANCE

#include "Arduino.h"
#include <util/atomic.h>

// Forward declarations for per-Arduino-board functions:
inline static void pinModeFast(uint8_t pin, uint8_t mode);
inline static void digitalWriteFast(uint8_t pin, uint8_t value);
inline static int digitalReadFast(uint8_t pin);
inline static void noAnalogWrite(uint8_t pin);

// These few per-board functions are designed for internal use, but
// you can call them yourself if you want.
inline static bool _isPWMPin(uint8_t pin);
inline static bool _directionIsAtomic(uint8_t pin);
inline static bool _outputIsAtomic(uint8_t pin);
inline static bool _inputIsAtomic(uint8_t pin);

#ifdef DIGITALIO_NO_INTERRUPT_SAFETY
#define DIGITALIO_NO_INTERRUPT_SAFETY 1
#else
#define DIGITALIO_NO_INTERRUPT_SAFETY 0
#endif

#ifdef DIGITALIO_NO_MIX_ANALOGWRITE
#define DIGITALIO_NO_MIX_ANALOGWRITE 1
#else
#define DIGITALIO_NO_MIX_ANALOGWRITE 0
#endif

// All the variables & conditionals in these functions should evaluate at
// compile time not run time...

__attribute__((always_inline))
static inline void pinModeSafe(uint8_t pin, uint8_t mode) {
  if(!__builtin_constant_p(pin)) {
    pinMode(pin, mode);
  }
  else {
    if((mode == INPUT || mode == INPUT_PULLUP) && !DIGITALIO_NO_MIX_ANALOGWRITE)
      noAnalogWrite(pin);

    const bool write_is_atomic = DIGITALIO_NO_INTERRUPT_SAFETY
      || (__builtin_constant_p(mode)
          && mode == OUTPUT
          && _directionIsAtomic(pin));
    if(write_is_atomic) {
      pinModeFast(pin, mode);
    }
    else {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        pinModeFast(pin, mode);
      }
    }
  }
}

__attribute__((always_inline))
static inline void digitalWriteSafe(uint8_t pin, uint8_t value) {
  if(!__builtin_constant_p(pin)) {
    digitalWrite(pin, value);
  }
  else {
    if(!DIGITALIO_NO_MIX_ANALOGWRITE)
      noAnalogWrite(pin);

    if(DIGITALIO_NO_INTERRUPT_SAFETY || _outputIsAtomic(pin)) {
      digitalWriteFast(pin, value);
    }
    else {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        digitalWriteFast(pin, value);
      }
    }
  }
}

__attribute__((always_inline))
static inline int digitalReadSafe(uint8_t pin) {
  if(!__builtin_constant_p(pin)) {
    return digitalRead(pin);
  }
  else {
    if(!DIGITALIO_NO_MIX_ANALOGWRITE)
      noAnalogWrite(pin);
    return digitalReadFast(pin);
  }
}

