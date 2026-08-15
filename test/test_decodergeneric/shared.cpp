#include "decoder_init.h"
#include "shared.h"

void configureStateForPrimaryTrigger(uint8_t decoder, decoder_status_t &status)
{
    extern volatile uint8_t toothSystemCount;
    extern volatile unsigned long toothLastToothRisingTime;
    extern volatile unsigned long toothLastSecToothRisingTime;
    extern volatile unsigned long toothLastToothTime;
    extern volatile unsigned long toothSystemLastToothTime;
    extern volatile uint16_t toothCurrentCount;
    extern volatile unsigned long triggerFilterTime;
    
    if (decoder==DECODER_24X) {
        toothCurrentCount = 0U;
    } else if (decoder==DECODER_JEEP2000) {
        toothCurrentCount = 0U;
    } else if (decoder==DECODER_AUDI135) {
        toothSystemCount = 2U;
        toothSystemLastToothTime = micros() - triggerFilterTime;
        status.syncStatus = SyncStatus::Full;
    } else if (decoder==DECODER_RENIX) {
        toothLastToothRisingTime = micros() - triggerFilterTime;
        toothLastSecToothRisingTime = toothLastToothRisingTime - triggerFilterTime;
    } else if (decoder==DECODER_ROVERMEMS) {
        toothLastToothTime = micros() - triggerFilterTime;
    }
}

void configurePinState(boardInputPin_t &p, uint8_t edge)
{
  if (edge == RISING)
  {
    if (p.isPinHigh())
    {
      p._pin.setPinLow();
    }
    p._pin.setPinHigh();
  }
  else if (edge == FALLING)
  {
    if (p.isPinLow())
    {
      p._pin.setPinHigh();
    }
    p._pin.setPinLow();
  }
  else if (edge == CHANGE)
  {
    if (p.isPinLow())
    {
      p._pin.setPinHigh();
    }
    else
    {
      p._pin.setPinLow();
    }
  }
}
