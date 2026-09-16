#include "scheduler_fuel_controller.h"
#include "scheduledIO_inj.h"
#include "units.h"
#include "table2d.h"
#include "globals.h"

FuelSchedule fuelSchedules[INJ_CHANNELS] = {
  FuelSchedule(FUEL1_COUNTER, FUEL1_COMPARE),
  #if (INJ_CHANNELS >= 2)
  FuelSchedule(FUEL2_COUNTER, FUEL2_COMPARE),
  #endif
  #if (INJ_CHANNELS >= 3)
  FuelSchedule(FUEL3_COUNTER, FUEL3_COMPARE),
  #endif
  #if (INJ_CHANNELS >= 4)
  FuelSchedule(FUEL4_COUNTER, FUEL4_COMPARE),
  #endif
  #if (INJ_CHANNELS >= 5)
  FuelSchedule(FUEL5_COUNTER, FUEL5_COMPARE),
  #endif
  #if (INJ_CHANNELS >= 6)
  FuelSchedule(FUEL6_COUNTER, FUEL6_COMPARE),
  #endif
  #if (INJ_CHANNELS >= 7)
  FuelSchedule(FUEL7_COUNTER, FUEL7_COMPARE),
  #endif
  #if (INJ_CHANNELS >= 8)
  FuelSchedule(FUEL8_COUNTER, FUEL8_COMPARE),
  #endif
};

static __attribute__((optimize("Os"))) void setupSequentialCallbacks(void)
{
  #define SET_CALLBACKS(index) fuelSchedules[index-1].setCallbacks(openInjector ## index, closeInjector ## index);
  
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
      fuelSchedules[0].setCallbacks(openInjector1and3, closeInjector1and3);
#if (INJ_CHANNELS >= 2)
      fuelSchedules[1].setCallbacks(openInjector2and4, closeInjector2and4);
#endif
    }
    else
    {
      fuelSchedules[0].setCallbacks(openInjector1and4, closeInjector1and4);
#if (INJ_CHANNELS >= 2)
      fuelSchedules[1].setCallbacks(openInjector2and3, closeInjector2and3);
#endif
    }
  }
  else if( nCylinders == 5 ) //This is similar to the paired injection but uses five injector outputs instead of four
  {
    fuelSchedules[0].setCallbacks(openInjector1, closeInjector1);
#if (INJ_CHANNELS >= 2)
    fuelSchedules[1].setCallbacks(openInjector2, closeInjector2);
#endif
#if (INJ_CHANNELS >= 3)
    fuelSchedules[2].setCallbacks(openInjector3and5, closeInjector3and5);
#endif
#if (INJ_CHANNELS >= 4)
    fuelSchedules[3].setCallbacks(openInjector4, closeInjector4);
#endif
  }
  else if( nCylinders == 6 )
  {
    fuelSchedules[0].setCallbacks(openInjector1and4, closeInjector1and4);
#if (INJ_CHANNELS >= 2)
    fuelSchedules[1].setCallbacks(openInjector2and5, closeInjector2and5);
#endif
#if (INJ_CHANNELS >= 3)
    fuelSchedules[2].setCallbacks(openInjector3and6, closeInjector3and6);
#endif
  }
  else if( nCylinders == 8 )
  {
    fuelSchedules[0].setCallbacks(openInjector1and5, closeInjector1and5);
#if (INJ_CHANNELS >= 2)
    fuelSchedules[1].setCallbacks(openInjector2and6, closeInjector2and6);
#endif
#if (INJ_CHANNELS >= 3)
    fuelSchedules[2].setCallbacks(openInjector3and7, closeInjector3and7);
#endif
#if (INJ_CHANNELS >= 4)
    fuelSchedules[3].setCallbacks(openInjector4and8, closeInjector4and8);
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
  return std::any_of(std::begin(fuelSchedules), std::end(fuelSchedules), isRunning);
}

TESTABLE_STATIC table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);

TESTABLE_INLINE_STATIC uint16_t lookupInjectorAngle(const statuses &current)
{
  uint16_t injAngle = table2D_getValue(&injectorAngleTable, current.RPMdiv100);
  // Do not combine (std::min)() & table2D_getValue() - if (std::min)() is a macro, we could call table2D_getValue twice
  return (std::min)(uint16_t(CRANK_ANGLE_MAX_INJ), injAngle);
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
 * @param schedule Fuel schedule to calculate for
 * @param pwDegrees How many crank degrees the calculated PW will take at the current speed
 * @param injAngle The requested injection angle
 * @return uint16_t 
 */
TESTABLE_INLINE_STATIC uint16_t _calculateOpenAngle(const FuelSchedule &schedule, uint16_t pwDegrees, uint16_t injAngle)
{
  // 0<=injAngle<=720°
  // 0<=injChannelDegrees<=720°
  // 0<pwDegrees<=??? (could be many crank rotations in the worst case!)
  // 45<=CRANK_ANGLE_MAX_INJ<=720
  // (CRANK_ANGLE_MAX_INJ can be as small as 360/nCylinders. E.g. 45° for 8 cylinder)

  uint16_t startAngle = injAngle + schedule.channelDegrees;
  return normalize((int16_t)0, (int16_t)CRANK_ANGLE_MAX_INJ, (int16_t)((int16_t)startAngle - (int16_t)pwDegrees));
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
  for (uint8_t index=0; index<_countof(fuelSchedules); ++index) {
    setFuelChannelSchedule(fuelSchedules[index], index+1, crankAngle, injChannelMask, injAngle, &angleCalcCache);
  }

  return injAngle;
}

/** @brief Clamp the angle to within [0,CRANK_ANGLE_MAX_INJ) */
TESTABLE_INLINE_STATIC uint16_t injectorLimits(uint16_t angle)
{
  return normalize((uint16_t)0, (uint16_t)CRANK_ANGLE_MAX_INJ, angle);
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
  for (uint8_t index=0; index<current.injOutputs.primary; ++index) {
    fuelSchedules[index].pw = applyFuelTrim(trimTables[index], pulse_widths.primary, page6, current);
  }
}

static inline void assignSecondaryPws(const pulseWidths &pulse_widths, const statuses &current)
{
  for (uint8_t index=current.injOutputs.primary; index<current.injOutputs.getTotalInjectors(); ++index) {
    fuelSchedules[index].pw = pulse_widths.secondary;
  }
}

static inline void zeroAllChannels(void)
{
  for (auto& schedule: fuelSchedules) {
    schedule.pw = 0U;
  }
}

static void __attribute__((optimize("Os"))) resetFuelSchedules(void)
{
  for (auto& schedule: fuelSchedules) {
    schedule.reset();
  }
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
    for (uint8_t index=0; index<current.injOutputs.getTotalInjectors(); ++index) {
      setSchedule(fuelSchedules[index], PRIMING_DELAY, primingValue, false);
    }
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
  if (channel>1U && channel<_countof(page2.oddfire)+2)
  {
    return page2.oddfire[channel-2U];
  }
  return 0U;
}

TESTABLE_INLINE_STATIC __attribute__((optimize("Os"))) bool useEvenFire(const config2 &page2)
{
    return (page2.engineType == EVEN_FIRE);
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

static inline __attribute__((optimize("Os"))) void setInjectorAngles(const statuses &current, const config2 &page2)
{
  for (uint8_t index=0; index<_countof(fuelSchedules); ++index) {
    fuelSchedules[index].channelDegrees = calcScheduleAngle(current, page2, index+1);
  }
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
  return (std::max)((uint8_t)1, nSquirts);
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
  if ((page2.nCylinders==5U) && (current.injLayout == INJ_SEMISEQUENTIAL))
  {
    primary = 4;
  }
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

  // Oddfire only supported on up to number of oddfire angles
  if ((page2.engineType == ODD_FIRE) && (page2.nCylinders>_countof(page2.oddfire)+1U))
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
  setInjectorAngles(current, page2);
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
