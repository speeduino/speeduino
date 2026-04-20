#include "resetControl.h"
#include "unit_testing.h"

static ResetControlMode _resetControl = ResetControlMode::Disabled;
static uint8_t _resetPin;
static bool _resetPreventActive = false;

ResetControlMode getResetControlMode(void)
{
    return _resetControl;
}

bool isResetPreventActive(void)
{
  return _resetPreventActive;
}

void __attribute__((optimize("Os"))) initialiseResetControl(ResetControlMode resetControlMode, uint8_t resetPin)
{
  _resetControl = resetControlMode;
  _resetPin = resetPin;
  _resetPreventActive = false;

  /* Setup reset control initial state */
  switch (resetControlMode)
  {
    case ResetControlMode::PreventWhenRunning:
      /* Set the reset control pin LOW and change it to HIGH later when we get sync. */
      digitalWrite(resetPin, LOW);
      _resetPreventActive = false;
      break;
    case ResetControlMode::PreventAlways:
      /* Set the reset control pin HIGH and never touch it again. */
      digitalWrite(resetPin, HIGH);
      _resetPreventActive = true;
      break;
    case ResetControlMode::SerialCommand:
      /* Set the reset control pin HIGH. There currently isn't any practical difference
         between this and PREVENT_ALWAYS but it doesn't hurt anything to have them separate. */
      digitalWrite(resetPin, HIGH);
      _resetPreventActive = false;
      break;
    default:
      // Do nothing - keep MISRA happy
      break;
  }

  /* Reset control is a special case. If reset control is enabled, it needs its initial state set BEFORE its pinMode.
     If that doesn't happen and reset control is in "Serial Command" mode, the Arduino will end up in a reset loop
     because the control pin will go low as soon as the pinMode is set to OUTPUT. */
  pinMode(resetPin, OUTPUT);
}

void matchResetControlToEngineState(const statuses &current)
{
  if (getResetControlMode() == ResetControlMode::PreventWhenRunning)
  {
    // TODO: consolidate this check with status.isEngineRunning, which is based on the same conditions? 
    if ((current.decoder.getStatus().syncStatus!=SyncStatus::None) && (current.RPM > 0))
    {
      if ( (!_resetPreventActive)) 
      {
        //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
        digitalWrite(_resetPin, HIGH);
        _resetPreventActive = true;
      }
    } 
    else
    {
      if (_resetPreventActive)
      {
        digitalWrite(_resetPin, LOW);
        _resetPreventActive = false;
      }
    }
  }
}