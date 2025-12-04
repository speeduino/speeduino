#include <Arduino.h>
#include "decoder_init.h"
#include "decoders.h"
#include "decoder_builder.h"
#include "globals.h"
#include "preprocessor.h"

static decoder_t globalDecoder = decoder_builder_t().build();
static void setDecoder(const decoder_t &newDecoder)
{
  globalDecoder = newDecoder;
}
const decoder_t& getDecoder(void)
{
  return globalDecoder;
}

static decoder_t buildDecoder(uint8_t decoder)
{
  // This array must be in the same order as the DECODER_ #defines (I.e. DECODER_MISSING_TOOTH etc.)
  // and therefore in the same order as the INI
  using decoder_init_func_t = decoder_t (*)(void);
  static constexpr decoder_init_func_t initialisers[] PROGMEM = {
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
  static_assert(size_t(DECODER_MAX)==_countof(initialisers), "Decoder initializer array mismatch");
  if (decoder<_countof(initialisers))
  {
    return ((decoder_init_func_t)pgm_read_ptr(&initialisers[decoder]))();
  }
  return decoder_builder_t().build();
}

/** Initialise the chosen trigger decoder. */
void setDecoder(uint8_t decoderType)
{
  //Set the trigger function based on the decoder in the config
  setDecoder(buildDecoder(decoderType));

  getDecoder().primary.attach(pinTrigger);
  getDecoder().secondary.attach(pinTrigger2);
  getDecoder().tertiary.attach(pinTrigger3);
  
  initDecoderPins(pinTrigger, pinTrigger2, pinTrigger3);
}
