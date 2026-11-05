/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/** @file
 * 
 * Crank and Cam decoders
 * 
 * This file contains the various crank and cam wheel decoder functions.
 * Each decoder must have the following 4 functions (Where xxxx is the decoder name):
 * 
 * - **triggerSetup_xxxx** - Called once from within setup() and configures any required variables
 * - **triggerPri_xxxx** - Called each time the primary (No. 1) crank/cam signal is triggered (Called as an interrupt, so variables must be declared volatile)
 * - **triggerSec_xxxx** - Called each time the secondary (No. 2) crank/cam signal is triggered (Called as an interrupt, so variables must be declared volatile)
 * - **getRevolutionTime_xxxx** - Returns the current crank revolution time, as calculated by the decoder (RPM is derived from this), or 0 if the engine speed is unknown. If a new measurement can't be taken (E.g. no tooth history yet), the current revolution time can be returned to keep the last known speed.
 * - **getCrankAngle_xxxx** - Returns the current crank angle, as calculated by the decoder
 * - **getCamAngle_xxxx** - Returns the current CAM angle, as calculated by the decoder
 *
 * Each decoder must utilise at least the following variables:
 * 
 * - _decoderState.toothLastToothTime - The time (In uS) that the last primary tooth was 'seen'
 */

/* Notes on Doxygen Groups/Modules documentation style:
 * - Installing doxygen (e.g. Ubuntu) via pkg mgr: sudo apt-get install doxygen graphviz
 * - @defgroup tag name/description becomes the short name on (Doxygen) "Modules" page
 * - Relying on JAVADOC_AUTOBRIEF (in Doxyfile, essentially automatic @brief), the first sentence (ending with period) becomes
 *   the longer description (second column following name) on (Doxygen) "Modules" page (old Desc: ... could be this sentence)
 * - All the content after first sentence (like old Note:...) is visible on the page linked from the name (1st col) on "Modules" page
 * - To group all decoders together add 1) @defgroup dec Decoders (on top) and 2) "@ingroup dec" to each decoder (under @defgroup)
 * - To compare Speeduino Doxyfile to default config, do: `doxygen -g Doxyfile.default ; diff Doxyfile.default Doxyfile`
 */
#include <limits.h>
#include "globals.h"
#include "decoders.h"
#include "elapsed_time.h"
#include "scheduler.h"
#include "crankMaths.h"
#include "timers.h"
#include "unit_testing.h"
#include "atomic.h"
#include "decoder_init.h"
#include "decoder_builder.h"
#include "scheduledIO_ign.h"
#include "src/pins/boardInputPin.h"
#include "scheduler_ignition_controller.h"
#include "src/decoders/shared/crank_angle_calculator.h"
#include "src/decoders/shared/decoder_state.h"
#include "src/decoders/shared/perToothIgnition.h"
#include "src/decoders/shared/rev_time_calcs.h"
#include "src/decoders/missingTooth.h"
#include "src/decoders/dualWheel.h"
#include "src/decoders/basicDistributor.h"
#include "src/decoders/gm7x.h"
#include "src/decoders/4g63.h"
#include "src/decoders/gm24x.h"
#include "src/decoders/jeep2000.h"
#include "src/decoders/audi135.h"
#include "src/decoders/hondad17.h"
#include "src/decoders/hondaj32.h"
#include "src/decoders/miata9905.h"
#include "src/decoders/mazdaau.h"
#include "src/decoders/non360.h"
#include "src/decoders/nissan360.h"
#include "src/decoders/subaru6_7.h"
#include "src/decoders/daihatsu.h"
#include "src/decoders/harley.h"
#include "src/decoders/36minus222.h"
#include "src/decoders/36minus21.h"
#include "src/decoders/420a.h"
#include "src/decoders/webber.h"
#include "src/decoders/st170.h"
#include "src/decoders/drz400.h"
#include "src/decoders/ngc.h"
#include "src/decoders/vmax.h"
#include "src/decoders/renix.h"
#include "src/decoders/rovermems.h"
#include "src/decoders/k6a.h"
#include "src/decoders/fordtfi.h"

using namespace decoders::shared;

TESTABLE_STATIC state_t _decoderState;

#define TOOTH_CRANK 0
#define TOOTH_CAM_SECONDARY 1
#define TOOTH_CAM_TERTIARY  2

// note the sequence of these defines which reference the bits used in a byte has moved when the third trigger & engine cycle was incorporated
constexpr uint8_t COMPOSITE_LOG_PRI = 0;
constexpr uint8_t COMPOSITE_LOG_SEC = 1;
constexpr uint8_t COMPOSITE_LOG_THIRD = 2;
constexpr uint8_t COMPOSITE_LOG_TRIG = 3;
constexpr uint8_t COMPOSITE_LOG_SYNC = 4;
constexpr uint8_t COMPOSITE_ENGINE_CYCLE = 5;

/** Universal (shared between decoders) decoder routines. */
// whichTooth - 0 for Primary (Crank), 1 for Secondary (Cam)

/** Add tooth log entry to toothHistory (array).
 * Enabled by (either) currentStatus.toothLogEnabled and currentStatus.compositeTriggerUsed.
 * @param toothTime - Tooth Time
 * @param whichTooth - 0 for Primary (Crank), 2 for Secondary (Cam) 3 for Tertiary (Cam)
 */
static inline void addToothLogEntry(unsigned long toothTime, byte whichTooth)
{
  // cppcheck-suppress misra-c2012-14.4 ; False positive - volatile is messing up the check
  if(currentStatus.isToothLog1Full) { return; }
  //High speed tooth logging history
  if( (currentStatus.toothLogEnabled == true) || (currentStatus.compositeTriggerUsed > 0) ) 
  {
    bool valueLogged = false;
    if(currentStatus.toothLogEnabled == true)
    {
      //Tooth log only works on the Crank tooth
      if(whichTooth == TOOTH_CRANK)
      { 
        toothHistory[toothHistoryIndex] = toothTime; //Set the value in the log. 
        valueLogged = true;
      } 
    }
    else if(currentStatus.compositeTriggerUsed > 0)
    {
      compositeLogHistory[toothHistoryIndex] = 0;
      if(currentStatus.compositeTriggerUsed == 4)
      {
        // we want to display both cams so swap the values round to display primary as cam1 and secondary as cam2, include the crank in the data as the third output
        if(currentStatus.decoder.secondary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_PRI); }
        if(currentStatus.decoder.tertiary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_SEC); }
        if(currentStatus.decoder.primary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_THIRD); }
        if(whichTooth > TOOTH_CAM_SECONDARY) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_TRIG); }
      }
      else
      {
        // we want to display crank and one of the cams
        if(currentStatus.decoder.primary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_PRI); }
        if(currentStatus.compositeTriggerUsed == 3)
        { 
          // display cam2 and also log data for cam 1
          if(currentStatus.decoder.tertiary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_SEC); } // only the COMPOSITE_LOG_SEC value is visualised hence the swapping of the data
          if(currentStatus.decoder.secondary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_THIRD); } 
        } 
        else
        { 
          // display cam1 and also log data for cam 2 - this is the historic composite view
          if(currentStatus.decoder.secondary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_SEC); } 
          if(currentStatus.decoder.tertiary.isPinHigh()) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_THIRD); }
        }
        if(whichTooth > TOOTH_CRANK) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_TRIG); }
      }  
      if(_decoderState.decoderStatus.syncStatus==SyncStatus::Full) { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_LOG_SYNC); }

      if(_decoderState.revolutionOne == 1)
      { BIT_SET(compositeLogHistory[toothHistoryIndex], COMPOSITE_ENGINE_CYCLE);}
      else
      { BIT_CLEAR(compositeLogHistory[toothHistoryIndex], COMPOSITE_ENGINE_CYCLE);}

      toothHistory[toothHistoryIndex] = micros();
      valueLogged = true;
    }

    //If there has been a value logged above, update the indexes
    if(valueLogged == true)
    {
      currentStatus.isToothLog1Full = toothHistoryIndex >= (_countof(toothHistory)-1);
      if (!currentStatus.isToothLog1Full) { ++toothHistoryIndex; }
    }


  } //Tooth/Composite log enabled
}

/** Interrupt handler for primary trigger.
* This function is called on both the rising and falling edges of the primary trigger, when either the 
* composite or tooth loggers are turned on. 
*/
void loggerPrimaryISR(void)
{
  _decoderState.decoderStatus.validTrigger = false; //This value will be set to the return value of the decoder function, indicating whether or not this pulse passed the filters
  bool validEdge = false; //This is set true below if the edge 
  /* 
  Need to still call the standard decoder trigger. 
  Two checks here:
  1) If the primary trigger is RISING, then check whether the primary is currently HIGH
  2) If the primary trigger is FALLING, then check whether the primary is currently LOW
  If either of these are true, the primary decoder function is called
  */
  if(currentStatus.decoder.primary.isTriggered())
  {
    currentStatus.decoder.primary.callback();
    validEdge = true;
  }
  if( (currentStatus.toothLogEnabled == true) && (_decoderState.decoderStatus.validTrigger) )
  {
    //Tooth logger only logs when the edge was correct
    if(validEdge == true) 
    { 
      addToothLogEntry(_decoderState.curGap, TOOTH_CRANK);
    }
  }
  else if( (currentStatus.compositeTriggerUsed > 0) )
  {
    //Composite logger adds an entry regardless of which edge it was
    addToothLogEntry(_decoderState.curGap, TOOTH_CRANK);
  }
}

/** Interrupt handler for secondary trigger.
* As loggerPrimaryISR, but for the secondary trigger.
*/
void loggerSecondaryISR(void)
{
  _decoderState.decoderStatus.validTrigger = true; //This value will be set to the return value of the decoder function, indicating whether or not this pulse passed the filters
  /* 3 checks here:
  1) If the primary trigger is RISING, then check whether the primary is currently HIGH
  2) If the primary trigger is FALLING, then check whether the primary is currently LOW
  3) The secondary trigger is CHANGING
  If any of these are true, the primary decoder function is called
  */
  if (currentStatus.decoder.secondary.isTriggered())
  {
    currentStatus.decoder.secondary.callback();
  }
  //No tooth logger for the secondary input
  if( (currentStatus.compositeTriggerUsed > 0) && (_decoderState.decoderStatus.validTrigger) )
  {
    //Composite logger adds an entry regardless of which edge it was
    addToothLogEntry(_decoderState.curGap2, TOOTH_CAM_SECONDARY);
  }
}

/** Interrupt handler for third trigger.
* As loggerPrimaryISR, but for the third trigger.
*/
void loggerTertiaryISR(void)
{
  _decoderState.decoderStatus.validTrigger = false; //This value will be set to the return value of the decoder function, indicating whether or not this pulse passed the filters
  _decoderState.decoderStatus.validTrigger = true; //This value will be set to the return value of the decoder function, indicating whether or not this pulse passed the filters
  /* 3 checks here:
  1) If the primary trigger is RISING, then check whether the primary is currently HIGH
  2) If the primary trigger is FALLING, then check whether the primary is currently LOW
  3) The secondary trigger is CHANGING
  If any of these are true, the primary decoder function is called
  */


  if(currentStatus.decoder.tertiary.isTriggered())
  {
    currentStatus.decoder.tertiary.callback();
  }
  //No tooth logger for the secondary input
  if( (currentStatus.compositeTriggerUsed > 0) && (_decoderState.decoderStatus.validTrigger) )
  {
    //Composite logger adds an entry regardless of which edge it was
    addToothLogEntry(_decoderState.curGap3, TOOTH_CAM_TERTIARY);
  }  
}

static decoder_status_t sharedGetStatus(void) noexcept
{
  // NOTE: we are deliberately returning a copy of the struct to avoid read tearing since it's written to within interrupts
  ATOMIC() {
    return _decoderState.decoderStatus;
  }
  __builtin_unreachable(); 
}

static bool sharedEngineIsRunning(uint32_t curTime) {
  return _decoderState.toothWithinMaxStallTime(curTime);
}

static decoder_features_t sharedGetDecoderFeatures(void)
{
  return _decoderState.decoderFeatures;
}

// Common function shared between decoders.
static void sharedDecoderReset(void) {
  _decoderState.reset();
}

static uint8_t getConfigPriTriggerEdge(const config4 &page4)
{
  return page4.TrigEdge == 0U ? RISING : FALLING;
}
static uint8_t getConfigSecTriggerEdge(const config4 &page4)
{
  return page4.TrigEdgeSec == 0U ? RISING : FALLING;
}
static uint8_t getConfigTerTriggerEdge(const config10 &page10)
{
  return page10.TrigEdgeThrd == 0U ? RISING : FALLING;
}

  
// LCOV_EXCL_START
// Exclude one line wrappers from coverage
static void triggerPri_missingTooth(void)
{
  decoders::missing_tooth::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
}

static void triggerThird_missingTooth(void)
{
  decoders::missing_tooth::triggerTertiary(micros(), currentStatus, _decoderState, configPage4, configPage6);
}

static uint32_t getRevolutionTime_missingTooth(void)
{
  return decoders::missing_tooth::getRevolutionTime(currentStatus, _decoderState, configPage4);
}

static int16_t getCrankAngle_missingTooth(uint32_t currMicros)
{
  return decoders::missing_tooth::getCrankAngle(currMicros, _decoderState, configPage4);
}

static void triggerSetEndTeeth_missingTooth(void)
{
  decoders::missing_tooth::setEndTeeth(_decoderState, configPage4);
}
// LCOV_EXCL_STOP

decoder_t __attribute__((optimize("Os"))) triggerSetup_missingTooth(void)
{
  _decoderState = decoders::missing_tooth::initialise(configPage4);

  bool hasSecondary =  (configPage4.TrigSpeed == CRANK_SPEED) 
                    && ( (configPage4.sparkMode == IGN_MODE_SEQUENTIAL) 
                      || (configPage2.injLayout == INJ_SEQUENTIAL) 
                      || (configPage6.vvtEnabled > 0) );

  static const auto triggerSecondary = []() {
    decoders::missing_tooth::triggerSecondary(micros(), currentStatus, _decoderState, configPage4, configPage6, configPage10);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(triggerPri_missingTooth, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(triggerSecondary, hasSecondary ? getConfigSecTriggerEdge(configPage4) : TRIGGER_EDGE_NONE)
                  .setTertiaryTrigger(triggerThird_missingTooth, configPage10.vvt2Enabled ? getConfigTerTriggerEdge(configPage10) : TRIGGER_EDGE_NONE)
                  .setGetRevolutionTime(getRevolutionTime_missingTooth)
                  .setGetCrankAngle(getCrankAngle_missingTooth)
                  .setSetEndTeeth(triggerSetEndTeeth_missingTooth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

// LCOV_EXCL_START
// Exclude one line wrappers from coverage
static void triggerPri_DualWheel(void)
{
    decoders::dual_wheel::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
}

static void triggerSec_DualWheel(void)
{
    decoders::dual_wheel::triggerSecondary(micros(), currentStatus, _decoderState, configPage4);
}

static uint32_t getRevolutionTime_DualWheel(void)
{
    return decoders::dual_wheel::getRevolutionTime(currentStatus, _decoderState, configPage4);
}

static int16_t getCrankAngle_DualWheel(uint32_t currMicros)
{
    return decoders::dual_wheel::getCrankAngle(currMicros, _decoderState, configPage4);
}

static void triggerSetEndTeeth_DualWheel(void)
{
    decoders::dual_wheel::setEndTeeth(_decoderState, configPage4);
}
// LCOV_EXCL_STOP

decoder_t  __attribute__((optimize("Os"))) triggerSetup_DualWheel(void)
{
  _decoderState = decoders::dual_wheel::initialise(configPage4);

  return decoder_builder_t()
                  .setPrimaryTrigger(triggerPri_DualWheel, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(triggerSec_DualWheel, getConfigSecTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime_DualWheel)
                  .setGetCrankAngle(getCrankAngle_DualWheel)
                  .setSetEndTeeth(triggerSetEndTeeth_DualWheel)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Basic Distributor where tooth count is equal to the number of cylinders and teeth are evenly spaced on the cam.
* No position sensing (Distributor is retained) so crank angle is
* a made up figure based purely on the first teeth to be seen.
* Note: This is a very simple decoder. See http://www.megamanual.com/ms2/GM_7pinHEI.htm
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_BasicDistributor(void)
{
  _decoderState = decoders::basic_distributor::initialise(configPage2);

  static const auto primaryTrigger = []() {
    decoders::basic_distributor::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::basic_distributor::getRevolutionTime(currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::basic_distributor::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::basic_distributor::setEndTeeth(_decoderState, configPage2, configPage4);
  };
  
  return decoder_builder_t()
                .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                .setGetRevolutionTime(getRevolutionTime)
                .setGetCrankAngle(getCrankAngle)
                .setSetEndTeeth(setEndTeeth)
                .setReset(sharedDecoderReset)
                .setIsEngineRunning(sharedEngineIsRunning)
                .setGetStatus(sharedGetStatus)
                .setGetFeatures(sharedGetDecoderFeatures)
                .build();
}

/** Decode GM 7X trigger wheel with six equally spaced teeth and a seventh tooth for cylinder identification.
* Note: Within the decoder code pf GM7X, the sync tooth is referred to as tooth #3 rather than tooth #7. This makes for simpler angle calculations
* (See: http://www.speeduino.com/forum/download/file.php?id=4743 ).
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_GM7X(void)
{
  _decoderState = decoders::gm7x::initialise();

  static const auto primaryTrigger = []() {
    decoders::gm7x::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
      return stdGetRevolutionTime(currentStatus, _decoderState, CRANK_SPEED);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
      return decoders::gm7x::getCrankAngle(curMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
      return decoders::gm7x::setEndTeeth(_decoderState, currentStatus);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}


/** Mitsubishi 4G63 / NA/NB Miata + MX-5 / 4/2.
Note: raw.githubusercontent.com/noisymime/speeduino/master/reference/wiki/decoders/4g63_trace.png
Tooth #1 is defined as the next crank tooth after the crank signal is HIGH when the cam signal is falling.
Tooth number one is at 355* ATDC.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_4G63(void)
{
  _decoderState = decoders::fourg63::initialise(micros(), currentStatus, configPage2);
  
  static const auto primaryTrigger = []() {
    decoders::fourg63::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::fourg63::triggerSecondary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::fourg63::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::fourg63::getCrankAngle(curMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::fourg63::setEndTeeth(_decoderState, configPage2, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, CHANGE)
                  .setSecondaryTrigger(secondaryTrigger, FALLING)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** GM 24X Decoder (eg early LS1 1996-2005).
Note: Useful references:
* 
- www.vems.hu/wiki/index.php?page=MembersPage%2FJorgenKarlsson%2FTwentyFourX

Provided that the cam signal is used, this decoder simply counts the teeth and then looks their angles up against a lookup table. The cam signal is used to determine tooth #1
*/
decoder_t __attribute__((optimize("Os"))) triggerSetup_24X(void)
{
  _decoderState = decoders::gm24x::initialise(micros(), currentStatus);
 
  static const auto primaryTrigger = []() {
    decoders::gm24x::triggerPrimary(micros(), currentStatus, _decoderState);
  };
  static const auto secondaryTrigger = []() {
    decoders::gm24x::triggerSecondary(_decoderState);
  };
  static const auto getRevolutionTime = []() {
    return stdGetRevolutionTime(currentStatus, _decoderState, CRANK_SPEED);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return (int16_t)decoders::shared::clampCrankAngle(decoders::shared::atomic_make_lookup_caa(_decoderState).calculateCrankAngle(curMicros, _decoderState.toothAngles, configPage4));
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, CHANGE)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Jeep 2000 - 24 crank teeth over 720 degrees, in groups of 4 ('91 to 2000 6 cylinder Jeep engines).
* Crank wheel is high for 360 crank degrees. Quite similar to the 24X setup.
* As we only need timing within 360 degrees, only 12 tooth angles are defined.
* Tooth number 1 represents the first tooth seen after the cam signal goes high.
* www.speeduino.com/forum/download/file.php?id=205
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Jeep2000(void)
{
  _decoderState = decoders::jeep2000::initialise(micros(), currentStatus);

  static const auto primaryTrigger = []() {
    decoders::jeep2000::triggerPrimary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::jeep2000::triggerSecondary(_decoderState);
  };
  static const auto getRevolutionTime = []() {
    return stdGetRevolutionTime(currentStatus, _decoderState, CRANK_SPEED);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::jeep2000::getCrankAngle(curMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, CHANGE)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Audi with 135 teeth on the crank and 1 tooth on the cam.
* This is very similar to the dual wheel decoder, however due to the 135 teeth not dividing evenly into 360,
* only every 3rd crank tooth is used in calculating the crank angle. This effectively makes it a 45 tooth dual wheel setup.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Audi135(void)
{
  _decoderState = decoders::audi135::initialise();

  static const auto primaryTrigger = []() {
    decoders::audi135::triggerPrimary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::audi135::triggerSecondary(currentStatus, _decoderState, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return stdGetRevolutionTime(currentStatus, _decoderState, CRANK_SPEED);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::audi135::getCrankAngle(curMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, RISING)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Honda D17 (1.7 liter 4 cyl SOHC).
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_HondaD17(void)
{
  _decoderState = decoders::hondad17::initialise();

  static const auto primaryTrigger = []() {
    decoders::hondad17::triggerPrimary(micros(), currentStatus, _decoderState);
  };
  static const auto getRevolutionTime = []() {
    return stdGetRevolutionTime(currentStatus, _decoderState, CRANK_SPEED);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::hondad17::getCrankAngle(currMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Honda J 32 (3.2 liter 6 cyl SOHC).
*  The Honda J32a4 (and all J series I'm aware of) has a crank trigger with nominal 24 teeth (22 teeth actually present). 
*  It has one missing tooth, then 7 teeth, then another missing tooth, then 15 teeth.
*  The tooth rising edges all have uniform spacing between them, except for teeth 14 and 22, which measure about
*  18 degrees between rising edges, rather than 15 degrees as the other teeth do.  These slightly larger
*  teeth are immediately before a gap, and the extra 3 degrees is made up for in the gap, the gap being about
*  3 degrees smaller than might be nominally expected, such that the expected rotational angle is restored immediately after
*  the gap (missing tooth) passes.  
*  Teeth are represented as 0V at the ECU, no teeth are represented as 5V.
*  Top dead center of cylinder number 1 occurs as we lose sight of (just pass) the first tooth in the string of 15 teeth
*  (this is a rising edge).
*  The second tooth in the string of 15 teeth is designated as tooth 1 in this code. This means that
*  when the interrupt for tooth 1 fires (as we just pass tooth 1), crank angle = 360/24 = 15 degrees.
*  It follows that the first tooth in the string of 7 teeth is tooth 16. 
*/ 
decoder_t __attribute__((optimize("Os"))) triggerSetup_HondaJ32(void)
{
  _decoderState = decoders::hondaj32::initialise();

  static const auto triggerPrimary = []() {
    decoders::hondaj32::triggerPrimary(micros(), currentStatus, _decoderState);
  };
  static const auto getRevolutionTime = []() {
    return decoders::hondaj32::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::hondaj32::getCrankAngle(curMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(triggerPrimary, RISING) // Don't honor the config, always use rising edge
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Miata '99 to '05 with 4x 70 degree duration teeth running at cam speed.
Teeth believed to be at the same angles as the 4g63 decoder.
Tooth #1 is defined as the next crank tooth after the crank signal is HIGH when the cam signal is falling.
Tooth number one is at 355* ATDC.
* (See: www.forum.diyefi.org/viewtopic.php?f=56&t=1077)
*/

int getCamAngle_Miata9905(void)
{
  int16_t curAngle;
  //_decoderState.lastVVTtime is the time between tooth #1 (10* BTDC) and the single cam tooth. 
  //All cam angles in in BTDC, so the actual advance angle is 370 - timeToAngle(_decoderState.lastVVTtime) - <the angle of the cam at 0 advance>
  curAngle = 370 - timeToAngle(_decoderState.lastVVTtime) - configPage10.vvtCL0DutyAng;
  currentStatus.vvt1.angle = LOW_PASS_FILTER( (curAngle << 1), configPage4.ANGLEFILTER_VVT, currentStatus.vvt1.angle);

  return currentStatus.vvt1.angle;
}

decoder_t  __attribute__((optimize("Os"))) triggerSetup_Miata9905(void)
{
  _decoderState = decoders::miata9905::initialise(micros(), currentStatus);

  static const auto primaryTrigger = []() {
    decoders::miata9905::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::miata9905::triggerSecondary(micros(), currentStatus, _decoderState);
  };
  static const auto getRevolutionTime = []() {
    return decoders::miata9905::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::miata9905::getCrankAngle(curMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::miata9905::setEndTeeth(currentStatus, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, getConfigSecTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Mazda AU version.
Tooth #2 is defined as the next crank tooth after the single cam tooth.
Tooth number one is at 348* ATDC.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_MazdaAU(void)
{
  _decoderState = decoders::mazdaau::initialise();

  static const auto primaryTrigger = [] () {
    decoders::mazdaau::triggerPrimary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto secondaryTrigger = [] () {
    decoders::mazdaau::triggerSecondary(micros(), _decoderState);
  };
  static const auto getRevolutionTime = [] () {
    return decoders::mazdaau::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [] (uint32_t curTime) {
    return decoders::mazdaau::getCrankAngle(curTime, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, FALLING)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Non-360 Dual wheel with 2 wheels located either both on the crank or with the primary on the crank and the secondary on the cam.
There can be no missing teeth on the primary wheel.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_non360(void)
{
  _decoderState = decoders::non360::initialise(configPage4);

  static const auto triggerPrimary = []() {
    // Note reuse of dual wheel
    decoders::dual_wheel::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto triggerSecondary = []() {
    // Note reuse of dual wheel
    decoders::dual_wheel::triggerSecondary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::non360::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::non360::getCrankAngle(curMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(triggerPrimary, getConfigPriTriggerEdge(configPage4)) //Is identical to the dual wheel decoder, so that is used. Same goes for the secondary below
                  .setSecondaryTrigger(triggerSecondary, FALLING) //Note the use of the Dual Wheel trigger function here. No point in having the same code in twice.
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Nissan 360 tooth on cam (Optical trigger disc inside distributor housing).
See http://wiki.r31skylineclub.com/index.php/Crank_Angle_Sensor .
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Nissan360(void)
{
  _decoderState = decoders::nissan360::initialise();

  static const auto triggerPrimary = []() {
    decoders::nissan360::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto triggerSecondary = []() {
    decoders::nissan360::triggerSecondary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::nissan360::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::nissan360::getCrankAngle(curMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    return decoders::nissan360::setEndTeeth(_decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(triggerPrimary, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(triggerSecondary, CHANGE)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Subaru 6/7 Trigger pattern decoder for 6 tooth (irregularly spaced) crank and 7 tooth (also fairly irregular) cam wheels (eg late 90's Impreza 2.2).
This seems to be present in late 90's Subaru. In 2001 Subaru moved to 36-2-2-2 (See: http://www.vems.hu/wiki/index.php?page=InputTrigger%2FSubaruTrigger ).
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Subaru67(void)
{
  _decoderState = decoders::subaru67::initialise();

  static const auto primaryTrigger = [] () {
    decoders::subaru67::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = [] () {
    decoders::subaru67::triggerSecondary(micros(), currentStatus, _decoderState);
  };
  static const auto getRevolutionTime = [] () {
    return decoders::subaru67::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [] (uint32_t curMicros) {
    return decoders::subaru67::getCrankAngle(curMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = [] () {
    decoders::subaru67::setEndTeeth(currentStatus, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, FALLING)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Daihatsu +1 trigger for 3 and 4 cylinder engines.
* Tooth equal to the number of cylinders are evenly spaced on the cam. No position sensing (Distributor is retained),
* so crank angle is a made up figure based purely on the first teeth to be seen.
* Note: This is a very simple decoder. See http://www.megamanual.com/ms2/GM_7pinHEI.htm
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Daihatsu(void)
{
  _decoderState = decoders::daihatsu::initialise(configPage2);

  static const auto primaryTrigger = []() {
    decoders::daihatsu::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::daihatsu::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return (int16_t)decoders::shared::clampCrankAngle(decoders::shared::atomic_make_lookup_caa(_decoderState).calculateCrankAngle(currMicros, _decoderState.toothAngles, configPage4));
  };
  
  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Harley Davidson (V2) with 2 unevenly Spaced Teeth.
Within the decoder code, the sync tooth is referred to as tooth #1. Derived from GMX7 and adapted for Harley.
Only rising Edge is used for simplicity.The second input is ignored, as it does not help to resolve cam position.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Harley(void)
{
  _decoderState = decoders::harley::initialise(micros(), currentStatus);

  static const auto primaryTrigger = []() {
    decoders::harley::triggerPrimary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::harley::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t curMicros) {
    return decoders::harley::getCrankAngle(curMicros, _decoderState, configPage4);
  };
  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, RISING)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** 36-2-2-2 crank based trigger wheel.
* A crank based trigger with a nominal 36 teeth, but 6 of these removed in 3 groups of 2.
* 2 of these groups are located concurrently.
* Note: This decoder supports both the H4 version (13-missing-16-missing-1-missing) and the H6 version of 36-2-2-2 (19-missing-10-missing-1-missing).
* The decoder checks which pattern is selected in order to determine the tooth number
* Note: www.thefactoryfiveforum.com/attachment.php?attachmentid=34279&d=1412431418
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_ThirtySixMinus222(void)
{
  _decoderState = decoders::thirtysixminus222::initialise(configPage4);

  static const auto primaryTrigger = []() {
    decoders::thirtysixminus222::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::thirtysixminus222::getRevolutionTime(currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    // Note reuse of missing tooth function
    return decoders::missing_tooth::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::thirtysixminus222::setEndTeeth(currentStatus, _decoderState, configPage2);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** 36-2-1 / Mistsubishi 4B11 - A crank based trigger with a nominal 36 teeth, but with 1 single and 1 double missing tooth.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_ThirtySixMinus21(void)
{
  _decoderState = decoders::thirtysixminus21::initialise(configPage4);

  static const auto primaryTrigger = []() {
    decoders::thirtysixminus21::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto triggerSecondary = []() {
    // Note reuse of missing tooth
    decoders::missing_tooth::triggerSecondary(micros(), currentStatus, _decoderState, configPage4, configPage6, configPage10);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    // Note reuse of missing tooth
    return decoders::missing_tooth::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::thirtysixminus21::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    return decoders::thirtysixminus21::setEndTeeth(_decoderState);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(triggerSecondary, getConfigSecTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

//************************************************************************************************************************

/** DSM 420a, For the DSM Eclipse with 16 teeth total on the crank.
* Tracks the falling side of the signal.
* Sync is determined by watching for a falling edge on the secondary signal and checking if the primary signal is high then.
* https://github.com/noisymime/speeduino/issues/133
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_420a(void)
{
  _decoderState = decoders::dsm420a::initialise();

  static const auto primaryTrigger = []() {
    decoders::dsm420a::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::dsm420a::triggerSecondary(currentStatus, _decoderState);
  };
  static const auto getRevolutionTime = []() {
    return stdGetRevolutionTime(currentStatus, _decoderState, CAM_SPEED);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return (int16_t)decoders::shared::clampCrankAngle(decoders::shared::atomic_make_lookup_caa(_decoderState).calculateCrankAngle(currMicros, _decoderState.toothAngles, configPage4));
  };
  static const auto setEndTeeth = []() {
    decoders::dsm420a::setEndTeeth(currentStatus, _decoderState);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, FALLING) //Always falling edge
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Weber-Marelli trigger setup with 2 wheels, 4 teeth 90deg apart on crank and 2 90deg apart on cam.
Uses DualWheel decoders, There can be no missing teeth on the primary wheel.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Webber(void)
{
  static const auto primaryTrigger = []() {
    decoders::webber::triggerPrimary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::webber::triggerSecondary(micros(), currentStatus, _decoderState, configPage4);
  };

  return decoder_builder_t(triggerSetup_DualWheel())
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, getConfigSecTriggerEdge(configPage4))
                  .build();
}

/** Ford ST170 - a dedicated decoder for 01-04 Ford Focus ST170/SVT engine.
Standard 36-1 trigger wheel running at crank speed and 8-3 trigger wheel running at cam speed.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_FordST170(void)
{
  _decoderState = decoders::st170::initialise(configPage4);

  static const auto primaryTrigger = []() {
    decoders::missing_tooth::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::st170::triggerSecondary(micros(), currentStatus, _decoderState, configPage4, configPage6, configPage10);
  };
  static const auto getRevolutionTime = []() {
    return decoders::st170::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return (int16_t)decoders::shared::clampCrankAngle(decoders::shared::atomic_make_angle_caa(_decoderState).calculateCrankAngle(currMicros, configPage4));
  };
  static const auto setEndTeeth = []() {
    decoders::st170::setEndTeeth(_decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, getConfigSecTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

decoder_t  __attribute__((optimize("Os"))) triggerSetup_DRZ400(void)
{
  _decoderState = decoders::drz400::initialise(configPage4);

  static const auto primaryTrigger = []() {
    // Reuse dual wheel primary trigger
    decoders::dual_wheel::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::drz400::triggerSecondary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::dual_wheel::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::dual_wheel::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::dual_wheel::setEndTeeth(_decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, getConfigSecTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}


/** Chrysler NGC - a dedicated decoder for vehicles with 4, 6 and 8 cylinder NGC pattern.
4-cyl: 36+2-2 crank wheel and 7 tooth cam
6-cyl: 36-2+2 crank wheel and 12 tooth cam in 6 groups
8-cyl: 36-2+2 crank wheel and 15 tooth cam in 8 groups
The crank decoder uses the polarity of the missing teeth to determine position
The 4-cyl cam decoder uses the polarity of the missing teeth to determine position
The 6 and 8-cyl cam decoder uses the amount of teeth in the two previous groups of teeth to determine position
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_NGC(void)
{
  _decoderState = decoders::ngc::initialise(configPage2, configPage4);

  static const auto primaryTrigger = []() {
    decoders::ngc::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger4 = []() {
    decoders::ngc::triggerSecondary4(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger68 = []() {
    decoders::ngc::triggerSecondary68(micros(), _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::ngc::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::missing_tooth::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::ngc::setEndTeeth(_decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, CHANGE)
                  .setSecondaryTrigger( configPage2.nCylinders == 4U ? secondaryTrigger4 : secondaryTrigger68,
                                        configPage2.nCylinders == 4U ? CHANGE : FALLING)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Yamaha Vmax 1990+ with 6 uneven teeth, triggering on the wide lobe.
Within the decoder code, the sync tooth is referred to as tooth #1. Derived from Harley and made to work on the Yamah Vmax.
Trigger is based on 'CHANGE' so we get a signal on the up and downward edges of the lobe. This is required to identify the wide lobe.
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Vmax(void)
{
  _decoderState = decoders::vmax::initialise(micros(), currentStatus);

  static const auto primaryTrigger = []() {
    decoders::vmax::triggerPrimary(micros(), currentStatus, _decoderState, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::vmax::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::vmax::getCrankAngle(currMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, CHANGE)
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Renix 44-2-2  and 66-2-2-2 decoder.  
* Renix trigger wheel doesn't decode into 360 degrees nicely (360/44 = 8.18 degrees or 360/66 = 5.454545). Speeduino can't handle any teeth that have a decimal point.
* Solution is to count teeth, every 11 teeth = a proper angle. For 66 tooth decoder its 60 degrees per 11 teeth, for 44 tooth decoder its 90 degrees per 11 teeth.
* This means the system sees 4 teeth on the 44 tooth wheel and 6 teeth on the 66 tooth wheel.
* Double missing tooth in the pattern is actually a large tooth and a large gap. If the trigger is set to rising you'll see the start of the large tooth
* then the gap. If its not set to rising the code won't work due to seeing two gaps
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_Renix(void)
{
  _decoderState = decoders::renix::initialise(configPage2, configPage4);

  static const auto primaryTrigger = []() {
    decoders::renix::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::missing_tooth::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::missing_tooth::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::renix::setEndTeeth(_decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/*****************************************************************
 * Rover MEMS decoder
 * Covers multiple trigger wheels used interchanbably over the range of MEMS units
 * Specifically covers teeth patterns on the primary trigger (crank)
 * 3 gap 14 gap 2 gap 13 gap
 * 11 gap 5 gap 12 gap 4 gap 
 * 2 gap 14 gap 3 gap 13 gap 
 * 17 gap 17 gap 
 *
 * Support no cam, single tooth Cam (or half moon cam), and multi tooth (5-3-2 teeth)
 */
decoder_t  __attribute__((optimize("Os"))) triggerSetup_RoverMEMS(void)
{
  _decoderState = decoders::rovermems::initialise(configPage4);

  static const auto primaryTrigger = []() {
    decoders::rovermems::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto secondaryTrigger = []() {
    decoders::rovermems::triggerSecondary(micros(), currentStatus, _decoderState, configPage4, configPage6, configPage10);
  };
  static const auto getRevolutionTime = []() {
    return decoders::rovermems::getRevolutionTime(currentStatus, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::rovermems::setEndTeeth(_decoderState, configPage4);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::missing_tooth::getCrankAngle(currMicros, _decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(secondaryTrigger, getConfigSecTriggerEdge(configPage4)) 
                  .setGetRevolutionTime(getRevolutionTime)
                  .setSetEndTeeth(setEndTeeth)
                  .setGetCrankAngle(getCrankAngle)   
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Suzuki K6A 3 cylinder engine

* (See: https://www.msextra.com/forums/viewtopic.php?t=74614)
*/
decoder_t  __attribute__((optimize("Os"))) triggerSetup_SuzukiK6A(void)
{
  _decoderState = decoders::k6a::initialise(micros(), currentStatus, configPage4);

  static const auto primaryTrigger = []() {
    decoders::k6a::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };
  static const auto getRevolutionTime = []() {
    return decoders::k6a::getRevolutionTime(currentStatus, _decoderState);
  };
  static const auto getCrankAngle = [](uint32_t currMicros) {
    return decoders::k6a::getCrankAngle(currMicros, _decoderState, configPage4);
  };
  static const auto setEndTeeth = []() {
    decoders::k6a::setEndTeeth(_decoderState, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4)) // only primary, no secondary, trigger pattern is over 720 degrees
                  .setGetRevolutionTime(getRevolutionTime)
                  .setGetCrankAngle(getCrankAngle)
                  .setSetEndTeeth(setEndTeeth)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}

/** Ford TFI - Distributor mounted signal tooth wheel. Same number of teeth as cylinders.
Evenly spaced rising edge triggers, Cylinder 1 has a narrow teeth and will have a signature falling edge

*/

/** Ford TFI Secondary (Falling Edge).
 * 
 * */
static void triggerSec_FordTFI(void)
{
  uint32_t curTime2 = micros();
  if (curTime2 >= _decoderState.toothLastSecToothTime) 
    { _decoderState.curGap2 = curTime2 - _decoderState.toothLastSecToothTime; } 
  else
    { _decoderState.curGap2 = (4294967296 - _decoderState.toothLastSecToothTime + curTime2); }
  
 
  //Safety check for initial startup
  if( (_decoderState.toothLastSecToothTime == 0) )
  { 
    _decoderState.curGap2 = 0; 
    _decoderState.toothLastSecToothTime = curTime2;
  }
  

  if ( _decoderState.curGap2 >= _decoderState.triggerSecFilterTime ) // Valid tooth falling edge
  {     
    if ((_decoderState.curGap > 0) && (_decoderState.curGap < 20000000)) // Limit to prevent overflow
    {  
      _decoderState.targetGap2 = percentage(110, _decoderState.curGap); // Wide last teeth gap min
      _decoderState.targetGap3 = percentage(90, _decoderState.curGap); // Narrow last teeth minus one gap max
    }
    else 
    {
    _decoderState.targetGap2 = 0;
    _decoderState.targetGap3 = 0;
    }
    if ( (_decoderState.curGap2 > _decoderState.targetGap2) && (_decoderState.curGap3 < _decoderState.targetGap3) && (_decoderState.lastGap < _decoderState.targetGap2) && (_decoderState.lastGap > _decoderState.targetGap3) ) // Signature Tooth detected
    {
      if( (_decoderState.decoderStatus.syncStatus!=SyncStatus::Full) || (currentStatus.startRevolutions <= configPage4.StgCycles) )
      {
        _decoderState.toothCurrentCount = 2; // Last primary tooth was #2
        _decoderState.triggerFilterTime = 0; //Need to turn the filter off here otherwise the first primary tooth after achieving sync is ignored
        _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
      }
      else 
      {
        if ( (_decoderState.toothCurrentCount != 2) && (currentStatus.startRevolutions > 2)) 
        { 
          currentStatus.syncLossCounter++;
        } //Indicates likely sync loss.
        if (configPage4.useResync == 1) 
        { 
          _decoderState.toothCurrentCount = 2;
          _decoderState.decoderStatus.syncStatus = SyncStatus::Full;  
        }
      }
      _decoderState.lastSyncRevolution = currentStatus.startRevolutions ; // Revolution count when signature tooth was detected

    }

  _decoderState.toothLastSecToothTime = curTime2; // 
  _decoderState.lastGap = _decoderState.curGap3; // Minus two Gap
  _decoderState.curGap3 = _decoderState.curGap2; // Minus one Gap

  } //Trigger filter
  //else { currentStatus.syncLossCounter = 0; }
  _decoderState.triggerSecFilterTime = _decoderState.curGap >> 1; //Set filter at 50 % speed of last primary gap
}

/** Ford TFI - Get RPM.
 * 
 * */
static uint32_t getRevolutionTime_FordTFI(void)
{
  const uint8_t distributorSpeed = CAM_SPEED;

  const bool useCrankingCalc = (currentStatus.RPM < currentStatus.crankRPM) || (currentStatus.RPM < 1500U);
  const uint32_t revolutionTime = useCrankingCalc ? crankingGetRevolutionTime(currentStatus, _decoderState, configPage4, _decoderState.triggerActualTeeth, distributorSpeed)
                                                  : stdGetRevolutionTime(currentStatus, _decoderState, distributorSpeed);

  _decoderState.MAX_STALL_TIME = revolutionTime << 1; //Set the stall time to be twice the current RPM. This is a safe figure as there should be no single revolution where this changes more than this
  if(_decoderState.MAX_STALL_TIME < 366667UL) { _decoderState.MAX_STALL_TIME = 366667UL; } //Check for 50rpm minimum

  return revolutionTime;
  
}

/** Ford TFI - Get Crank angle.
 * 
 * */
static int16_t getCrankAngle_FordTFI(uint32_t currMicros)
{
  auto calculator = decoders::shared::atomic_make_angle_caa(_decoderState);

  //Handle case where the secondary tooth was the last one seen
  if(calculator._toothCurrentCount == 0) { calculator._toothCurrentCount = 2; } 

  return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(currMicros, configPage4));
}

/** Ford TFI - Set End Teeth.
 * 
 * */
static void triggerSetEndTeeth_FordTFI(void)
{
  int tempEndAngle = (ignitionSchedule1.dischargeAngle - configPage4.triggerAngle);
  tempEndAngle = ignitionLimits((tempEndAngle));

  switch(configPage2.nCylinders)
  {
    case 4:
      if( (tempEndAngle > 180) || (tempEndAngle <= 0) )
      {
        _decoderState.ignitionEndTeeth[0] = 2;
        _decoderState.ignitionEndTeeth[1] = 1;
      }
      else
      {
        _decoderState.ignitionEndTeeth[0] = 1;
        _decoderState.ignitionEndTeeth[1] = 2;
      }
      break;
    case 6:
      if( (tempEndAngle > 120) && (tempEndAngle <= 240) )
      {
        _decoderState.ignitionEndTeeth[0] = 2;
        _decoderState.ignitionEndTeeth[1] = 3;
        _decoderState.ignitionEndTeeth[2] = 1;
      }
      else if( (tempEndAngle > 240) || (tempEndAngle <= 0) )
      {
        _decoderState.ignitionEndTeeth[0] = 3;
        _decoderState.ignitionEndTeeth[1] = 1;
        _decoderState.ignitionEndTeeth[2] = 2;
      }
      else
      {
        _decoderState.ignitionEndTeeth[0] = 1;
        _decoderState.ignitionEndTeeth[1] = 2;
        _decoderState.ignitionEndTeeth[2] = 3;
      }
      break;
    case 8:
      if( (tempEndAngle > 90) && (tempEndAngle <= 180) )
      {
        _decoderState.ignitionEndTeeth[0] = 2;
        _decoderState.ignitionEndTeeth[1] = 3;
        _decoderState.ignitionEndTeeth[2] = 4;
        _decoderState.ignitionEndTeeth[3] = 1;
      }
      else if( (tempEndAngle > 180) && (tempEndAngle <= 270) )
      {
        _decoderState.ignitionEndTeeth[0] = 3;
        _decoderState.ignitionEndTeeth[1] = 4;
        _decoderState.ignitionEndTeeth[2] = 1;
        _decoderState.ignitionEndTeeth[3] = 2;
      }
      else if( (tempEndAngle > 270) || (tempEndAngle <= 0) )
      {
        _decoderState.ignitionEndTeeth[0] = 4;
        _decoderState.ignitionEndTeeth[1] = 1;
        _decoderState.ignitionEndTeeth[2] = 2;
        _decoderState.ignitionEndTeeth[3] = 3;
      }
      else
      {
        _decoderState.ignitionEndTeeth[0] = 1;
        _decoderState.ignitionEndTeeth[1] = 2;
        _decoderState.ignitionEndTeeth[2] = 3;
        _decoderState.ignitionEndTeeth[3] = 4;
      }
      break;
  }
}

/** Ford TFI Setup.
 * 
 * */
decoder_t  __attribute__((optimize("Os"))) triggerSetup_FordTFI(void)
{
  _decoderState = decoders::ford_tfi::initialise(configPage2);

  static const auto primaryTrigger = []() {
    decoders::ford_tfi::triggerPrimary(micros(), currentStatus, _decoderState, configPage2, configPage4);
  };

  return decoder_builder_t()
                  .setPrimaryTrigger(primaryTrigger, getConfigPriTriggerEdge(configPage4))
                  .setSecondaryTrigger(triggerSec_FordTFI, getConfigSecTriggerEdge(configPage4))
                  .setGetRevolutionTime(getRevolutionTime_FordTFI)
                  .setGetCrankAngle(getCrankAngle_FordTFI)
                  .setSetEndTeeth(triggerSetEndTeeth_FordTFI)
                  .setReset(sharedDecoderReset)
                  .setIsEngineRunning(sharedEngineIsRunning)
                  .setGetStatus(sharedGetStatus)
                  .setGetFeatures(sharedGetDecoderFeatures)
                  .build();
}
