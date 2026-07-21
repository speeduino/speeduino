#include <Arduino.h>
#include "decoder_t.h"

#pragma GCC optimize("Os")

// Just in case
static_assert(TRIGGER_EDGE_NONE != LOW, "LOW edge value conflict");
static_assert(TRIGGER_EDGE_NONE != HIGH, "HIGH edge value conflict");
static_assert(TRIGGER_EDGE_NONE != RISING, "RISING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != FALLING, "FALLING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != CHANGE, "CHANGE edge value conflict");

uint8_t interrupt_t::attach(uint8_t pin) const
{
    detach(pin);
    if (isValid() && (pin!=NOT_A_PIN))
    {
        attachInterrupt(digitalPinToInterrupt(pin), callback, edge);
        return pin;
    }
    return NOT_A_PIN;
}  

/** @brief Detach the interrupt from a pin */
void interrupt_t::detach(uint8_t pin) const
{
    detachInterrupt( digitalPinToInterrupt(pin) );
}  
