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
  // We are assuming the engine is not running at the point of initialisation, so reset prevent 
  // should only be active if the mode is "Prevent Always"
  _resetPreventActive = resetControlMode == ResetControlMode::PreventAlways;
  digitalWrite(resetPin, resetControlMode == ResetControlMode::PreventWhenRunning ? LOW : HIGH);

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
    _resetPreventActive = (current.decoder.getStatus().syncStatus!=SyncStatus::None) && (current.RPM > 0);
    digitalWrite(_resetPin, _resetPreventActive ? HIGH : LOW);
  }
}