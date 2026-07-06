#include "scheduler_fuel_controller.h"
#include "scheduledIO_inj.h"
#include "units.h"
#include "table2d.h"
#include "globals.h"

FuelSchedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE); //cppcheck-suppress misra-c2012-8.4
#if (INJ_CHANNELS >= 2)
FuelSchedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 3)
FuelSchedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 4)
FuelSchedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 5)
FuelSchedule fuelSchedule5(FUEL5_COUNTER, FUEL5_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 6)
FuelSchedule fuelSchedule6(FUEL6_COUNTER, FUEL6_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 7)
FuelSchedule fuelSchedule7(FUEL7_COUNTER, FUEL7_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif
#if (INJ_CHANNELS >= 8)
FuelSchedule fuelSchedule8(FUEL8_COUNTER, FUEL8_COMPARE); //cppcheck-suppress misra-c2012-8.4
#endif

static __attribute__((optimize("Os"))) void setupSequentialCallbacks(void)
{
  #define SET_CALLBACKS(index) setCallbacks(fuelSchedule ## index, openInjector ## index, closeInjector ## index);\
  
  SET_CALLBACKS(1)
#if INJ_CHANNELS >= 2
  SET_CALLBACKS(2)
#endif
#if INJ_CHANNELS >= 3
  SET_CALLBACKS(3)
#endif
#if INJ_CHANNELS >= 4
  SET_CALLBACKS(4)
#endif
#if INJ_CHANNELS >= 5
  SET_CALLBACKS(5)
#endif
#if INJ_CHANNELS >= 6
  SET_CALLBACKS(6)
#endif
#if INJ_CHANNELS >= 7
  SET_CALLBACKS(7)
#endif
#if INJ_CHANNELS >= 8
  SET_CALLBACKS(8)
#endif
}

static __attribute__((optimize("Os"))) void setupPairedCallbacks(void)
{
  setupSequentialCallbacks();
}

static __attribute__((optimize("Os"))) void setupSemiSequentialCallbacks(uint8_t nCylinders, uint8_t inj4cylPairing)
{
  //Semi-Sequential injection. Currently possible with 4, 6 and 8 cylinders. 5 cylinder is a special case
  if( nCylinders == 4 )
  {
    if(inj4cylPairing == INJ_PAIR_13_24)
    {
      setCallbacks(fuelSchedule1, openInjector1and3, closeInjector1and3);
#if (INJ_CHANNELS >= 2)
      setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
#endif
    }
    else
    {
      setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
#if (INJ_CHANNELS >= 2)
      setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
#endif
    }
  }
  else if( nCylinders == 5 ) //This is similar to the paired injection but uses five injector outputs instead of four
  {
    if (INJ_CHANNELS>=5U)
    {
      setupSequentialCallbacks();
    }
    else
    {
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
#if (INJ_CHANNELS >= 2)
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
#endif
#if (INJ_CHANNELS >= 3)
      setCallbacks(fuelSchedule3, openInjector3and5, closeInjector3and5);
#endif
#if (INJ_CHANNELS >= 4)
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
#endif
    }
  }
  else if( nCylinders == 6 )
  {
    setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
#if (INJ_CHANNELS >= 2)
    setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
#endif
#if (INJ_CHANNELS >= 3)
    setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
#endif
  }
  else if( nCylinders == 8 )
  {
    setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
#if (INJ_CHANNELS >= 2)
    setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
#endif
#if (INJ_CHANNELS >= 3)
    setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
#endif
#if (INJ_CHANNELS >= 4)
    setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
#endif
  }
  else
  {
    setupPairedCallbacks();
  }
}

static __attribute__((optimize("Os"))) void setupCallbacks(uint8_t injLayout, uint8_t nCylinders, uint8_t inj4cylPairing)
{
  switch(injLayout)
  {
  default:
  case INJ_PAIRED: setupPairedCallbacks(); break;
  case INJ_SEMISEQUENTIAL: setupSemiSequentialCallbacks(nCylinders, inj4cylPairing); break;
  case INJ_SEQUENTIAL: setupSequentialCallbacks(); break;
  }
}

TESTABLE_INLINE_STATIC bool isAnyFuelScheduleRunning(void) {
  return isRunning(fuelSchedule1)
#if (INJ_CHANNELS >= 2)
      || isRunning(fuelSchedule2)
#endif
#if (INJ_CHANNELS >= 3)
      || isRunning(fuelSchedule3)
#endif
#if (INJ_CHANNELS >= 4)
      || isRunning(fuelSchedule4)
#endif
#if INJ_CHANNELS >= 5      
      || isRunning(fuelSchedule5)
#endif
#if INJ_CHANNELS >= 6
      || isRunning(fuelSchedule6)
#endif
#if INJ_CHANNELS >= 7
      || isRunning(fuelSchedule7)
#endif
#if INJ_CHANNELS >= 8
      || isRunning(fuelSchedule8)
#endif
      ;
}

TESTABLE_STATIC table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);

TESTABLE_INLINE_STATIC uint16_t lookupInjectorAngle(const statuses &current)
{
  uint16_t injAngle = table2D_getValue(&injectorAngleTable, current.RPMdiv100);
  // Do not combine min() & table2D_getValue() - if min() is a macro, we could call table2D_getValue twice
  return min(uint16_t(CRANK_ANGLE_MAX_INJ), injAngle);
}

TESTABLE_INLINE_STATIC uint16_t updatePwAngleCache(uint16_t pw, injectorAngleCalcCache *pCache) {
  // We can afford to be a bit loose updating the cache since injection timing doesn't 
  // need to be precise (the PW calcs liberally use approximations)
  //
  // 1% of a revolution at max RPM should be plenty accurate.
  constexpr int16_t PW_DELTA_THRESHOLD = MIN_REVOLUTION_TIME/100U; // in µS
  if (abs((int16_t)pCache->pw-(int16_t)pw)>PW_DELTA_THRESHOLD) {
    pCache->pwDegrees = timeToAngle(pw);
    pCache->pw = pw;
  }
  return pCache->pwDegrees;
}

/**
 * @brief Compute the injector open angle for an injection channel
 * 
 * @param pwDegrees How many crank degrees the calculated PW will take at the current speed
 * @param tdcOffset The number of crank degrees until cylinder is at TDC (at rest)
 * @param injAngle The requested injection angle
 * @return uint16_t 
 */
TESTABLE_INLINE_STATIC uint16_t _calculateOpenAngle(FuelSchedule &schedule, uint16_t pwDegrees, uint16_t injAngle)
{
  // 0<=injAngle<=720°
  // 0<=injChannelDegrees<=720°
  // 0<pwDegrees<=??? (could be many crank rotations in the worst case!)
  // 45<=CRANK_ANGLE_MAX_INJ<=720
  // (CRANK_ANGLE_MAX_INJ can be as small as 360/nCylinders. E.g. 45° for 8 cylinder)

  uint16_t startAngle = injAngle + schedule.channelDegrees;
  
  while (startAngle<pwDegrees) { startAngle = startAngle + (uint16_t)CRANK_ANGLE_MAX_INJ; } // Avoid underflow
  startAngle = startAngle - pwDegrees; // startAngle guaranteed to be >=0.
  while (startAngle>=(uint16_t)CRANK_ANGLE_MAX_INJ) { startAngle = startAngle - (uint16_t)CRANK_ANGLE_MAX_INJ; } // Clamp to 0<=startAngle<=CRANK_ANGLE_MAX_INJ

  return startAngle;
}

/**
 * @brief Calculate the time in uS from now to when the injector should be opened.
 * 
 * @param schedule The ignition channel
 * @param openAngle The angle at which to open the injector
 * @param crankAngle The current crank angle
 * @return uint32_t 
 */
TESTABLE_INLINE_STATIC uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int16_t crankAngle, uint16_t openAngle)
{
  int16_t delta = openAngle - crankAngle;

  if (delta<0)
  {
    if (schedule._status != PENDING)
    {
      while(delta < 0) { delta += CRANK_ANGLE_MAX_INJ; }
    }
    else
    {
      delta = 0;
      return 0U;
    }
  }
  return angleToTime((uint16_t)delta);
}

TESTABLE_INLINE_STATIC void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask, uint16_t injAngle, injectorAngleCalcCache *pCache) noexcept
{
  if( (schedule.pw != 0U) && (BIT_CHECK(injChannelMask, channel-1U)) )
  {
    uint32_t timeOut = calculateInjectorTimeout(schedule, crankAngle, 
                                                _calculateOpenAngle(schedule, updatePwAngleCache(schedule.pw, pCache), injAngle));
    if (timeOut>0U)
    {
      // Only queue up the next schedule if the maximum time between squirts (Based on CRANK_ANGLE_MAX_INJ) is less than the max timer period
      setSchedule(schedule, timeOut, schedule.pw, angleToTime((uint16_t)CRANK_ANGLE_MAX_INJ) < MAX_TIMER_PERIOD);
    }
  }
}

TESTABLE_INLINE_STATIC uint16_t setFuelChannelSchedules(uint16_t crankAngle, byte injChannelMask, uint16_t injAngle)
{
  injectorAngleCalcCache angleCalcCache;
#define SET_FUEL_CHANNEL(channel) \
  setFuelChannelSchedule(fuelSchedule ##channel, UINT8_C(channel), crankAngle, injChannelMask, injAngle, &angleCalcCache);

  SET_FUEL_CHANNEL(1)
#if INJ_CHANNELS >= 2
  SET_FUEL_CHANNEL(2)
#endif
#if INJ_CHANNELS >= 3
  SET_FUEL_CHANNEL(3)
#endif
#if INJ_CHANNELS >= 4
  SET_FUEL_CHANNEL(4)
#endif
#if INJ_CHANNELS >= 5
  SET_FUEL_CHANNEL(5)
#endif
#if INJ_CHANNELS >= 6
  SET_FUEL_CHANNEL(6)
#endif
#if INJ_CHANNELS >= 7
  SET_FUEL_CHANNEL(7)
#endif
#if INJ_CHANNELS >= 8
  SET_FUEL_CHANNEL(8)
#endif

#undef SET_FUEL_CHANNEL

  return injAngle;
}

/** @brief Clamp the angle to within [0,CRANK_ANGLE_MAX_INJ] */
TESTABLE_INLINE_STATIC uint16_t injectorLimits(uint16_t angle)
{
    while(angle >= (uint16_t)CRANK_ANGLE_MAX_INJ ) { angle -= (uint16_t)CRANK_ANGLE_MAX_INJ; }
    return angle;
}

// LCOV_EXCL_START
BEGIN_LTO_ALWAYS_INLINE(uint16_t) __attribute__((flatten)) setFuelChannelSchedules(const statuses &current)
{
  return setFuelChannelSchedules(
    injectorLimits(current.decoder.getCrankAngle()),
    current.schedulerCutState.fuelChannels,
    lookupInjectorAngle(current));
}
// LCOV_EXCL_STOP

static inline uint16_t applyFuelTrim(const table3d6RpmLoad &trimTable, uint16_t pw, const config6 &page6, const statuses &current)
{
  if (pw!=0U && (page6.fuelTrimEnabled))
  {
    int8_t trimPct = FUEL_TRIM.toUser(get3DTableValue(&trimTable, current.fuelLoad, current.RPM));
    if (trimPct != 0) 
    { 
      pw = percentageApprox((uint8_t)(100+trimPct), pw); 
    }
  }

  return pw;
}

static inline void assignPrimaryPws(const pulseWidths &pulse_widths, const config6 &page6, const statuses &current)
{
  #define ASSIGN_PRIMARY_PW(index) fuelSchedule ## index .pw = applyFuelTrim(trimTables[index-1U], pulse_widths.primary, page6, current);

  switch (current.injOutputs.primary)
  {
  case 8:
#if INJ_CHANNELS >= 8
   ASSIGN_PRIMARY_PW(8);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 7:
#if INJ_CHANNELS >= 7
   ASSIGN_PRIMARY_PW(7);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 6:
#if INJ_CHANNELS >= 6
   ASSIGN_PRIMARY_PW(6);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 5:
#if INJ_CHANNELS >= 5
   ASSIGN_PRIMARY_PW(5);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 4:
#if INJ_CHANNELS >= 4
   ASSIGN_PRIMARY_PW(4);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 3:
#if INJ_CHANNELS >= 3
   ASSIGN_PRIMARY_PW(3);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 2:
#if INJ_CHANNELS >= 2
   ASSIGN_PRIMARY_PW(2);
#endif
  [[gnu::fallthrough]];
  //cppcheck-suppress misra-c2012-16.3
  case 1:
  default:
    ASSIGN_PRIMARY_PW(1);
    break;
  }
#undef ASSIGN_PRIMARY_PW
}

static inline void assignSecondaryPws(const pulseWidths &pulse_widths, const statuses &current)
{
  #define ASSIGN_SECONDARY_PW(index) \
    if (current.injOutputs.isSecondaryInjector(index)) \
    { \
      fuelSchedule ## index .pw = pulse_widths.secondary; \
    }

  if (current.injOutputs.secondary>0U)
  {
#if INJ_CHANNELS >= 2
    ASSIGN_SECONDARY_PW(2);
#endif
#if INJ_CHANNELS >= 3
    ASSIGN_SECONDARY_PW(3);
#endif
#if INJ_CHANNELS >= 4
    ASSIGN_SECONDARY_PW(4);
#endif
#if INJ_CHANNELS >= 5
    ASSIGN_SECONDARY_PW(5);
#endif
#if INJ_CHANNELS >= 6
    ASSIGN_SECONDARY_PW(6);
#endif
#if INJ_CHANNELS >= 7
    ASSIGN_SECONDARY_PW(7);
#endif
#if INJ_CHANNELS >= 8
    ASSIGN_SECONDARY_PW(8);
#endif
  }
#undef ASSIGN_SECONDARY_PW
}

static inline void zeroAllChannels(void)
{
  #define ASSIGN_ZERO_PW(index) fuelSchedule ## index .pw = 0U;

   ASSIGN_ZERO_PW(1);
#if INJ_CHANNELS >= 2
   ASSIGN_ZERO_PW(2);
#endif
#if INJ_CHANNELS >= 3
   ASSIGN_ZERO_PW(3);
#endif
#if INJ_CHANNELS >= 4
   ASSIGN_ZERO_PW(4);
#endif
#if INJ_CHANNELS >= 5
   ASSIGN_ZERO_PW(5);
#endif
#if INJ_CHANNELS >= 6
   ASSIGN_ZERO_PW(6);
#endif
#if INJ_CHANNELS >= 7
   ASSIGN_ZERO_PW(7);
#endif
#if INJ_CHANNELS >= 8
   ASSIGN_ZERO_PW(8);
#endif
#undef ASSIGN_ZERO_PW
}

static void __attribute__((optimize("Os"))) resetFuelSchedules(void)
{
  fuelSchedule1.reset();
#if (INJ_CHANNELS >= 2)
  fuelSchedule2.reset();
#endif
#if (INJ_CHANNELS >= 3)
  fuelSchedule3.reset();
#endif
#if (INJ_CHANNELS >= 4)
  fuelSchedule4.reset();
#endif
#if INJ_CHANNELS >= 5
  fuelSchedule5.reset();
#endif
#if INJ_CHANNELS >= 6
  fuelSchedule6.reset();
#endif
#if INJ_CHANNELS >= 7
  fuelSchedule7.reset();
#endif
#if INJ_CHANNELS >= 8
  fuelSchedule8.reset();
#endif
}

void __attribute__((optimize("Os"))) startFuelSchedulers(void)
{
  FUEL1_TIMER_ENABLE();
#if INJ_CHANNELS >= 2
  FUEL2_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 3
  FUEL3_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 4
  FUEL4_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_ENABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_ENABLE();
#endif
}

void __attribute__((optimize("Os"))) stopFuelSchedulers(void)
{
  FUEL1_TIMER_DISABLE();
#if INJ_CHANNELS >= 2
  FUEL2_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 3
  FUEL3_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 4
  FUEL4_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 5
  FUEL5_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 6
  FUEL6_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 7
  FUEL7_TIMER_DISABLE();
#endif
#if INJ_CHANNELS >= 8
  FUEL8_TIMER_DISABLE();
#endif  
}

TESTABLE_CONSTEXPR table2D_u8_u8_4 PrimingPulseTable(&configPage2.primeBins, &configPage2.primePulse);

/** Perform the injector priming pulses.
 * Set these to run at an arbitrary time in the future (100us).
 * The prime pulse value is in ms*10, so need to multiple by 100 to get to uS
 */
void __attribute__((optimize("Os"))) beginInjectorPriming(const statuses &current, const config4 &page4)
{
  uint16_t primingValue = (uint16_t)table2D_getValue(&PrimingPulseTable, temperatureAddOffset(current.coolant));
  if( (primingValue > 0U) && (current.TPS <= page4.floodClear) )
  {
    constexpr uint32_t PRIMING_DELAY = 100U; // 100us
    // The prime pulse value is in ms*2, so need to multiply by 500 to get to µS
    constexpr uint16_t PULSE_TS_SCALE_FACTOR = 100U * 5U; 

    primingValue = primingValue * PULSE_TS_SCALE_FACTOR; 
    if ( current.injOutputs.getTotalInjectors() >= 1U ) { setSchedule(fuelSchedule1, PRIMING_DELAY, primingValue, false); }
#if (INJ_CHANNELS >= 2)
    if ( current.injOutputs.getTotalInjectors() >= 2U ) { setSchedule(fuelSchedule2, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( current.injOutputs.getTotalInjectors() >= 3U ) { setSchedule(fuelSchedule3, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( current.injOutputs.getTotalInjectors() >= 4U ) { setSchedule(fuelSchedule4, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( current.injOutputs.getTotalInjectors() >= 5U ) { setSchedule(fuelSchedule5, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( current.injOutputs.getTotalInjectors() >= 6U ) { setSchedule(fuelSchedule6, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( current.injOutputs.getTotalInjectors() >= 7U) { setSchedule(fuelSchedule7, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( current.injOutputs.getTotalInjectors() >= 8U ) { setSchedule(fuelSchedule8, PRIMING_DELAY, primingValue, false); }
#endif
  }
}

void __attribute__((optimize("Os"))) closeAllInjectors(void)
{
  for (uint8_t index=1; index<=INJ_CHANNELS; ++index)
  {
    closeInjector(index);
  }
}

TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) uint16_t calcAngularCylinderSeparation(const statuses &current, const config2 &page2)
{
  // LCOV_EXCL_BR_START
  INTERNAL_TEST_ASSERT(current.injOutputs.primary!=0);
  INTERNAL_TEST_ASSERT(page2.nCylinders!=0);
  // LCOV_EXCL_BR_STOP
  
  // Default
  uint16_t separationAngle = CRANK_ANGLE_MAX_INJ/(current.injOutputs.primary);
  
  // Special cases
  if ((current.injLayout == INJ_SEMISEQUENTIAL) || (current.injLayout == INJ_PAIRED) || (page2.strokes == TWO_STROKE))
  {
    if (page2.nCylinders==5U)
    {
      separationAngle = 360/page2.nCylinders; // Force 5 cylinder to even spacing over 360 deg
    }
    if (page2.nCylinders==6U)
    {
      separationAngle = 720/page2.nCylinders; // Force 6 cylinder to even spacing over 720 deg
    }
  }
  return injectorLimits(separationAngle);
}

TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) uint16_t getOddfireAngle(const config2 &page2, uint8_t channel)
{
  switch (channel)
  {
    case 2: return page2.oddfire2; break;
    case 3: return page2.oddfire3; break;
    case 4: return page2.oddfire4; break;
    default: break;
  }
  return 0U;
}

TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) bool useEvenFire(const config2 &page2)
{
    return (page2.engineType == EVEN_FIRE) 
        || (page2.nCylinders!=2U) // Oddfire only supported on 2 cylinders (not sure why, should go up to 4 cylinders?)
        ;
}

TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) uint16_t getEvenFireAngle(const statuses &current, const config2 &page2, uint8_t channel)
{
  // Special case...
  if ((page2.nCylinders==2U) && (channel==2U))
  {
    return 180;
  }
  else
  {
    // LCOV_EXCL_BR_START
    INTERNAL_TEST_ASSERT(channel>0);
    // LCOV_EXCL_BR_STOP
    return (channel-1)*calcAngularCylinderSeparation(current, page2);
  }
}

/**
 * @brief Calculate the schedule channel angle. I.e. @ref FuelSchedule::channelDegrees
 * 
 * Most cases are spaced evenly round the crank cycle, with cylinder 1 at 0deg. 
 * E.g. 2 cylinder non-sequential is over 360deg so cylinder 2 is at 180deg.
 * 
 * There are special cases to account for though.
 */
TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) uint16_t calcScheduleAngle(const statuses &current, const config2 &page2, uint8_t channel)
{
  uint16_t angle = 0;

  if (page2.injTiming)
  {
    if (useEvenFire(page2))
    { 
      // This works for both primary & secondary injectors since injectorLimits() will clamp to <CRANK_ANGLE_MAX_INJ.
      // E.g. Assume 4 cylinder sequential + staging. 
      // Thus cylinder4 is a primary and cylinder8 is the secondary for cylinder4
      //    cylinder4 = (720/4)*(4-1) = 540
      //    cylinder8 = (720/4)*(8-1) = 1260 = 1260-720 = 540
      // (likewise for [1,5], [2,6], [3,7])
      angle = getEvenFireAngle(current, page2, channel);
    }
    else
    {
      // LCOV_EXCL_BR_START
      INTERNAL_TEST_ASSERT(page2.engineType == ODD_FIRE);
      // LCOV_EXCL_BR_STOP
      angle = getOddfireAngle(page2, channel);
    }
  }

  return injectorLimits(angle);
}

static inline __attribute__((optimize("Os"))) void initInjectorAngles(const statuses &current, const config2 &page2)
{
  #define ASSIGN_PRIMARY_ANGLE(index) \
    fuelSchedule ## index .channelDegrees = calcScheduleAngle(current, page2, index);

  ASSIGN_PRIMARY_ANGLE(1);
#if INJ_CHANNELS>=2
  ASSIGN_PRIMARY_ANGLE(2);
#endif
#if INJ_CHANNELS>=3
  ASSIGN_PRIMARY_ANGLE(3);
#endif
#if INJ_CHANNELS>=4
  ASSIGN_PRIMARY_ANGLE(4);
#endif
#if INJ_CHANNELS>=5
  ASSIGN_PRIMARY_ANGLE(5);
#endif
#if INJ_CHANNELS>=6
  ASSIGN_PRIMARY_ANGLE(6);
#endif
#if INJ_CHANNELS>=7
  ASSIGN_PRIMARY_ANGLE(7);
#endif
#if INJ_CHANNELS>=8
  ASSIGN_PRIMARY_ANGLE(8);
#endif
}

TESTABLE_STATIC __attribute__((optimize("Os"))) uint8_t calulateNumSquirts(const statuses &current, const config2 &page2)
{
  uint8_t nSquirts = 2U;
  if (page2.divider != 0)
  { 
    nSquirts = page2.nCylinders / page2.divider; //The number of squirts being requested. This is manually overridden below for sequential setups (Due to TS req_fuel calc limitations)
  }
  if ( (current.injLayout == INJ_SEQUENTIAL) && (page2.strokes == FOUR_STROKE) )
  {
    nSquirts = 1U;
  }
  // Force nSquirts to 2 for individual port injection.
  // This prevents TunerStudio forcing the value to 3 even when this isn't wanted. 
  if ((page2.nCylinders==3U) && (page2.injType == INJ_TYPE_PORT)
  && ((current.injLayout == INJ_SEMISEQUENTIAL) || (current.injLayout == INJ_PAIRED)))
  {
    nSquirts = 2;
  }

  //Safety check. Should never happen as TS will give an error, but leave in case tune is manually altered etc. 
  return max((uint8_t)1, nSquirts);
}

TESTABLE_STATIC __attribute__((optimize("Os"))) uint16_t calculateMaxInjAngle(const statuses &current, const config2 &page2)
{
  // Default
  uint16_t maxAngle = (page2.strokes == FOUR_STROKE ? 720 : 360) / current.nSquirts;

  // Special cases
  if (page2.nCylinders==3U)
  {
    if (current.injLayout == INJ_SEQUENTIAL)
    {
      maxAngle = (page2.strokes == FOUR_STROKE) ? 720 : 360;
    }
    else if ((page2.injType == INJ_TYPE_PORT)
        && ( (current.injLayout == INJ_SEMISEQUENTIAL) || (current.injLayout == INJ_PAIRED) ))
    { 
      maxAngle = (page2.strokes == FOUR_STROKE) ? 360 : 180;
    }
    else
    {
      // Use default
    }
  }
  // 3 or 5 squirts per cycle MUST be tracked over 720 degrees. This is because the angles for them (Eg 720/3=240) are 
  // not evenly divisible into 360. This is ONLY the case on 4 stroke systems
  if ((page2.strokes == FOUR_STROKE) && ( (current.nSquirts == 3) || (current.nSquirts == 5) ))
  {
    maxAngle = 720U / current.nSquirts;
  }

  return maxAngle;
}

TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) uint8_t calcNumPrimaryInjectors(const statuses &current, const config2 &page2)
{
  uint8_t primary = (page2.nCylinders==1U) 
                 || (page2.nCylinders==2U)
                 || (page2.nCylinders==3U)
                 || (page2.nCylinders==5U)
                 || (current.injLayout == INJ_SEQUENTIAL)           
                 ? page2.nCylinders : page2.nCylinders/2U;

  return clamp(primary, (uint8_t)1, (uint8_t)INJ_CHANNELS);
}

TESTABLE_STATIC __attribute__((optimize("Os"))) uint8_t calcNumSecondaryInjectors(uint16_t primary, const config2 &page2, const config10 &page10)
{
  uint8_t spareInjectors = INJ_CHANNELS - primary;

  uint16_t secondary = 0;
  if ((page10.stagingEnabled) && (spareInjectors>0))
  {
    // We have at least as many spare injectors as there are primariies
    // so we can have 1 secondary per primary.
    if (spareInjectors>=primary)
    {
      secondary = primary;
    }
    else
    {
      // Not enough to mirror (1:1) primaries, so just use 1 as staging.
      if (page2.nCylinders!=6)
      {
        secondary = 1;
      }
    }
  }

  return secondary;
}

TESTABLE_STATIC __attribute__((optimize("Os"))) num_injector_t calcNumInjectors(const statuses &current, const config2 &page2, const config10 &page10)
{
  uint8_t primary = calcNumPrimaryInjectors(current, page2);
  return num_injector_t { .primary = primary, .secondary = calcNumSecondaryInjectors(primary, page2, page10) };
}

static __attribute__((optimize("Os"))) uint8_t validateInjLayout(uint8_t layout, const config2 &page2)
{
  // Sequential only applies if enough channels.
  if (layout == INJ_SEQUENTIAL) {
    // If those conditions aren't met, revert to paired injection.
    if (page2.nCylinders>INJ_CHANNELS) {
      layout = INJ_PAIRED;
    }
  }

  if (layout == INJ_SEMISEQUENTIAL) {
    // Semi-sequential only valid for 4,5,6,8 cylinders and enough injectors and channels
    // If those conditions aren't met, revert to paired injection.
    if (!(page2.nCylinders==4U || page2.nCylinders==5U || page2.nCylinders==6U || page2.nCylinders==8U)
        || (page2.nInjectors<page2.nCylinders)) {
      layout = INJ_PAIRED;
    }
  }

  return layout;
}

TESTABLE_STATIC __attribute__((optimize("Os"))) void validateInjectionSetup(config2 &page2, config6 &page6)
{
  // Clamp the number of injectors to the number of channels available.
  page2.nInjectors = clamp(page2.nInjectors, (uint8_t)1U, (uint8_t)INJ_CHANNELS);
  
  //
  page2.injLayout = validateInjLayout(page2.injLayout, page2);

  // Oddfire only supported on 2 cylinders (not sure why, should go up to 4 cylinders?)
  if ((page2.engineType == ODD_FIRE) && (page2.nCylinders!=2U))
  {
    page2.engineType = EVEN_FIRE;
  }

  // Fuel trims are only applied in sequential mode.
  page6.fuelTrimEnabled = page6.fuelTrimEnabled && page2.injLayout == INJ_SEQUENTIAL;

  if (page2.injLayout == INJ_SEQUENTIAL)
  {
    // Force injection timing when sequential
    page2.injTiming = true;
  }
}

void __attribute__((optimize("Os"))) configureFuelSchedules(statuses &current, const config2 &page2, const config4 &page4, const config10 &page10)
{
  current.nSquirts = calulateNumSquirts(current, page2);
  CRANK_ANGLE_MAX_INJ = calculateMaxInjAngle(current, page2);
  current.injOutputs = calcNumInjectors(current, page2, page10);
  initInjectorAngles(current, page2);
  setupCallbacks(current.injLayout, page2.nCylinders, page4.inj4cylPairing);
}

void __attribute__((optimize("Os"))) initialiseFuelSchedules(statuses &current, config2 &page2, const config4 &page4, config6 &page6, config10 &page10, const pinNumbers_t &pins)
{
  initialiseInjectionIO(page4, pins);
  closeAllInjectors();
  resetFuelSchedules();
 
  validateInjectionSetup(page2, page6);
  current.injLayout = page2.injLayout;

  configureFuelSchedules(current, page2, page4, page10);

  // Turn off staging if no secondary injectors
  page10.stagingEnabled = page10.stagingEnabled && current.injOutputs.secondary>0;
}

static inline bool isSwitchableConfig(const config2 &page2)
{
  return (page2.injLayout == INJ_SEQUENTIAL) 
      && ((page2.nCylinders==4U)
      || (page2.nCylinders==6U)
      || (page2.nCylinders==8U))
      ;
}

TESTABLE_INLINE_STATIC bool changeToSemiSequentialInjection(const statuses &current, const config2 &page2)
{
  return isSwitchableConfig(page2)
      && (current.injLayout == INJ_SEQUENTIAL)
      && (current.decoder.getStatus().syncStatus==SyncStatus::Partial);
}

TESTABLE_INLINE_STATIC bool changeToFullSequentialInjection(const statuses &current, const config2 &page2)
{
  return isSwitchableConfig(page2)
      && (current.injLayout == INJ_SEMISEQUENTIAL)
      && (current.decoder.getStatus().syncStatus==SyncStatus::Full);
}

static inline void changeFuellingToFullSequential(const config2 &page2, const config4 &page4, const config10 &page10, statuses &current)
{
  ATOMIC() {
    if( !isAnyFuelScheduleRunning() )
    {
      current.injLayout = INJ_SEQUENTIAL;
      configureFuelSchedules(current, page2, page4, page10);
    }
  }
}

static inline void changeFuellingToSemiSequential(const config2 &page2, const config4 &page4, const config10 &page10, statuses &current)
{
  ATOMIC()
  {
    if( !isAnyFuelScheduleRunning() )
    {
      current.injLayout = INJ_SEMISEQUENTIAL;
      configureFuelSchedules(current, page2, page4, page10);
    }
  }
}

// If:
// 1. The users has chosen sequential injection; and
// 2. We have an even number of cylinders; and
// 3. Thue engine only has half sync; 
// Then
//  change to semi-sequential fuelling *and* change back once sync is restored
TESTABLE_STATIC void matchFuelSchedulersToSyncState(const config2 &page2, const config4 &page4, const config10 &page10, statuses &current) {
  if (isSwitchableConfig(page2))
  {
    if (changeToFullSequentialInjection(current, page2)) {
      changeFuellingToFullSequential(page2, page4, page10, current);
    } else if(changeToSemiSequentialInjection(current, page2)) { 
      changeFuellingToSemiSequential(page2, page4, page10, current);
    } else {
      // Injection layout matches current sync - do nothing
    }
  }
}

BEGIN_LTO_ALWAYS_INLINE(void) applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, const config4 &page4, const config6 &page6, const config10 &page10, statuses &current) {
  matchFuelSchedulersToSyncState(page2, page4, page10, current);
  zeroAllChannels();
  assignPrimaryPws(pulse_widths, page6, current);
  assignSecondaryPws(pulse_widths, current);
}
END_LTO_INLINE()
