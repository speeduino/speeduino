#include "pinMapping.h"
#include "../../board_definition.h"
#include "../../preprocessor.h"

uint8_t pinTranslate(uint8_t rawPin)
{
  uint8_t outputPin = rawPin;
  if(rawPin > BOARD_MAX_DIGITAL_PINS) { outputPin = A8 + (outputPin - BOARD_MAX_DIGITAL_PINS - 1); }

  return outputPin;
}

uint8_t pinTranslateAnalog(uint8_t rawPin)
{
    if (rawPin<_countof(ANALOG_PINS))
    {
        return ANALOG_PINS[rawPin];
    }
    return rawPin;
}
