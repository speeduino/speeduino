#include "globals.h"

//This needs to be here because using the config page directly can prevent burning the setting
static uint8_t _resetControl = RESET_CONTROL_DISABLED;

uint8_t getResetControl(void)
{
    return _resetControl;
}

void __attribute__((optimize("Os"))) setResetControlPinState(uint8_t resetControl)
{
  _resetControl = resetControl;
  currentStatus.resetPreventActive = false;

  /* Setup reset control initial state */
  switch (resetControl)
  {
    case RESET_CONTROL_PREVENT_WHEN_RUNNING:
      /* Set the reset control pin LOW and change it to HIGH later when we get sync. */
      digitalWrite(pinResetControl, LOW);
      currentStatus.resetPreventActive = false;
      break;
    case RESET_CONTROL_PREVENT_ALWAYS:
      /* Set the reset control pin HIGH and never touch it again. */
      digitalWrite(pinResetControl, HIGH);
      currentStatus.resetPreventActive = true;
      break;
    case RESET_CONTROL_SERIAL_COMMAND:
      /* Set the reset control pin HIGH. There currently isn't any practical difference
         between this and PREVENT_ALWAYS but it doesn't hurt anything to have them separate. */
      digitalWrite(pinResetControl, HIGH);
      currentStatus.resetPreventActive = false;
      break;
    default:
      // Do nothing - keep MISRA happy
      break;
  }
}