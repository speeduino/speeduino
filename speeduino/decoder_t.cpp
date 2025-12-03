#include <Arduino.h>
#include "decoder_t.h"

#pragma GCC optimize ("Os")

// Just in case
static_assert(TRIGGER_EDGE_NONE != LOW, "LOW edge value conflict");
static_assert(TRIGGER_EDGE_NONE != HIGH, "HIGH edge value conflict");
static_assert(TRIGGER_EDGE_NONE != RISING, "RISING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != FALLING, "FALLING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != CHANGE, "CHANGE edge value conflict");

void interrupt_t::attach(uint8_t pin) const
{
  detach(pin);
  if (isValid())
  {
    attachInterrupt(digitalPinToInterrupt(pin), callback, edge);
  }
}
void interrupt_t::detach(uint8_t pin) const
{
  detachInterrupt( digitalPinToInterrupt(pin) );
}
