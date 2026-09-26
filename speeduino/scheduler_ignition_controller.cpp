#include "scheduler_ignition_controller.h"
#include "scheduledIO_ign.h"
#include "elapsed_time.h"
#include "scheduledIO_ign.h"
#include "globals.h"
#include "unit_testing.h"
#include "prog_mem_support.h"
#include "src/utils/static_for.hpp"

IgnitionSchedule ignitionSchedules[IGN_CHANNELS] = {
  IgnitionSchedule(IGN1_COUNTER, IGN1_COMPARE),
#if IGN_CHANNELS >= 2
  IgnitionSchedule(IGN2_COUNTER, IGN2_COMPARE),
#endif
#if IGN_CHANNELS >= 3
  IgnitionSchedule(IGN3_COUNTER, IGN3_COMPARE),
#endif
#if IGN_CHANNELS >= 4
  IgnitionSchedule(IGN4_COUNTER, IGN4_COMPARE),
#endif
#if IGN_CHANNELS >= 5
  IgnitionSchedule(IGN5_COUNTER, IGN5_COMPARE),
#endif
#if IGN_CHANNELS >= 6
  IgnitionSchedule(IGN6_COUNTER, IGN6_COMPARE),
#endif
#if IGN_CHANNELS >= 7
  IgnitionSchedule(IGN7_COUNTER, IGN7_COMPARE),
#endif
#if IGN_CHANNELS >= 8
  IgnitionSchedule(IGN8_COUNTER, IGN8_COMPARE),
#endif
};

constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);

static inline int8_t constrainAdvanceTrim(int16_t advance)
{
  return (int8_t)clamp(advance, (int16_t)INT8_MIN, (int16_t)INT8_MAX);
}

static inline int8_t getIgnitionTrimmedAdvance(const config13 &page13, int8_t baseAdvance, uint8_t channelIndex)
{
  return constrainAdvanceTrim((int16_t)baseAdvance + (int16_t)page13.ignTrim[channelIndex]);
}

static __attribute__((optimize("Os"))) void setCallbacksP(const Schedule::callback_pair_t *begin, const Schedule::callback_pair_t *end)
{
  for (auto& schedule: ignitionSchedules) {
    if (begin!=end) {
      schedule.setCallbacks(copyObject_P(begin));
      ++begin;
    }
  }
}

static void __attribute__((optimize("Os"))) setSequentialCallbacks(uint8_t numChannels)
{
  static const Schedule::callback_pair_t callbacks[] PROGMEM = {
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil2Charge, endCoil2Charge },
    { beginCoil3Charge, endCoil3Charge },
    { beginCoil4Charge, endCoil4Charge },
    { beginCoil5Charge, endCoil5Charge },
    { beginCoil6Charge, endCoil6Charge },
    { beginCoil7Charge, endCoil7Charge },
    { beginCoil8Charge, endCoil8Charge },
  };
  numChannels = (std::min)(numChannels, (uint8_t)_countof(ignitionSchedules));
  setCallbacksP(callbacks, callbacks+numChannels);
}

static void __attribute__((optimize("Os"))) setWastedSparkCallbacks(void)
{
  setSequentialCallbacks(5U);
}

static void __attribute__((optimize("Os"))) setSingleChannelCallbacks(void)
{
  //Single channel mode. All ignition pulses are on channel 1
  constexpr Schedule::callback_pair_t callbacks = { beginCoil1Charge, endCoil1Charge };
  for (auto& schedule: ignitionSchedules) {
    schedule.setCallbacks(callbacks);
  }
}

static void __attribute__((optimize("Os"))) set4CylinderWastedCOPCallbacks(void)
{
  static const Schedule::callback_pair_t callbacks[] PROGMEM = {
    { beginCoil1and3Charge, endCoil1and3Charge },
    { beginCoil2and4Charge, endCoil2and4Charge },
  };
  setCallbacksP(std::begin(callbacks), std::end(callbacks));
}

static void __attribute__((optimize("Os"))) set6CylinderWastedCOPCallbacks(void)
{
  //Wasted COP mode for 6 cylinders. Ignition channels 1&4, 2&5 and 3&6 are paired together
  static const Schedule::callback_pair_t callbacks[] PROGMEM = {
    { beginCoil1and4Charge, endCoil1and4Charge },
    { beginCoil2and5Charge, endCoil2and5Charge },
    { beginCoil3and6Charge, endCoil3and6Charge },
  };
  setCallbacksP(std::begin(callbacks), std::end(callbacks));
}

static void __attribute__((optimize("Os"))) set8CylinderWastedCOPCallbacks(void)
{
  //Wasted COP mode for 8 cylinders. Ignition channels 1&5, 2&6, 3&7 and 4&8 are paired together
  static const Schedule::callback_pair_t callbacks[] PROGMEM = {
    { beginCoil1and5Charge, endCoil1and5Charge },
    { beginCoil2and6Charge, endCoil2and6Charge },
    { beginCoil3and7Charge, endCoil3and7Charge },
    { beginCoil4and8Charge, endCoil4and8Charge },
  };
  setCallbacksP(std::begin(callbacks), std::end(callbacks));
}

static void __attribute__((optimize("Os"))) setWastedCOPCallbacks(uint8_t numCylinders)
{
  //Wasted COP mode. Note, most of the boards can only run this for 4-cyl only.
  switch (numCylinders)
  {
  case 4: set4CylinderWastedCOPCallbacks(); break;
  case 6: set6CylinderWastedCOPCallbacks(); break;
  case 8: set8CylinderWastedCOPCallbacks(); break;
  default: setWastedSparkCallbacks(); break;
  }
}

static void __attribute__((optimize("Os"))) setRotaryFcCallbacks(void)
{
  //Ignition channel 1 is a wasted spark signal for leading signal on both rotors
  static const Schedule::callback_pair_t callbacks[] PROGMEM = {
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    { beginTrailingCoilCharge, endTrailingCoilCharge1 },
    { beginTrailingCoilCharge, endTrailingCoilCharge2 },
  };
  setCallbacksP(std::begin(callbacks), std::end(callbacks));
}

static void __attribute__((optimize("Os"))) setRotaryFdCallbacks(void)
{
  static const Schedule::callback_pair_t callbacks[] PROGMEM = {
    { beginCoil1Charge, endCoil1Charge },
    { beginCoil1Charge, endCoil1Charge },
    //Trailing coils have their own channel each
    //IGN2 = front rotor trailing spark
    { beginCoil2Charge, endCoil2Charge },
    //IGN3 = rear rotor trailing spark
    { beginCoil3Charge, endCoil3Charge },
  };
  setCallbacksP(std::begin(callbacks), std::end(callbacks));
}

static void __attribute__((optimize("Os"))) setRotaryCallbacks(uint8_t rotaryType)
{
  switch (rotaryType)
  {
  case ROTARY_IGN_FC: setRotaryFcCallbacks(); break;
  case ROTARY_IGN_FD: setRotaryFdCallbacks(); break;
  // RX8 outputs are simply 1 coil and 1 output per plug
  case ROTARY_IGN_RX8: setSequentialCallbacks(4U); break;
  // LCOV_EXCL_BR_START
  default:
    //No action for other RX ignition modes (Future expansion / MISRA compliant). 
    break;
  // LCOV_EXCL_BR_STOP
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
  // LCOV_EXCL_BR_START
  default:
    setWastedSparkCallbacks(); break;
  // LCOV_EXCL_BR_STOP
  }
}

TESTABLE_STATIC void __attribute__((optimize("Os"))) resetIgnitionSchedulers(void)
{
  for (auto& schedule: ignitionSchedules) {
    schedule.reset();
  }  
}

void __attribute__((optimize("Os"))) stopAllCoilsCharging(void)
{
  for (uint8_t index=1; index<=_countof(ignitionSchedules); ++index)
  {
    endCoilCharge(index);
  }
}

static void __attribute__((optimize("Os"))) setOddfireScheduleAngles(const config2 &page2)
{
  ignitionSchedules[0].channelDegrees = 0;
  for (uint8_t i = 0; i < (std::min)(_countof(ignitionSchedules)-1U, _countof(page2.oddfire)); i++)
  {
    ignitionSchedules[i+1U].channelDegrees = page2.oddfire[i];
  }
}

static void __attribute__((optimize("Os"))) setRotaryScheduleAngles(void)
{
#if IGN_CHANNELS >= 4
  ignitionSchedules[0].channelDegrees = 0;
  ignitionSchedules[1].channelDegrees = 180;
//Rotary uses the ign 3 and 4 schedules for the trailing spark. They are offset from the ign 1 and 2 channels respectively and so use the same degrees as them
  ignitionSchedules[2].channelDegrees = ignitionSchedules[0].channelDegrees;
  ignitionSchedules[3].channelDegrees = ignitionSchedules[1].channelDegrees;
#endif
}

static void __attribute__((optimize("Os"))) setEvenfireScheduleAngles(const statuses &current)
{
  // LCOV_EXCL_START
  INTERNAL_TEST_ASSERT(current.maxIgnOutputs!=0);
  // LCOV_EXCL_STOP

  uint16_t interCylinderAngle = CRANK_ANGLE_MAX_IGN/current.maxIgnOutputs;
  for (uint8_t i = 0; i < _countof(ignitionSchedules); i++)
  {
    ignitionSchedules[i].channelDegrees = i*interCylinderAngle;
  }
}

static void __attribute__((optimize("Os"))) initScheduleAngles(const statuses &current, const config2 &page2, const config4 &page4)
{
  if (page2.engineType == ODD_FIRE)
  {
    setOddfireScheduleAngles(page2);
  }
  else if (page4.sparkMode==IGN_MODE_ROTARY)
  {
    setRotaryScheduleAngles();
  }
  else
  {
    setEvenfireScheduleAngles(current);
  }
}

TESTABLE_STATIC __attribute__((optimize("Os"))) uint8_t validateSparkMode(uint8_t mode, const config2 &page2)
{
  if (mode == IGN_MODE_SEQUENTIAL) {
        // Sequential only applies if enough channels.
    if ((page2.nCylinders>IGN_CHANNELS) 
        // And 4 stroke
     || (page2.strokes!=FOUR_STROKE)) {
      mode = IGN_MODE_WASTED;
    }
  }

  if (mode == IGN_MODE_ROTARY) {
    // Rotary is only supported on 4 "cylinder"
    if (page2.nCylinders!=4U) {
      mode = IGN_MODE_WASTED;
    }
  }

  if (mode == IGN_MODE_WASTEDCOP) {
    // Wasted COP is only supported on 4-/6-/8- cylinder
    if ((page2.nCylinders!=4U)
     && (page2.nCylinders!=6U)
     && (page2.nCylinders!=8U)) {
      mode = IGN_MODE_WASTED;
    }
  }
  
  return mode;
}

TESTABLE_STATIC __attribute__((optimize("Os"))) void validateIgnitionSetup(config2 &page2, config4 &page4, config13 &page13)
{
  // Can't have zero cylinders!
  page2.nCylinders = (std::max)((uint8_t)1, (uint8_t)page2.nCylinders);

  page4.sparkMode = validateSparkMode(page4.sparkMode, page2);

  // Oddfire only supported on up to number of oddfire angles
  if ((page2.engineType == ODD_FIRE) && (page2.nCylinders>_countof(page2.oddfire)+1U))
  {
    page2.engineType = EVEN_FIRE;
  }

  // Ignition trims are only applied in sequential mode.
  if (page4.sparkMode!=IGN_MODE_SEQUENTIAL)
  {
    std::fill(page13.ignTrim, page13.ignTrim+_countof(page13.ignTrim), 0);
  }

  if (page4.sparkMode == IGN_MODE_ROTARY)
  {
    // Force Going Low ignition mode (Going high is never used for rotary)
    page4.IgInv = GOING_LOW; 
    // Rotary must be 4 stroke...
    page2.strokes = FOUR_STROKE;
    // ...even fire.
    page2.engineType = EVEN_FIRE;
  }
}

static inline bool isSequential720(const config4 &page4)
{
  return (page4.sparkMode == IGN_MODE_SEQUENTIAL);
}

static inline bool isSingle720(const config2 &page2, const config4 &page4)
{
  return (page2.strokes == FOUR_STROKE)
      && (page4.sparkMode == IGN_MODE_SINGLE)
      && ((page2.nCylinders==3U) || (page2.nCylinders==5U))
      ;
}

static inline bool is720(const config2 &page2, const config4 &page4)
{
  return isSequential720(page4)
      || isSingle720(page2, page4)
  ;
}

static __attribute__((optimize("Os"))) uint16_t calculateMaxIgnCrankAngle(const config2 &page2, const config4 &page4)
{
  return is720(page2, page4) ? 720 : 360;
}

static __attribute__((optimize("Os"))) uint8_t calculateMaxIgnChannels(const config2 &page2, const config4 &page4)
{
  if ((page2.nCylinders<4)
   || (page2.nCylinders==5)
   || (page2.engineType == ODD_FIRE)
   || (page4.sparkMode == IGN_MODE_SEQUENTIAL)
   || (page4.sparkMode==IGN_MODE_ROTARY))
  {
    return page2.nCylinders;
  }
  // LCOV_EXCL_START
  INTERNAL_TEST_ASSERT(page2.nCylinders%2==0);
  // LCOV_EXCL_STOP
  return page2.nCylinders/2;
}

void __attribute__((optimize("Os"))) initialiseIgnitionSchedules(statuses &current, config2 &page2, config4 &page4, const config10 &page10, config13 &page13, const pinNumbers_t &pins)
{
  initialiseIgnitionIO(page4, pins);
  stopAllCoilsCharging();

  resetIgnitionSchedulers();

  validateIgnitionSetup(page2, page4, page13);

  CRANK_ANGLE_MAX_IGN = calculateMaxIgnCrankAngle(page2, page4);
  current.maxIgnOutputs = calculateMaxIgnChannels(page2, page4);

  initScheduleAngles(current, page2, page4);
  setCallbacks(page4.sparkMode, page2.nCylinders, page10.rotaryType);
}

TESTABLE_INLINE_STATIC bool isAnyIgnScheduleRunning(void) {
  return std::any_of(std::begin(ignitionSchedules), std::end(ignitionSchedules), isRunning);
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
      current.maxIgnOutputs = (std::min)((uint8_t)IGN_CHANNELS, page2.nCylinders);
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
  calculateIgnitionAngles(ignitionSchedules[0], dwellAngle, current.advance);
  calculateIgnitionAngles(ignitionSchedules[1], dwellAngle, current.advance);
  uint8_t splitDegrees = table2D_getValue(&rotarySplitTable, (uint8_t)current.ignLoad);

  //The trailing angles are set relative to the leading ones
  calculateIgnitionTrailingRotary(ignitionSchedules[0], dwellAngle, splitDegrees, ignitionSchedules[2]);
  calculateIgnitionTrailingRotary(ignitionSchedules[1], dwellAngle, splitDegrees, ignitionSchedules[3]);
#endif
}

static inline void calculateNonRotaryIgnitionAngles(const config4 &page4, const config13 &page13, uint16_t dwellAngle, const statuses &current)
{
  const bool useIndividualTrim = isFullSequentialIgnition(page4, current.decoder.getStatus());
  for (uint8_t i = 0; i < current.maxIgnOutputs; i++)
  {
    calculateIgnitionAngles(ignitionSchedules[i], dwellAngle, useIndividualTrim ? getIgnitionTrimmedAdvance(page13, current.advance, i) : current.advance);
  }
}

/** Calculate the Ignition angles for all cylinders (based on @ref config2.nCylinders).
 * both start and end angles are calculated for each channel.
 * Also the mode of ignition firing - wasted spark vs. dedicated spark per cyl. - is considered here.
 */
BEGIN_LTO_ALWAYS_INLINE(void) __attribute__((flatten)) calculateIgnitionAngles(const config2 &page2, const config4 &page4, const config13 &page13, statuses &current)
{
  matchIgnitionSchedulersToSyncState(page2, page4, current);

  uint16_t dwellAngle = timeToAngle(current.dwell);

  if((current.maxIgnOutputs==4U) && (page4.sparkMode == IGN_MODE_ROTARY))
  {
    calculateRotaryIgnitionAngles(dwellAngle, current);
  }
  else
  {
    calculateNonRotaryIgnitionAngles(page4, page13, dwellAngle, current);
  }
  
  //If ignition timing is being tracked per tooth, perform the calcs to get the end teeth
  if (page2.perToothIgn) { current.decoder.setEndTeeth(); }
}
END_LTO_INLINE()

TESTABLE_INLINE_STATIC void setIgnitionScheduleDuration(IgnitionSchedule &schedule, uint32_t delay, uint16_t duration) 
{
  // Only queue up the next schedule if the maximum time between sparks (Based on CRANK_ANGLE_MAX_IGN) is less than the max timer period
  setSchedule(schedule, delay, duration, angleToTime((uint16_t)CRANK_ANGLE_MAX_IGN) < MAX_TIMER_PERIOD);
}

TESTABLE_INLINE_STATIC uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t crankAngle)
{
  return _calculateAngularTime(schedule, schedule.channelDegrees, schedule.chargeAngle, crankAngle, CRANK_ANGLE_MAX_IGN);
}

// Fixed cranking override is used to extend the dwell during cranking so that the decoder 
// can trigger the spark upon seeing a certain tooth. 
static uint16_t applyFixedCrankingOverride(const statuses &current, const config4 &page4)
{
  uint16_t dwellAdjust = 0;
  if ( current.isFixedCrankingIgnitionTimingActive(page4))
  {
    dwellAdjust = current.dwell * 3;

    // This is a safety step to prevent the ignition start time occurring AFTER the target tooth pulse has already occurred.
    // It simply moves the start time forward a little, which is compensated for by the increase in the dwell time
    if(current.RPM < 250) // Why 250?
    {
      for (auto& schedule: ignitionSchedules) {
        schedule.chargeAngle -= 5;
      }
    }
  }

  return dwellAdjust;
}

BEGIN_LTO_ALWAYS_INLINE(void) __attribute__((flatten)) setIgnitionChannels(const statuses &current, const config4 &page4, uint16_t crankAngle) {
  crankAngle = ignitionLimits(crankAngle);
  uint16_t dwellTime = current.dwell + applyFixedCrankingOverride(current, page4);
  auto channelMask = current.schedulerCutState.ignitionChannels;

  auto setDuration = [crankAngle, dwellTime, channelMask](uint8_t i) __attribute__((always_inline)) {
    if (BIT_CHECK(channelMask, i)) {
      setIgnitionScheduleDuration(ignitionSchedules[i], _calculateIgnitionTimeout(ignitionSchedules[i], crankAngle), dwellTime);
    }
  };
  static_for<_countof(ignitionSchedules)>(setDuration);
}
END_LTO_INLINE()

TESTABLE_INLINE_STATIC void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t now, uint32_t dwellLimit_uS) {
    if (isRunning(schedule) && hasIntervalElapsed(now, schedule._startTime, dwellLimit_uS)) {
      moveToNextState(schedule); //Call the end function to disable the spark output
    }
}

// LCOV_EXCL_START
// The lower level function should be tested, so this can be excluded from coverage
static void applyChannelOverDwellProtection(IgnitionSchedule &schedule, uint32_t dwellLimit_uS) {
  //Check first whether each spark output is currently on. Only check it's dwell time if it is
  ATOMIC() {
    uint32_t now = micros(); // This **must** be inside the atomic block to avoid a race. See #1581
    applyChannelOverDwellProtection(schedule, now, dwellLimit_uS);
  }
}
// LCOV_EXCL_STOP

TESTABLE_INLINE_STATIC bool isOverDwellActive(const config4 &page4, const statuses &current){
  bool isCrankLocked = page4.ignCranklock && (current.RPM < current.crankRPM); //Dwell limiter is disabled during cranking on setups using the locked cranking timing. WE HAVE to do the RPM check here as relying on the engine cranking bit can be potentially too slow in updating
  return (page4.useDwellLim) && !isCrankLocked;
}

// LCOV_EXCL_START
// The lower level function should be tested, so this can be excluded from coverage
void applyOverDwellProtection(const config4 &page4, const statuses &current)
{
  if (isOverDwellActive(page4, current)) {
    uint32_t dwellLimit_uS = page4.dwellLimit * 1000U; //Convert to uS

    for (auto& schedule: ignitionSchedules) {
      applyChannelOverDwellProtection(schedule, dwellLimit_uS);
    }
  }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
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
// LCOV_EXCL_STOP

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
