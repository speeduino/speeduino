#include <Arduino.h>
#include "decoder_init.h"
#include "decoders.h"
#include "decoder_builder.h"
#include "globals.h"
#include "preprocessor.h"
#include "unit_testing.h"

static decoder_t defaultInitFunc(void)
{
  return decoder_builder_t().build();
}

using decoder_init_func_t = decoder_t (*)(void);
static decoder_init_func_t getDecoderInitFunc(uint8_t decoderIndex)
{
  // This array must be in the same order as the DECODER_ #defines (I.e. DECODER_MISSING_TOOTH etc.)
  // and therefore in the same order as the INI
  static constexpr decoder_init_func_t initialisers[DECODER_MAX] PROGMEM = {
    triggerSetup_missingTooth,
    triggerSetup_BasicDistributor,
    triggerSetup_DualWheel,
    triggerSetup_GM7X,
    triggerSetup_4G63,
    triggerSetup_24X,
    triggerSetup_Jeep2000,
    triggerSetup_Audi135,
    triggerSetup_HondaD17,
    triggerSetup_Miata9905,
    triggerSetup_MazdaAU,
    triggerSetup_non360,
    triggerSetup_Nissan360,
    triggerSetup_Subaru67,
    triggerSetup_Daihatsu,
    triggerSetup_Harley,
    triggerSetup_ThirtySixMinus222,
    triggerSetup_ThirtySixMinus21,
    triggerSetup_420a,
    triggerSetup_Webber,
    triggerSetup_FordST170,
    triggerSetup_DRZ400,
    triggerSetup_NGC,
    triggerSetup_Vmax,
    triggerSetup_Renix,
    triggerSetup_RoverMEMS,
    triggerSetup_SuzukiK6A,
    triggerSetup_HondaJ32,
    triggerSetup_FordTFI,
  };
  if (decoderIndex<DECODER_MAX)
  {
    return ((decoder_init_func_t)pgm_read_ptr(&initialisers[decoderIndex]));
  }
  return &defaultInitFunc;
}

/** Initialise the chosen trigger decoder. */
decoder_t buildDecoder(uint8_t decoderIndex)
{
  decoder_t decoder = getDecoderInitFunc(decoderIndex)();

  decoder.primary.attach(pinTrigger);
  decoder.secondary.attach(pinTrigger2);
  decoder.tertiary.attach(pinTrigger3);
  
  initDecoderPins(pinTrigger, pinTrigger2, pinTrigger3);

  // Turn off per tooth ignition if the decoder doesn't support it
  configPage2.perToothIgn = configPage2.perToothIgn && decoder.getFeatures().supportsPerToothIgnition;

  return decoder;
}