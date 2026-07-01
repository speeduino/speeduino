#include "scheduler_ignition_controller.h"
#include "scheduledIO_ign.h"
#include "scheduledIO_ign.h"
#include "globals.h"
#include "unit_testing.h"

IgnitionSchedule ignitionSchedule1(IGN1_COUNTER, IGN1_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if IGN_CHANNELS >= 2
IgnitionSchedule ignitionSchedule2(IGN2_COUNTER, IGN2_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 3
IgnitionSchedule ignitionSchedule3(IGN3_COUNTER, IGN3_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 4
IgnitionSchedule ignitionSchedule4(IGN4_COUNTER, IGN4_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 5
IgnitionSchedule ignitionSchedule5(IGN5_COUNTER, IGN5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 6
IgnitionSchedule ignitionSchedule6(IGN6_COUNTER, IGN6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 7
IgnitionSchedule ignitionSchedule7(IGN7_COUNTER, IGN7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if IGN_CHANNELS >= 8
IgnitionSchedule ignitionSchedule8(IGN8_COUNTER, IGN8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif

constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);

static void __attribute__((optimize("Os"))) setSequentialCallbacks(uint8_t numChannels)
{
  setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
#if IGN_CHANNELS >= 2
  if (numChannels>=2)
  {
    setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
  }
#endif
#if IGN_CHANNELS >= 3
  if (numChannels>=3)
  {
    setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
  }
#endif
#if IGN_CHANNELS >= 4
  if (numChannels>=4)
  {
    setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
  }
#endif
#if IGN_CHANNELS >= 5
  if (numChannels>=5)
  {
    setCallbacks(ignitionSchedule5, beginCoil5Charge, endCoil5Charge);
  }
#endif
#if IGN_CHANNELS >= 6
  if (numChannels>=6)
  {
    setCallbacks(ignitionSchedule6, beginCoil6Charge, endCoil6Charge);
  }
#endif
#if IGN_CHANNELS >= 7
  if (numChannels>=7)
  {
    setCallbacks(ignitionSchedule7, beginCoil7Charge, endCoil7Charge);
  }
#endif
#if IGN_CHANNELS >= 8
  if (numChannels>=8)
  {
    setCallbacks(ignitionSchedule8, beginCoil8Charge, endCoil8Charge);
  }
#endif
}

static void __attribute__((optimize("Os"))) setWastedSparkCallbacks(void)
{
  setSequentialCallbacks(min((uint8_t)5U, (uint8_t)IGN_CHANNELS));
}

static void __attribute__((optimize("Os"))) setSingleChannelCallbacks(void)
{
  //Single channel mode. All ignition pulses are on channel 1
  setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
#if IGN_CHANNELS >= 2
  setCallbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 3
  setCallbacks(ignitionSchedule3, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 4
  setCallbacks(ignitionSchedule4, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 5
  setCallbacks(ignitionSchedule5, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 6
  setCallbacks(ignitionSchedule6, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 7
  setCallbacks(ignitionSchedule7, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 8
  setCallbacks(ignitionSchedule8, beginCoil1Charge, endCoil1Charge);
#endif
}

static void __attribute__((optimize("Os"))) setWastedCOPCallbacks(uint8_t numCylinders)
{
  //Wasted COP mode. Note, most of the boards can only run this for 4-cyl only.
  switch (numCylinders)
  {
  //If the person has inadvertently selected this when running more than 4 cylinders or other than 6 cylinders, just use standard Wasted spark mode
  default:
  //1-3 cylinder wasted COP is the same as regular wasted mode
  case 1:
  case 2:
  case 3:
    setWastedSparkCallbacks();
    break;

  case 4:
    //Wasted COP mode for 4 cylinders. Ignition channels 1&3 and 2&4 are paired together
    setCallbacks(ignitionSchedule1, beginCoil1and3Charge, endCoil1and3Charge);
#if IGN_CHANNELS >= 2
    setCallbacks(ignitionSchedule2, beginCoil2and4Charge, endCoil2and4Charge);
#endif
    break;
  
  case 6:
    //Wasted COP mode for 6 cylinders. Ignition channels 1&4, 2&5 and 3&6 are paired together
    setCallbacks(ignitionSchedule1, beginCoil1and4Charge, endCoil1and4Charge);
#if IGN_CHANNELS >= 2
    setCallbacks(ignitionSchedule2, beginCoil2and5Charge, endCoil2and5Charge);
#endif
#if IGN_CHANNELS >= 3
    setCallbacks(ignitionSchedule3, beginCoil3and6Charge, endCoil3and6Charge);
#endif
    break;
  
  case 8:
    //Wasted COP mode for 8 cylinders. Ignition channels 1&5, 2&6, 3&7 and 4&8 are paired together
    setCallbacks(ignitionSchedule1, beginCoil1and5Charge, endCoil1and5Charge);
#if IGN_CHANNELS >= 2
    setCallbacks(ignitionSchedule2, beginCoil2and6Charge, endCoil2and6Charge);
#endif
#if IGN_CHANNELS >= 3
    setCallbacks(ignitionSchedule3, beginCoil3and7Charge, endCoil3and7Charge);
#endif
#if IGN_CHANNELS >= 4
    setCallbacks(ignitionSchedule4, beginCoil4and8Charge, endCoil4and8Charge);
#endif
  }
}

static void __attribute__((optimize("Os"))) setRotaryCallbacks(uint8_t rotaryType)
{
  switch (rotaryType)
  {
  case ROTARY_IGN_FC:
    //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
    setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
#if IGN_CHANNELS >= 2
    setCallbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge);
#endif
#if IGN_CHANNELS >= 3
    setCallbacks(ignitionSchedule3, beginTrailingCoilCharge, endTrailingCoilCharge1);
#endif
#if IGN_CHANNELS >= 4
    setCallbacks(ignitionSchedule4, beginTrailingCoilCharge, endTrailingCoilCharge2);
#endif
    break;

    case ROTARY_IGN_FD:
    //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
    setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
#if IGN_CHANNELS >= 2
    setCallbacks(ignitionSchedule2, beginCoil1Charge, endCoil1Charge);
#endif

    //Trailing coils have their own channel each
    //IGN2 = front rotor trailing spark
#if IGN_CHANNELS >= 3
    setCallbacks(ignitionSchedule3, beginCoil2Charge, endCoil2Charge);
    //IGN3 = rear rotor trailing spark
#endif
#if IGN_CHANNELS >= 4
    setCallbacks(ignitionSchedule4, beginCoil3Charge, endCoil3Charge);
#endif
    break;
  
  case ROTARY_IGN_RX8:
    //RX8 outputs are simply 1 coil and 1 output per plug
    setSequentialCallbacks(4U);

  default:
    //No action for other RX ignition modes (Future expansion / MISRA compliant). 
    break;
  }
}

TESTABLE_STATIC void __attribute__((optimize("Os"))) setCallbacks(uint8_t sparkMode, uint8_t numCylinders, uint8_t rotaryMode)
{
  switch(sparkMode)
  {
  case IGN_MODE_SINGLE: setSingleChannelCallbacks(); break;
  case IGN_MODE_WASTEDCOP: setWastedCOPCallbacks(numCylinders); break;
  case IGN_MODE_SEQUENTIAL: setSequentialCallbacks(IGN_CHANNELS); break;
  case IGN_MODE_ROTARY: setRotaryCallbacks(rotaryMode); break;
  case IGN_MODE_WASTED:
  default:
    setWastedSparkCallbacks(); break;
  }
}

TESTABLE_STATIC void __attribute__((optimize("Os"))) resetIgnitionSchedulers(void)
{
  ignitionSchedule1.reset();
#if IGN_CHANNELS >= 2
  ignitionSchedule2.reset();
#endif
#if IGN_CHANNELS >= 3
  ignitionSchedule3.reset();
#endif
#if IGN_CHANNELS >= 4
  ignitionSchedule4.reset();
#endif
#if (IGN_CHANNELS >= 5)
  ignitionSchedule5.reset();
#endif
#if IGN_CHANNELS >= 6
  ignitionSchedule6.reset();
#endif
#if IGN_CHANNELS >= 7
  ignitionSchedule7.reset();
#endif
#if IGN_CHANNELS >= 8
  ignitionSchedule8.reset();
#endif
}

static void __attribute__((optimize("Os"))) initScheduleAngles(statuses &current, const config2 &page2, config4 &page4)
{
  CRANK_ANGLE_MAX_IGN = 360;

  switch (page2.nCylinders) {
  case 1:
      ignitionSchedule1.channelDegrees = 0;
      current.maxIgnOutputs = 1;

      //Sequential ignition works identically on a 1 cylinder whether it's odd or even fire. 
      if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }
      break;

  case 2:
      ignitionSchedule1.channelDegrees = 0;
      current.maxIgnOutputs = 2;
#if IGN_CHANNELS >= 2
      if (page2.engineType == EVEN_FIRE ) { ignitionSchedule2.channelDegrees = 180; }
      else { ignitionSchedule2.channelDegrees = page2.oddfire2; }
#endif

      //Sequential ignition works identically on a 2 cylinder whether it's odd or even fire (With the default being a 180 degree second cylinder).
      if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page2.strokes == FOUR_STROKE) ) { CRANK_ANGLE_MAX_IGN = 720; }
      break;

  case 3:
      ignitionSchedule1.channelDegrees = 0;
      current.maxIgnOutputs= 3;
      if (page2.engineType == EVEN_FIRE )
      {
        //Sequential and Single channel modes both run over 720 crank degrees, but only on 4 stroke engines.
        if( ( (page4.sparkMode == IGN_MODE_SEQUENTIAL) || (page4.sparkMode == IGN_MODE_SINGLE) ) && (page2.strokes == FOUR_STROKE) )
        {
#if IGN_CHANNELS >= 2
          ignitionSchedule2.channelDegrees = 240;
#endif
#if IGN_CHANNELS >= 3
          ignitionSchedule3.channelDegrees = 480;
#endif

          CRANK_ANGLE_MAX_IGN = 720;
        }
        else
        {
#if IGN_CHANNELS >= 2
          ignitionSchedule2.channelDegrees = 120;
#endif
#if IGN_CHANNELS >= 3
          ignitionSchedule3.channelDegrees = 240;
#endif
        }
      }
      else
      {
#if IGN_CHANNELS >= 2
        ignitionSchedule2.channelDegrees = page2.oddfire2;
#endif
#if IGN_CHANNELS >= 3
        ignitionSchedule3.channelDegrees = page2.oddfire3;
#endif
      }
      break;
  case 4:
      ignitionSchedule1.channelDegrees = 0;
      current.maxIgnOutputs = 2; //Default value for 4 cylinder, may be changed below
      if (page2.engineType == EVEN_FIRE )
      {
#if IGN_CHANNELS >= 2
        ignitionSchedule2.channelDegrees = 180;
#endif

        if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page2.strokes == FOUR_STROKE) )
        {
#if IGN_CHANNELS >= 3
          ignitionSchedule3.channelDegrees = 360;
#endif
#if IGN_CHANNELS >= 4
          ignitionSchedule4.channelDegrees = 540;
#endif

          CRANK_ANGLE_MAX_IGN = 720;
          current.maxIgnOutputs= 4;
        }
        if(page4.sparkMode == IGN_MODE_ROTARY)
        {
          //Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
#if IGN_CHANNELS >= 3
          ignitionSchedule3.channelDegrees = 0;
#endif
#if IGN_CHANNELS >= 4
          ignitionSchedule4.channelDegrees = 180;
#endif
          current.maxIgnOutputs= 4;

          page4.IgInv = GOING_LOW; //Force Going Low ignition mode (Going high is never used for rotary)
        }
      }
      else
      {
#if IGN_CHANNELS >= 2
        ignitionSchedule2.channelDegrees = page2.oddfire2;
#endif
#if IGN_CHANNELS >= 3
        ignitionSchedule3.channelDegrees = page2.oddfire3;
#endif
#if IGN_CHANNELS >= 4
        ignitionSchedule4.channelDegrees = page2.oddfire4;
#endif
        current.maxIgnOutputs= 4;
      }
      break;
  case 5:
      ignitionSchedule1.channelDegrees = 0;
#if IGN_CHANNELS >= 2
      ignitionSchedule2.channelDegrees = 72;
#endif
#if IGN_CHANNELS >= 3
      ignitionSchedule3.channelDegrees = 144;
#endif
#if IGN_CHANNELS >= 4
      ignitionSchedule4.channelDegrees = 216;
#endif
#if (IGN_CHANNELS >= 5)
      ignitionSchedule5.channelDegrees = 288;
#endif
      current.maxIgnOutputs= 5; //Only 4 actual outputs, so that's all that can be cut

      if(page4.sparkMode == IGN_MODE_SEQUENTIAL)
      {
#if IGN_CHANNELS >= 2
        ignitionSchedule2.channelDegrees = 144;
#endif
#if IGN_CHANNELS >= 3
        ignitionSchedule3.channelDegrees = 288;
#endif
#if IGN_CHANNELS >= 4
        ignitionSchedule4.channelDegrees = 432;
#endif
#if (IGN_CHANNELS >= 5)
        ignitionSchedule5.channelDegrees = 576;
#endif

        CRANK_ANGLE_MAX_IGN = 720;
      }
      break;
  case 6:
      ignitionSchedule1.channelDegrees = 0;
#if IGN_CHANNELS >= 2
      ignitionSchedule2.channelDegrees = 120;
#endif
#if IGN_CHANNELS >= 3
      ignitionSchedule3.channelDegrees = 240;
#endif
      current.maxIgnOutputs= 3;

  #if IGN_CHANNELS >= 6
      if( (page4.sparkMode == IGN_MODE_SEQUENTIAL))
      {
      ignitionSchedule4.channelDegrees = 360;
      ignitionSchedule5.channelDegrees = 480;
      ignitionSchedule6.channelDegrees = 600;
      CRANK_ANGLE_MAX_IGN = 720;
      current.maxIgnOutputs= 6;
      }
  #endif
      break;
  case 8:
      ignitionSchedule1.channelDegrees = 0;
#if IGN_CHANNELS >= 2
      ignitionSchedule2.channelDegrees = 90;
#endif
#if IGN_CHANNELS >= 3
      ignitionSchedule3.channelDegrees = 180;
#endif
#if IGN_CHANNELS >= 4
      ignitionSchedule4.channelDegrees = 270;
#endif
      current.maxIgnOutputs= 4;

      if( (page4.sparkMode == IGN_MODE_SINGLE))
      {
        current.maxIgnOutputs= 4;
        CRANK_ANGLE_MAX_IGN = 360;
      }
  
  #if IGN_CHANNELS >= 8
      if( (page4.sparkMode == IGN_MODE_SEQUENTIAL))
      {
      ignitionSchedule5.channelDegrees = 360;
      ignitionSchedule6.channelDegrees = 450;
      ignitionSchedule7.channelDegrees = 540;
      ignitionSchedule8.channelDegrees = 630;
      current.maxIgnOutputs= 8;
      CRANK_ANGLE_MAX_IGN = 720;
      }
  #endif
      break;
  default: //Handle this better!!!
    break;
  }
}

void __attribute__((optimize("Os"))) dischargeAllCoils(void)
{
  //End all coil charges to ensure no stray sparks on startup
  for (uint8_t index=1; index<=IGN_CHANNELS; ++index)
  {
    endCoilCharge(index);
  }
}

void __attribute__((optimize("Os"))) initialiseIgnitionSchedules(statuses &current, const config2 &page2, config4 &page4, const config10 &page10)
{
  dischargeAllCoils();
  resetIgnitionSchedulers();
  initScheduleAngles(current, page2, page4);
  setCallbacks(page4.sparkMode, page2.nCylinders, page10.rotaryType);
}

TESTABLE_INLINE_STATIC bool isAnyIgnScheduleRunning(void) {
  return isRunning(ignitionSchedule1)      
#if IGN_CHANNELS >= 2 
      || isRunning(ignitionSchedule2)
#endif      
#if IGN_CHANNELS >= 3 
      || isRunning(ignitionSchedule3)
#endif      
#if IGN_CHANNELS >= 4       
      || isRunning(ignitionSchedule4)
#endif      
#if IGN_CHANNELS >= 5      
      || isRunning(ignitionSchedule5)
#endif
#if IGN_CHANNELS >= 6
      || isRunning(ignitionSchedule6)
#endif
#if IGN_CHANNELS >= 7
      || isRunning(ignitionSchedule7)
#endif
#if IGN_CHANNELS >= 8
      || isRunning(ignitionSchedule8)
#endif
      ;
}

static inline bool isSwitchableCylinderCount(const config2 &page2)
{
  return (page2.nCylinders==4U)
      || (page2.nCylinders==6U)
      || (page2.nCylinders==8U)
      ;
}

static inline bool isSemiSequentialIgnition(const config2 &page2, const config4 &page4, const decoder_status_t &decoderStatus)
{
  return (page4.sparkMode == IGN_MODE_SEQUENTIAL) 
      && isSwitchableCylinderCount(page2)
      && decoderStatus.syncStatus==SyncStatus::Partial;
}

static inline bool isFullSequentialIgnition(const config4 &page4, const decoder_status_t &decoderStatus)
{
  return (page4.sparkMode == IGN_MODE_SEQUENTIAL) 
      && decoderStatus.syncStatus==SyncStatus::Full;
}

TESTABLE_STATIC void changeIgnitionToHalfSync(const config2 &page2, statuses &current)
{
  ATOMIC()
  {
    if (!isAnyIgnScheduleRunning() && isSwitchableCylinderCount(page2)) {
      CRANK_ANGLE_MAX_IGN = 360;
      current.maxIgnOutputs = page2.nCylinders/2U;
      setCallbacks(IGN_MODE_WASTEDCOP, page2.nCylinders, 0U);
    }
  }
}

TESTABLE_STATIC void changeIgnitionToFullSequential(const config2 &page2, statuses &current)
{
  ATOMIC()
  {
    if (!isAnyIgnScheduleRunning() && isSwitchableCylinderCount(page2)) {
      CRANK_ANGLE_MAX_IGN = 720;
      current.maxIgnOutputs = min((uint8_t)IGN_CHANNELS, page2.nCylinders);
      setCallbacks(IGN_MODE_SEQUENTIAL, page2.nCylinders, 0U);
    }
  }
}

TESTABLE_INLINE_STATIC void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current)
{
  if (isFullSequentialIgnition(page4, current.decoder.getStatus()) && ( CRANK_ANGLE_MAX_IGN != 720 )) {
    changeIgnitionToFullSequential(page2, current);
  } else if(isSemiSequentialIgnition(page2, page4, current.decoder.getStatus()) && (CRANK_ANGLE_MAX_IGN != 360) ) { 
    changeIgnitionToHalfSync(page2, current);
  } else {
    // Ignition layout matches current sync - nothing to do but keep MISRA checker happy
  }
}

static inline int16_t _calculateSparkAngle(const IgnitionSchedule &schedule, int8_t advance) {
  int16_t angle = (int16_t)(schedule.channelDegrees==0U ? CRANK_ANGLE_MAX_IGN : schedule.channelDegrees) - advance;
  if(angle > CRANK_ANGLE_MAX_IGN) {angle -= CRANK_ANGLE_MAX_IGN;}
  return angle;
}

static inline int16_t _calculateCoilChargeAngle(uint16_t dwellAngle, int16_t dischargeAngle) {
  if (dischargeAngle>(int16_t)dwellAngle) {
    return dischargeAngle - (int16_t)dwellAngle;
  }
  return dischargeAngle + CRANK_ANGLE_MAX_IGN - (int16_t)dwellAngle;
}

TESTABLE_INLINE_STATIC void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance)
{
  schedule.dischargeAngle = _calculateSparkAngle(schedule,  advance);
  schedule.chargeAngle = _calculateCoilChargeAngle(dwellAngle, schedule.dischargeAngle);
}

TESTABLE_STATIC void calculateIgnitionTrailingRotary(IgnitionSchedule &leading, uint16_t dwellAngle, int16_t rotarySplitDegrees, IgnitionSchedule &trailing) 
{
  trailing.dischargeAngle = (int16_t)ignitionLimits(leading.dischargeAngle + rotarySplitDegrees);
  trailing.chargeAngle = (int16_t)ignitionLimits(trailing.dischargeAngle - (int16_t)dwellAngle); 
}

static inline void calculateRotaryIgnitionAngles(uint16_t dwellAngle, const statuses &current)
{
#if IGN_CHANNELS>=4
  calculateIgnitionAngles(ignitionSchedule1, dwellAngle, current.advance);
  calculateIgnitionAngles(ignitionSchedule2, dwellAngle, current.advance);
  uint8_t splitDegrees = table2D_getValue(&rotarySplitTable, (uint8_t)current.ignLoad);

  //The trailing angles are set relative to the leading ones
  calculateIgnitionTrailingRotary(ignitionSchedule1, dwellAngle, splitDegrees, ignitionSchedule3);
  calculateIgnitionTrailingRotary(ignitionSchedule2, dwellAngle, splitDegrees, ignitionSchedule4);
#endif
}

static inline void calculateNonRotaryIgnitionAngles(uint16_t dwellAngle, const statuses &current)
{
  switch (current.maxIgnOutputs)
  {
  case 8:
#if IGN_CHANNELS >= 8
    calculateIgnitionAngles(ignitionSchedule8, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 7:
#if IGN_CHANNELS >= 7
    calculateIgnitionAngles(ignitionSchedule7, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 6:
#if IGN_CHANNELS >= 6
    calculateIgnitionAngles(ignitionSchedule6, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 5:
#if IGN_CHANNELS >= 5
    calculateIgnitionAngles(ignitionSchedule5, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 4:
#if IGN_CHANNELS >= 4
    calculateIgnitionAngles(ignitionSchedule4, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 3:
#if IGN_CHANNELS >= 3
    calculateIgnitionAngles(ignitionSchedule3, dwellAngle, current.advance);
#endif
    [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 2:
#if IGN_CHANNELS >= 2
    calculateIgnitionAngles(ignitionSchedule2, dwellAngle, current.advance);
#endif
    break;
  default:
    // Do nothing
    break;
  }
  calculateIgnitionAngles(ignitionSchedule1, dwellAngle, current.advance);
}

/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
BEGIN_LTO_ALWAYS_INLINE(void) __attribute__((flatten)) calculateIgnitionAngles(const config2 &page2, const config4 &page4, statuses &current)
{
  matchIgnitionSchedulersToSyncState(page2, page4, current);

  uint16_t dwellAngle = timeToAngleDegPerMicroSec(current.dwell);

  if((current.maxIgnOutputs==4U) && (page4.sparkMode == IGN_MODE_ROTARY))
  {
    calculateRotaryIgnitionAngles(dwellAngle, current);
  }
  else
  {
    calculateNonRotaryIgnitionAngles(dwellAngle, current);
  }
  
  //If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
  if (page2.perToothIgn == true) { current.decoder.setEndTeeth(); }
}
END_LTO_INLINE()

TESTABLE_INLINE_STATIC void setIgnitionScheduleDuration(IgnitionSchedule &schedule, uint32_t delay, uint16_t duration) 
{
  // Only queue up the next schedule if the maximum time between sparks (Based on CRANK_ANGLE_MAX_IGN) is less than the max timer period
  setSchedule(schedule, delay, duration, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_IGN) < MAX_TIMER_PERIOD);
}

TESTABLE_INLINE_STATIC uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t crankAngle)
{
  return _calculateAngularTime(schedule, schedule.channelDegrees, schedule.chargeAngle, crankAngle, CRANK_ANGLE_MAX_IGN);
}

static inline void setIgnitionChannel(IgnitionSchedule &schedule, uint16_t crankAngle, uint16_t dwellDuration, byte channelMask, uint8_t channelIdx)
{
  if (BIT_CHECK(channelMask, (channelIdx)-1U)) {
    setIgnitionScheduleDuration(schedule, _calculateIgnitionTimeout(schedule, crankAngle), dwellDuration);
  }
}

BEGIN_LTO_ALWAYS_INLINE(void) __attribute__((flatten)) setIgnitionChannels(const statuses &current, uint16_t crankAngle, uint16_t dwellTime) {
  crankAngle = ignitionLimits(crankAngle);
  #define SET_IGNITION_CHANNEL(channelIdx) setIgnitionChannel(ignitionSchedule ##channelIdx, crankAngle, dwellTime, current.schedulerCutState.ignitionChannels, channelIdx);

  SET_IGNITION_CHANNEL(1)
#if IGN_CHANNELS >= 2
  SET_IGNITION_CHANNEL(2)
#endif
#if IGN_CHANNELS >= 3
  SET_IGNITION_CHANNEL(3)
#endif
#if IGN_CHANNELS >= 4
  SET_IGNITION_CHANNEL(4)
#endif
#if IGN_CHANNELS >= 5
  SET_IGNITION_CHANNEL(5)
#endif
#if IGN_CHANNELS >= 6
  SET_IGNITION_CHANNEL(6)
#endif
#if IGN_CHANNELS >= 7
  SET_IGNITION_CHANNEL(7)
#endif
#if IGN_CHANNELS >= 8
  SET_IGNITION_CHANNEL(8)
#endif

#undef SET_IGNITION_CHANNEL
}
END_LTO_INLINE()

TESTABLE_INLINE_STATIC void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t targetOverdwellTime) {
  //Check first whether each spark output is currently on. Only check it's dwell time if it is
  ATOMIC() {
    if (isRunning(schedule) && (schedule._startTime < targetOverdwellTime)) { 
      moveToNextState(schedule); //Call the end function to disable the spark output
    }
  }
}

TESTABLE_INLINE_STATIC bool isOverDwellActive(const config4 &page4, const statuses &current){
  bool isCrankLocked = page4.ignCranklock && (current.RPM < current.crankRPM); //Dwell limiter is disabled during cranking on setups using the locked cranking timing. WE HAVE to do the RPM check here as relying on the engine cranking bit can be potentially too slow in updating
  return (page4.useDwellLim) && !isCrankLocked;
}

// LCOV_EXCL_START
// The lower level function should be tested, so this can be excluded from coverage
void applyOverDwellProtection(const config4 &page4, const statuses &current)
{
  if (isOverDwellActive(page4, current)) {
    uint32_t targetOverdwellTime = micros() - (page4.dwellLimit * 1000U); //Convert to uS

    applyChannelOverDwellProtection(ignitionSchedule1, targetOverdwellTime);
#if IGN_CHANNELS >= 2
    applyChannelOverDwellProtection(ignitionSchedule2, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 3
    applyChannelOverDwellProtection(ignitionSchedule3, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 4
    applyChannelOverDwellProtection(ignitionSchedule4, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 5
    applyChannelOverDwellProtection(ignitionSchedule5, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 6
    applyChannelOverDwellProtection(ignitionSchedule6, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 7
    applyChannelOverDwellProtection(ignitionSchedule7, targetOverdwellTime);
#endif
#if IGN_CHANNELS >= 8
    applyChannelOverDwellProtection(ignitionSchedule8, targetOverdwellTime);
#endif
  }
}
// LCOV_EXCL_STOP

void __attribute__((optimize("Os"))) startIgnitionSchedulers(void)
{
  IGN1_TIMER_ENABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 5
  IGN5_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 6
  IGN6_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_ENABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_ENABLE();
#endif  
}

void __attribute__((optimize("Os"))) stopIgnitionSchedulers(void)
{
  IGN1_TIMER_DISABLE();
#if IGN_CHANNELS >= 2
  IGN2_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 3
  IGN3_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 4
  IGN4_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 5
  IGN5_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 6
  IGN6_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 7
  IGN7_TIMER_DISABLE();
#endif
#if IGN_CHANNELS >= 8
  IGN8_TIMER_DISABLE();
#endif  
}
