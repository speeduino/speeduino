#include <Arduino.h>
#include "decoder_init.h"
#include "decoders.h"
#include "decoder_builder.h"
#include "globals.h"

static decoder_t globalDecoder = decoder_builder_t().build();
static void setDecoder(const decoder_t &newDecoder)
{
  globalDecoder = newDecoder;
}
const decoder_t& getDecoder(void)
{
  return globalDecoder;
}

static decoder_t buildDecoder(uint8_t decoderType)
{
  switch (decoderType)
  {
    case DECODER_MISSING_TOOTH:
      return triggerSetup_missingTooth();
      break;

    case DECODER_BASIC_DISTRIBUTOR:
      return triggerSetup_BasicDistributor();
      break;

    case DECODER_DUAL_WHEEL:
      return triggerSetup_DualWheel();
      break;

    case DECODER_GM7X:
      return triggerSetup_GM7X();
      break;

    case DECODER_4G63:
      return triggerSetup_4G63();
      break;

    case DECODER_24X:
      return triggerSetup_24X();
      break;

    case DECODER_JEEP2000:
      return triggerSetup_Jeep2000();
      break;

    case DECODER_AUDI135:
      return triggerSetup_Audi135();
      break;

    case DECODER_HONDA_D17:
      return triggerSetup_HondaD17();
      break;

    case DECODER_HONDA_J32:
      return triggerSetup_HondaJ32();
      break;

    case DECODER_MIATA_9905:
      return triggerSetup_Miata9905();
      break;

    case DECODER_MAZDA_AU:
      return triggerSetup_MazdaAU();
      break;

    case DECODER_NON360:
      return triggerSetup_non360();
      break;

    case DECODER_NISSAN_360:
      return triggerSetup_Nissan360();
      break;

    case DECODER_SUBARU_67:
      return triggerSetup_Subaru67();
      break;

    case DECODER_DAIHATSU_PLUS1:
      return triggerSetup_Daihatsu();
      break;

    case DECODER_HARLEY:
      return triggerSetup_Harley();
      break;

    case DECODER_36_2_2_2:
      return triggerSetup_ThirtySixMinus222();
      break;

    case DECODER_36_2_1:
      return triggerSetup_ThirtySixMinus21();
      break;

    case DECODER_420A:
      return triggerSetup_420a();
      break;

    case DECODER_WEBER:
      //Weber-Marelli
      return triggerSetup_Webber();
      break;

    case DECODER_ST170:
      //Ford ST170
      return triggerSetup_FordST170();
      break;
	  
    case DECODER_DRZ400:
      return triggerSetup_DRZ400();
      break;

    case DECODER_NGC:
      //Chrysler NGC - 4, 6 and 8 cylinder
      return triggerSetup_NGC();
      break;

    case DECODER_VMAX:
      return triggerSetup_Vmax();
      break;

    case DECODER_RENIX:
      //Renault 44 tooth decoder
      return triggerSetup_Renix();
      break;

    case DECODER_ROVERMEMS:
      //Rover MEMs - covers multiple flywheel trigger combinations.
      return triggerSetup_RoverMEMS();
      break;   

    case DECODER_SUZUKI_K6A:
      return triggerSetup_SuzukiK6A();
      break;

    case DECODER_FORD_TFI:
      // Ford TFI
      return triggerSetup_FordTFI();
      break;

    default:
      break;
  }
  return decoder_builder_t().build();
}

/** Initialise the chosen trigger decoder. */
void __attribute__((optimize("Os"))) setDecoder(uint8_t decoderType)
{
  //Set the trigger function based on the decoder in the config
  setDecoder(buildDecoder(decoderType));

  getDecoder().primary.attach(pinTrigger);
  getDecoder().secondary.attach(pinTrigger2);
  getDecoder().tertiary.attach(pinTrigger3);
  
  initDecoderPins(pinTrigger, pinTrigger2, pinTrigger3);
}
