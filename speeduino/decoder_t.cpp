#include <Arduino.h>
#include "decoder_t.h"

#pragma GCC optimize("Os")

// Just in case
static_assert(TRIGGER_EDGE_NONE != LOW, "LOW edge value conflict");
static_assert(TRIGGER_EDGE_NONE != HIGH, "HIGH edge value conflict");
static_assert(TRIGGER_EDGE_NONE != RISING, "RISING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != FALLING, "FALLING edge value conflict");
static_assert(TRIGGER_EDGE_NONE != CHANGE, "CHANGE edge value conflict");

uint8_t interrupt_t::attach(uint8_t pin)
{
    detach(pin);

    _pin.setPin(pin);
    if (isValid())
    {
        attachInterrupt(digitalPinToInterrupt(pin), callback, edge);
        return pin;
    }
    return NOT_A_PIN;
}  

/** @brief Detach the interrupt from a pin */
void interrupt_t::detach(uint8_t pin)
{
    detachInterrupt( digitalPinToInterrupt(pin) );
    _pin.setPin(NOT_A_PIN);
}  

bool interrupt_t::isTriggered(void) const
{
    return isValid()
    && (
           (edge==CHANGE)
        || (edge==FALLING && !isPinHigh())
        || (edge==RISING && isPinHigh())
    );
}