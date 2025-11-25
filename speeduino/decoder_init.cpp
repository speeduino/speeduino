#include <Arduino.h>
#include "decoder_init.h"
#include "decoders.h"
#include "decoder_builder.h"
#include "globals.h"

static decoder_t triggerFuncs = decoder_builder_t().build();
const decoder_t& getDecoder(void)
{
  return triggerFuncs;
}

static uint8_t getPriTriggerEdge(const config4 &page4)
{
  return page4.TrigEdge == 0U ? RISING : FALLING;
}
static uint8_t getSecTriggerEdge(const config4 &page4)
{
  return page4.TrigEdgeSec == 0U ? RISING : FALLING;
}
static uint8_t getTerTriggerEdge(const config10 &page10)
{
  return page10.TrigEdgeThrd == 0U ? RISING : FALLING;
}

static decoder_t getDecoder(uint8_t decoderType)
{
  switch (decoderType)
  {
    case DECODER_MISSING_TOOTH:
      //Missing tooth decoder
      triggerSetup_missingTooth();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_missingTooth, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_missingTooth, getDecoderFeatures().hasSecondary ? getSecTriggerEdge(configPage4) : TRIGGER_EDGE_NONE)
                      .setTertiaryTrigger(triggerThird_missingTooth, configPage10.vvt2Enabled > 0 ? getTerTriggerEdge(configPage10) : TRIGGER_EDGE_NONE)
                      .setGetRPM(getRPM_missingTooth)
                      .setGetCrankAngle(getCrankAngle_missingTooth)
                      .setSetEndTeeth(triggerSetEndTeeth_missingTooth)
                      .build();
      break;

    case DECODER_BASIC_DISTRIBUTOR:
      // Basic distributor
      triggerSetup_BasicDistributor();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_BasicDistributor, getPriTriggerEdge(configPage4))
                      .setGetRPM(getRPM_BasicDistributor)
                      .setGetCrankAngle(getCrankAngle_BasicDistributor)
                      .setSetEndTeeth(triggerSetEndTeeth_BasicDistributor)
                      .build();
      break;

    case 2:
      triggerSetup_DualWheel();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_DualWheel, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_DualWheel, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_DualWheel)
                      .setGetCrankAngle(getCrankAngle_DualWheel)
                      .setSetEndTeeth(triggerSetEndTeeth_DualWheel)
                      .build();
      break;

    case DECODER_GM7X:
      triggerSetup_GM7X();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_GM7X, getPriTriggerEdge(configPage4))
                      .setGetRPM(getRPM_GM7X)
                      .setGetCrankAngle(getCrankAngle_GM7X)
                      .setSetEndTeeth(triggerSetEndTeeth_GM7X)
                      .build();
      break;

    case DECODER_4G63:
      triggerSetup_4G63();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_4G63, CHANGE)
                      .setSecondaryTrigger(triggerSec_4G63, FALLING)
                      .setGetRPM(getRPM_4G63)
                      .setGetCrankAngle(getCrankAngle_4G63)
                      .setSetEndTeeth(triggerSetEndTeeth_4G63)
                      .build();
      break;

    case DECODER_24X:
      triggerSetup_24X();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_24X, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_24X, CHANGE)
                      .setGetRPM(getRPM_24X)
                      .setGetCrankAngle(getCrankAngle_24X)
                      .setSetEndTeeth(triggerSetEndTeeth_24X)
                      .build();
      break;

    case DECODER_JEEP2000:
      triggerSetup_Jeep2000();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Jeep2000, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_Jeep2000, CHANGE)
                      .setGetRPM(getRPM_Jeep2000)
                      .setGetCrankAngle(getCrankAngle_Jeep2000)
                      .setSetEndTeeth(triggerSetEndTeeth_Jeep2000)
                      .build();
      break;

    case DECODER_AUDI135:
      triggerSetup_Audi135();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Audi135, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_Audi135, RISING)
                      .setGetRPM(getRPM_Audi135)
                      .setGetCrankAngle(getCrankAngle_Audi135)
                      .setSetEndTeeth(triggerSetEndTeeth_Audi135)
                      .build();
      break;

    case DECODER_HONDA_D17:
      triggerSetup_HondaD17();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_HondaD17, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_HondaD17, CHANGE)
                      .setGetRPM(getRPM_HondaD17)
                      .setGetCrankAngle(getCrankAngle_HondaD17)
                      .setSetEndTeeth(triggerSetEndTeeth_HondaD17)
                      .build();
      break;

    case DECODER_HONDA_J32:
      triggerSetup_HondaJ32();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_HondaJ32, RISING) // Don't honor the config, always use rising edge
                      .setSecondaryTrigger(triggerSec_HondaJ32, RISING)
                      .setGetRPM(getRPM_HondaJ32)
                      .setGetCrankAngle(getCrankAngle_HondaJ32)
                      .setSetEndTeeth(triggerSetEndTeeth_HondaJ32)
                      .build();
      break;

    case DECODER_MIATA_9905:
      triggerSetup_Miata9905();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Miata9905, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_Miata9905, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_Miata9905)
                      .setGetCrankAngle(getCrankAngle_Miata9905)
                      .setSetEndTeeth(triggerSetEndTeeth_Miata9905)
                      .build();
      break;

    case DECODER_MAZDA_AU:
      triggerSetup_MazdaAU();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_MazdaAU, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_MazdaAU, FALLING)
                      .setGetRPM(getRPM_MazdaAU)
                      .setGetCrankAngle(getCrankAngle_MazdaAU)
                      .setSetEndTeeth(triggerSetEndTeeth_MazdaAU)
                      .build();
      break;

    case DECODER_NON360:
      triggerSetup_non360();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_DualWheel, getPriTriggerEdge(configPage4)) //Is identical to the dual wheel decoder, so that is used. Same goes for the secondary below
                      .setSecondaryTrigger(triggerSec_DualWheel, FALLING) //Note the use of the Dual Wheel trigger function here. No point in having the same code in twice.
                      .setGetRPM(getRPM_non360)
                      .setGetCrankAngle(getCrankAngle_non360)
                      .setSetEndTeeth(triggerSetEndTeeth_non360)
                      .build();
      break;

    case DECODER_NISSAN_360:
      triggerSetup_Nissan360();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Nissan360, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_Nissan360, CHANGE)
                      .setGetRPM(getRPM_Nissan360)
                      .setGetCrankAngle(getCrankAngle_Nissan360)
                      .setSetEndTeeth(triggerSetEndTeeth_Nissan360)
                      .build();
      break;

    case DECODER_SUBARU_67:
      triggerSetup_Subaru67();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Subaru67, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_Subaru67, FALLING)
                      .setGetRPM(getRPM_Subaru67)
                      .setGetCrankAngle(getCrankAngle_Subaru67)
                      .setSetEndTeeth(triggerSetEndTeeth_Subaru67)
                      .build();
      break;

    case DECODER_DAIHATSU_PLUS1:
      triggerSetup_Daihatsu();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Daihatsu, getPriTriggerEdge(configPage4))
                      .setGetRPM(getRPM_Daihatsu)
                      .setGetCrankAngle(getCrankAngle_Daihatsu)
                      .setSetEndTeeth(triggerSetEndTeeth_Daihatsu)
                      .build();
      break;

    case DECODER_HARLEY:
      triggerSetup_Harley();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Harley, RISING)
      //                .setSecondaryTrigger(triggerSec_Harley)
                      .setGetRPM(getRPM_Harley)
                      .setGetCrankAngle(getCrankAngle_Harley)
                      .setSetEndTeeth(triggerSetEndTeeth_Harley)
                      .build();
      break;

    case DECODER_36_2_2_2:
      //36-2-2-2
      triggerSetup_ThirtySixMinus222();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_ThirtySixMinus222, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_ThirtySixMinus222, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_ThirtySixMinus222)
                      .setGetCrankAngle(getCrankAngle_missingTooth) //This uses the same function as the missing tooth decoder, so no need to duplicate code
                      .setSetEndTeeth(triggerSetEndTeeth_ThirtySixMinus222)
                      .build();
      break;

    case DECODER_36_2_1:
      //36-2-1
      triggerSetup_ThirtySixMinus21();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_ThirtySixMinus21, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_missingTooth, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_ThirtySixMinus21)
                      .setGetCrankAngle(getCrankAngle_missingTooth) //This uses the same function as the missing tooth decoder, so no need to duplicate code
                      .setSetEndTeeth(triggerSetEndTeeth_ThirtySixMinus21)
                      .build();
      break;

    case DECODER_420A:
      //DSM 420a
      triggerSetup_420a();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_420a, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_420a, FALLING) //Always falling edge
                      .setGetRPM(getRPM_420a)
                      .setGetCrankAngle(getCrankAngle_420a)
                      .setSetEndTeeth(triggerSetEndTeeth_420a)
                      .build();
      break;

    case DECODER_WEBER:
      //Weber-Marelli
      triggerSetup_DualWheel();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Webber, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_Webber, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_DualWheel)
                      .setGetCrankAngle(getCrankAngle_DualWheel)
                      .setSetEndTeeth(triggerSetEndTeeth_DualWheel)
                      .build();
      break;

    case DECODER_ST170:
      //Ford ST170
      triggerSetup_FordST170();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_missingTooth, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_FordST170, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_FordST170)
                      .setGetCrankAngle(getCrankAngle_FordST170)
                      .setSetEndTeeth(triggerSetEndTeeth_FordST170)
                      .build();
      break;
	  
    case DECODER_DRZ400:
      triggerSetup_DRZ400();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_DualWheel, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_DRZ400, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_DualWheel)
                      .setGetCrankAngle(getCrankAngle_DualWheel)
                      .setSetEndTeeth(triggerSetEndTeeth_DualWheel)
                      .build();
      break;

    case DECODER_NGC:
      //Chrysler NGC - 4, 6 and 8 cylinder
      triggerSetup_NGC();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_NGC, CHANGE)
                      .setSecondaryTrigger( configPage2.nCylinders == 4U ? triggerSec_NGC4 : triggerSec_NGC68,
                                            configPage2.nCylinders == 4U ? CHANGE : FALLING)
                      .setGetRPM(getRPM_NGC)
                      .setGetCrankAngle(getCrankAngle_missingTooth)
                      .setSetEndTeeth(triggerSetEndTeeth_NGC)
                      .build();
      break;

    case DECODER_VMAX:
      triggerSetup_Vmax();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Vmax, CHANGE)
                      .setGetRPM(getRPM_Vmax)
                      .setGetCrankAngle(getCrankAngle_Vmax)
                      .setSetEndTeeth(triggerSetEndTeeth_Vmax)
                      .build();
      break;

    case DECODER_RENIX:
      //Renault 44 tooth decoder
      triggerSetup_Renix();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_Renix, getPriTriggerEdge(configPage4))
                      .setGetRPM(getRPM_missingTooth)
                      .setGetCrankAngle(getCrankAngle_missingTooth)
                      .setSetEndTeeth(triggerSetEndTeeth_Renix)
                      .build();
      break;

    case DECODER_ROVERMEMS:
      //Rover MEMs - covers multiple flywheel trigger combinations.
      triggerSetup_RoverMEMS();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_RoverMEMS, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_RoverMEMS, getSecTriggerEdge(configPage4)) 
                      .setGetRPM(getRPM_RoverMEMS)
                      .setSetEndTeeth(triggerSetEndTeeth_RoverMEMS)
                      .setGetCrankAngle(getCrankAngle_missingTooth)   
                      .build();
      break;   

    case DECODER_SUZUKI_K6A:
      triggerSetup_SuzukiK6A();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_SuzukiK6A, getPriTriggerEdge(configPage4)) // only primary, no secondary, trigger pattern is over 720 degrees
                      .setGetRPM(getRPM_SuzukiK6A)
                      .setGetCrankAngle(getCrankAngle_SuzukiK6A)
                      .setSetEndTeeth(triggerSetEndTeeth_SuzukiK6A)
                      .build();
      break;

    case DECODER_FORD_TFI:
      // Ford TFI
      triggerSetup_FordTFI();
      return decoder_builder_t()
                      .setPrimaryTrigger(triggerPri_FordTFI, getPriTriggerEdge(configPage4))
                      .setSecondaryTrigger(triggerSec_FordTFI, getSecTriggerEdge(configPage4))
                      .setGetRPM(getRPM_FordTFI)
                      .setGetCrankAngle(getCrankAngle_FordTFI)
                      .setSetEndTeeth(triggerSetEndTeeth_FordTFI)
                      .build();
      break;

    default:
      break;
  }
  return decoder_builder_t().build();
}

/** Initialise the chosen trigger decoder.
 * - Set Interrupt numbers @ref triggerInterrupt, @ref triggerInterrupt2 and @ref triggerInterrupt3  by pin their numbers (based on board CORE_* define)
 * - Call decoder specific setup function triggerSetup_*() (by @ref config4.TrigPattern, set to one of the DECODER_* defines) and do any additional initialisations needed.
 * 
 * @todo Explain why triggerSetup_*() alone cannot do all the setup, but there's ~10+ lines worth of extra init for each of decoders.
 */
void setDecoder(uint8_t decoderType)
{
  //Set the trigger function based on the decoder in the config
  triggerFuncs = getDecoder(decoderType);

  getDecoder().primary.attach(pinTrigger);
  getDecoder().secondary.attach(pinTrigger2);
  getDecoder().tertiary.attach(pinTrigger3);
  
  initDecoderPins(pinTrigger, pinTrigger2, pinTrigger3);
}
