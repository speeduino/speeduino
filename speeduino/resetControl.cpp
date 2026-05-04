#include "resetControl.h"

//This needs to be here because using the config page directly can prevent burning the setting
static uint8_t _resetControl = RESET_CONTROL_DISABLED;
static uint8_t _resetPin;

uint8_t getResetControl(void)
{
    return _resetControl;
}

void __attribute__((optimize("Os"))) initialiseResetControl(statuses &current, uint8_t resetControlMode, uint8_t resetPin)
{
  _resetControl = resetControlMode;
  _resetPin = resetPin;
  current.resetPreventActive = false;

  /* Setup reset control initial state */
  switch (resetControlMode)
  {
    case RESET_CONTROL_PREVENT_WHEN_RUNNING:
      /* Set the reset control pin LOW and change it to HIGH later when we get sync. */
      digitalWrite(resetPin, LOW);
      current.resetPreventActive = false;
      break;
    case RESET_CONTROL_PREVENT_ALWAYS:
      /* Set the reset control pin HIGH and never touch it again. */
      digitalWrite(resetPin, HIGH);
      current.resetPreventActive = true;
      break;
    case RESET_CONTROL_SERIAL_COMMAND:
      /* Set the reset control pin HIGH. There currently isn't any practical difference
         between this and PREVENT_ALWAYS but it doesn't hurt anything to have them separate. */
      digitalWrite(resetPin, HIGH);
      current.resetPreventActive = false;
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

void matchResetControlToEngineState(statuses &current)
{
  if ((current.decoder.getStatus().syncStatus!=SyncStatus::None) && (current.RPM > 0))
  {
    if ( (!current.resetPreventActive) && (getResetControl() == RESET_CONTROL_PREVENT_WHEN_RUNNING) ) 
    {
      //Reset prevention is supposed to be on while the engine is running but isn't. Fix that.
      digitalWrite(_resetPin, HIGH);
      current.resetPreventActive = true;
    }
  } 
  else
  {
    if ( (current.resetPreventActive) && (getResetControl() == RESET_CONTROL_PREVENT_WHEN_RUNNING) )
    {
      digitalWrite(_resetPin, LOW);
      current.resetPreventActive = false;
    }
  }
}