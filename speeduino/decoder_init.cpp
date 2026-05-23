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
  decoder_init_func_t initFunc = defaultInitFunc;

#if !defined(SMALL_FLASH_DECODER) 
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
    initFunc = (decoder_init_func_t)pgm_read_ptr(&initialisers[decoderIndex]);
  }
// If SMALL_FLASH_DECODER is defined, we only compile in the decoder that is selected
// Modify as needed, but the idea is that for a given build, only 1 decoder is compiled in to save flash space. 
//
// SMALL_FLASH_DECODER should be set to the decoder index of the desired decoder, which corresponds to the DECODER_ constants. 
// E.g. setting SMALL_FLASH_DECODER to 9 will compile in the Miata 9905 decoder
#elif SMALL_FLASH_DECODER==0 
  return &triggerSetup_missingTooth;
#elif SMALL_FLASH_DECODER==1
    return &triggerSetup_BasicDistributor;
#elif SMALL_FLASH_DECODER==2
  return &triggerSetup_DualWheel;
#elif SMALL_FLASH_DECODER==3
  return &triggerSetup_GM7X;
#elif SMALL_FLASH_DECODER==4
  return &triggerSetup_4G63;
#elif SMALL_FLASH_DECODER==5
  return &triggerSetup_24X;
#elif SMALL_FLASH_DECODER==6
  return &triggerSetup_Jeep2000;
#elif SMALL_FLASH_DECODER==7
  return &triggerSetup_Audi135;
#elif SMALL_FLASH_DECODER==8
  return &triggerSetup_HondaD17;
#elif SMALL_FLASH_DECODER==9
  return &triggerSetup_Miata9905;
#elif SMALL_FLASH_DECODER==10
  return &triggerSetup_MazdaAU;
#elif SMALL_FLASH_DECODER==11
  return &triggerSetup_non360;
#elif SMALL_FLASH_DECODER==12
  return &triggerSetup_Nissan360;
#elif SMALL_FLASH_DECODER==13
  return &triggerSetup_Subaru67;
#elif SMALL_FLASH_DECODER==14
  return &triggerSetup_Daihatsu;
#elif SMALL_FLASH_DECODER==15
  return &triggerSetup_Harley;
#elif SMALL_FLASH_DECODER==16
  return &triggerSetup_ThirtySixMinus21;
#elif SMALL_FLASH_DECODER==17
  return &triggerSetup_420a;
#elif SMALL_FLASH_DECODER==18
  return &triggerSetup_Webber;
#elif SMALL_FLASH_DECODER==19
  return &triggerSetup_FordST170;
#elif SMALL_FLASH_DECODER==20
  return &triggerSetup_DRZ400;
#elif SMALL_FLASH_DECODER==21
  return &triggerSetup_NGC;
#elif SMALL_FLASH_DECODER==22
  return &triggerSetup_Vmax;
#elif SMALL_FLASH_DECODER==23
  return &triggerSetup_Renix;
#elif SMALL_FLASH_DECODER==24
  return &triggerSetup_RoverMEMS;
#elif SMALL_FLASH_DECODER==25
  return &triggerSetup_SuzukiK6A;
#elif SMALL_FLASH_DECODER==26
  return &triggerSetup_HondaJ32;
#elif SMALL_FLASH_DECODER==27
  return &triggerSetup_FordTFI;
#endif
  return initFunc;
}

/** Initialise the chosen trigger decoder. */
decoder_t buildDecoder(uint8_t decoderIndex)
{
  // TODO: attach interrupts & init pins within the decoder init func
  decoder_t decoder = getDecoderInitFunc(decoderIndex)().attachInterrupts(pinNumbers.triggerPins);
  
  initDecoderPins(pinNumbers.triggerPins);

  // Turn off per tooth ignition if the decoder doesn't support it
  configPage2.perToothIgn = configPage2.perToothIgn && decoder.getFeatures().supportsPerToothIgnition;

  return decoder;
}