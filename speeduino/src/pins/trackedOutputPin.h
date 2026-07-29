#pragma once

#include "../../atomic.h"

/**
 * @brief An output pin that tracks it's state (HIGH/LOW)
 * 
 * This is impossible to do across all platforms and boards using digitalRead():
 * 1. A pin can't be both 'INPUT' and 'OUTPUT` concurrently
 * 2. Swapping pin modes resets pin state on some platforms 
 */
template <class TPin>
class trackedOutputPin_t
{
public:

    /** @brief Set the input pin */
    void setPin(uint8_t pin, uint8_t mode) noexcept {
        _pin.setPin(pin, mode);
        _pinState = LOW;
    }

    /** @brief Is the pin set? */
    bool isValid(void) const {
        return _pin.isValid();
    }

    /** @brief Check if the pin is set high */
    bool isPinHigh(void) const noexcept {
        return _pinState==HIGH;
    }

    /** @brief Check if the pin is set low */
    bool isPinLow(void) const noexcept {
        return !isPinHigh();
    }

    /** @brief Set the pin high */
    void setPinHigh(void) noexcept {
        ATOMIC() {
            _pin.setPinHigh();
            _pinState = HIGH;
        }
    }

    /** @brief Set the pin low */
    void setPinLow(void) noexcept {
        ATOMIC() {
            _pin.setPinLow();
            _pinState = LOW;
        }
    }

private:
    TPin _pin;
    bool _pinState = LOW;
};