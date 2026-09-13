#pragma once

#include "../../atomic.h"

/**
 * @brief An output pin that, optionally, can be have it's state inverted.
 * 
 * Convention is that setting the pin high is the "on" state. This class inverts that
 * convention.
 */
template <class TPin>
class invertableOutputPinAdaper_t : public TPin
{
public:

    /** @brief Set the inversion behavior */
    void setInverted(bool isInverted) {
        _isInverted = isInverted;
    }

    /** @brief Set the pin high */
    void setPinHigh(void) noexcept {
        if (_isInverted) {
            TPin::setPinLow();
        } else {
            TPin::setPinHigh();
        }
    }

    /** @brief Set the pin low */
    void setPinLow(void) noexcept {
        if (_isInverted) {
            TPin::setPinHigh();
        } else {
            TPin::setPinLow();
        }
    }

private:
    bool _isInverted = false;
};