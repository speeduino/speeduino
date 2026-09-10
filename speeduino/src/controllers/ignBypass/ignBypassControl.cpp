#include "ignBypassControl.h"
#include "ignBypassControl_detail.h"
#include "src/pins/pinNumbers_t.h"
#include "unit_testing.h"
#include "globals.h"

TESTABLE_STATIC ignBypassController::details::state_t _state;

void initialiseIgnBypass(const statuses &current, const config4 &page4, const pinNumbers_t &pins)
{
    _state.ignBypassPin = outputPin_t();
    _state.rotationStatus = current.rotationStatus;
    if(page4.ignBypassEnabled && !pinIsOutput(pins.pinIgnBypass, pins)) 
    { 
        _state.ignBypassPin.setPin(pins.pinIgnBypass);
    }
}

void ignBypassControl(const statuses &current)
{
    if (_state.ignBypassPin.isValid() && (current.rotationStatus!=_state.rotationStatus))
    {
        if (current.rotationStatus!=EngineRotationStatus::Running)
        {
            // Reset the ignition bypass
            _state.ignBypassPin.setPinLow();
        }
        else
        {
            // Engine is running, so we can enable the ignition bypass
            _state.ignBypassPin.setPinHigh();
        }
        _state.rotationStatus = current.rotationStatus;
    }
}