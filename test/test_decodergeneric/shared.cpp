#include "decoder_init.h"
#include "shared.h"
#include "src/decoders/decoder_state.h"

extern decoders::detail::state_t _decoderState;

void configureStateForPrimaryTrigger(uint8_t decoder, decoder_status_t &status)
{
    if (decoder==DECODER_24X) {
        _decoderState.toothCurrentCount = 0U;
    } else if (decoder==DECODER_JEEP2000) {
        _decoderState.toothCurrentCount = 0U;
    } else if (decoder==DECODER_AUDI135) {
        _decoderState.toothSystemCount = 2U;
        _decoderState.toothSystemLastToothTime = micros() - _decoderState.triggerFilterTime;
        status.syncStatus = SyncStatus::Full;
    } else if (decoder==DECODER_RENIX) {
        _decoderState.toothLastToothRisingTime = micros() - _decoderState.triggerFilterTime;
        _decoderState.toothLastSecToothRisingTime = _decoderState.toothLastToothRisingTime - _decoderState.triggerFilterTime;
    } else if (decoder==DECODER_ROVERMEMS) {
        _decoderState.toothLastToothTime = micros() - _decoderState.triggerFilterTime;
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
