/* Arduino board:
 *   %(id)s
 *   %(name)s
 *   MCU: %(build.mcu)s
 */
#if %(ifdef_clause)s
#ifdef _DIGITALIO_MATCHED_BOARD
#error "This header's Arduino configuration heuristics have matched multiple boards. The header may be out of date."
#endif
#define _DIGITALIO_MATCHED_BOARD

__attribute__((always_inline))
static inline void pinModeFast(uint8_t pin, uint8_t mode) {
  if(!__builtin_constant_p(pin)) {
    pinMode(pin, mode);
  }
%(pinmode_clause)s
}

__attribute__((always_inline))
static inline void digitalWriteFast(uint8_t pin, uint8_t value) {
  if(!__builtin_constant_p(pin)) {
    digitalWrite(pin, value);
  }
%(digitalwrite_clause)s
}

__attribute__((always_inline))
static inline int digitalReadFast(uint8_t pin) {
  if(!__builtin_constant_p(pin)) {
    return digitalRead(pin);
  }
%(digitalread_clause)s
  return LOW;
}

__attribute__((always_inline))
static inline void noAnalogWrite(uint8_t pin) {
  if(!__builtin_constant_p(pin)) {
    return; // noAnalogWrite is taken care of by digitalWrite() for variables
  }
%(timer_clause)s
}

__attribute__((always_inline))
static inline bool _isPWMPin(uint8_t pin) {
%(ispwm_clause)s
  return false;
}

__attribute__((always_inline))
static inline bool _directionIsAtomic(uint8_t pin) {
%(direction_clause)s
  return false;
}

__attribute__((always_inline))
static inline bool _outputIsAtomic(uint8_t pin) {
%(output_clause)s
  return false;
}

__attribute__((always_inline))
static inline bool _inputIsAtomic(uint8_t pin) {
%(input_clause)s
  return false;
}


#endif
