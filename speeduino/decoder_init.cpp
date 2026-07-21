#include <Arduino.h>
#include "decoder_init.h"
#include "decoders.h"
#include "decoder_builder.h"
#include "globals.h"
#include "preprocessor.h"
#include "unit_testing.h"

#pragma GCC optimize("Os")

static decoder_t defaultInitFunc(void)
{
  return decoder_builder_t().build();
}

using decoder_init_func_t = decoder_t (*)(void);
static decoder_init_func_t getDecoderInitFunc(uint8_t decoderIndex)
{
  decoder_init_func_t initFunc = defaultInitFunc;

  // If SMALL_FLASH_DECODER is defined, we only compile in the decoder that is selected
  // Modify as needed, but the idea is that for a given build, only 1 decoder is compiled in to save flash space. 
#if !defined(SMALL_FLASH_DECODER) 
  constexpr uint8_t DECODER_COUNT = DECODER_MAX;
#else
  constexpr uint8_t DECODER_COUNT = 1;
  decoderIndex = 0;
#endif

  // This array must be in the same order as the DECODER_ #defines (I.e. DECODER_MISSING_TOOTH etc.)
  // and therefore in the same order as the INI
  static constexpr decoder_init_func_t initialisers[DECODER_COUNT] PROGMEM = {
    // SMALL_FLASH_DECODER should be set to the decoder index of the desired decoder, which corresponds to the DECODER_ constants. 
    // E.g. setting SMALL_FLASH_DECODER to 9 will compile in the Miata 9905 decoder
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==0) 
  &triggerSetup_missingTooth,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==1)
    &triggerSetup_BasicDistributor,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==2)
  &triggerSetup_DualWheel,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==3)
  &triggerSetup_GM7X,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==4)
  &triggerSetup_4G63,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==5)
  &triggerSetup_24X,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==6)
  &triggerSetup_Jeep2000,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==7)
  &triggerSetup_Audi135,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==8)
  &triggerSetup_HondaD17,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==9)
  &triggerSetup_Miata9905,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==10)
  &triggerSetup_MazdaAU,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==11)
  &triggerSetup_non360,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==12)
  &triggerSetup_Nissan360,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==13)
  &triggerSetup_Subaru67,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==14)
  &triggerSetup_Daihatsu,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==15)
  &triggerSetup_Harley,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==16)
  &triggerSetup_ThirtySixMinus222,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==17)
  &triggerSetup_ThirtySixMinus21,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==18)
  &triggerSetup_420a,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==19)
  &triggerSetup_Webber,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==20)
  &triggerSetup_FordST170,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==21)
  &triggerSetup_DRZ400,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==22)
  &triggerSetup_NGC,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==23)
  &triggerSetup_Vmax,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==24)
  &triggerSetup_Renix,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==25)
  &triggerSetup_RoverMEMS,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==26)
  &triggerSetup_SuzukiK6A,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==27)
  &triggerSetup_HondaJ32,
#endif
#if !defined(SMALL_FLASH_DECODER) || (SMALL_FLASH_DECODER==28)
  &triggerSetup_FordTFI,
#endif
  };
  if (decoderIndex<DECODER_COUNT)
  {
    initFunc = (decoder_init_func_t)pgm_read_ptr(&initialisers[decoderIndex]);
  }
  return initFunc;
}

/** Initialise the chosen trigger decoder. */
decoder_t buildDecoder(uint8_t decoderIndex)
{
  decoder_t decoder = getDecoderInitFunc(decoderIndex)();

  // TODO: attach interrupts & init pins within the decoder init func
  pinTrigger = decoder.primary.attach(pinTrigger);
  pinTrigger2 = decoder.secondary.attach(pinTrigger2);
  pinTrigger3 = decoder.tertiary.attach(pinTrigger3);
  
  initDecoderPins(pinTrigger, pinTrigger2, pinTrigger3);

  // Turn off per tooth ignition if the decoder doesn't support it
  configPage2.perToothIgn = configPage2.perToothIgn && decoder.getFeatures().supportsPerToothIgnition;

  return decoder;
}