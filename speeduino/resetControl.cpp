#include "globals.h"

void __attribute__((optimize("Os"))) setResetControlPinState(void)
{
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