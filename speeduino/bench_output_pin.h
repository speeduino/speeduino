#pragma once
#include <stdint.h>
#include "injector_bench.h"

// Keep normal PWM phase tracking alive while a bench command owns its pin.
// The outer trackedOutputPinAdapter still records logical transitions.
template<class Pin>
class BenchOutputPin : public Pin {
  uint8_t number=255;
public:
  void setPin(uint8_t pin,uint8_t mode) noexcept {number=pin;Pin::setPin(pin,mode);}
  void setPinHigh() noexcept {if(!injectorBenchOwnsPin(number)) Pin::setPinHigh();}
  void setPinLow() noexcept {if(!injectorBenchOwnsPin(number)) Pin::setPinLow();}
};
