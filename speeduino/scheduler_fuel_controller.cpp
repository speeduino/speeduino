#include "scheduler_fuel_controller.h"
#include "scheduledIO_inj.h"
#include "units.h"

FuelSchedule fuelSchedule1(FUEL1_COUNTER, FUEL1_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule2(FUEL2_COUNTER, FUEL2_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule3(FUEL3_COUNTER, FUEL3_COMPARE); //cppcheck-suppress misra-c2012-8.4
FuelSchedule fuelSchedule4(FUEL4_COUNTER, FUEL4_COMPARE); //cppcheck-suppress misra-c2012-8.4
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
      setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
    }
    else
    {
      setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
      setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
    }
  }
  else if( nCylinders == 5 ) //This is similar to the paired injection but uses five injector outputs instead of four
  {
    setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
    setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
    setCallbacks(fuelSchedule3, openInjector3and5, closeInjector3and5);
    setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  }
  else if( nCylinders == 6 )
  {
    setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
    setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
    setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
  }
  else if( nCylinders == 8 )
  {
    setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
    setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
    setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
    setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
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

static inline bool isSwitchableCylinderCount(const config2 &page2)
{
  return (page2.nCylinders==4U)
      || (page2.nCylinders==6U)
      || (page2.nCylinders==8U)
      ;
}

TESTABLE_INLINE_STATIC bool changeToSemiSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus)
{
  return (page2.injLayout == INJ_SEQUENTIAL) 
      && isSwitchableCylinderCount(page2)
      && (decoderStatus.syncStatus==SyncStatus::Partial)
      && (CRANK_ANGLE_MAX_INJ != 360U);
}

TESTABLE_INLINE_STATIC bool changeToFullSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus)
{
  return (page2.injLayout == INJ_SEQUENTIAL) 
      && (decoderStatus.syncStatus==SyncStatus::Full)
      && (CRANK_ANGLE_MAX_INJ!=720U);
}

TESTABLE_INLINE_STATIC bool isAnyFuelScheduleRunning(void) {
  return isRunning(fuelSchedule1)
      || isRunning(fuelSchedule2)
      || isRunning(fuelSchedule3)
      || isRunning(fuelSchedule4)
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

static inline void changeFuellingToFullSequential(const config2 &page2, statuses &current)
{
  ATOMIC() {
    if( !isAnyFuelScheduleRunning() )
    {
      CRANK_ANGLE_MAX_INJ = 720;
      current.numPrimaryInjOutputs = page2.nCylinders;
      current.injLayout = INJ_SEQUENTIAL;
      setupCallbacks(INJ_SEQUENTIAL, page2.nCylinders, 0U);
    }
  }
}

static inline void changeFuellingToSemiSequential(const config2 &page2, const config4 &page4, statuses &current)
{
  ATOMIC()
  {
    if( !isAnyFuelScheduleRunning() )
    {
      CRANK_ANGLE_MAX_INJ = 360;
      current.numPrimaryInjOutputs = page2.nCylinders/2U;
      current.injLayout = INJ_SEMISEQUENTIAL;
      setupCallbacks(INJ_SEMISEQUENTIAL, page2.nCylinders, page4.inj4cylPairing);
    }
  }
}

// If:
// 1. The users has chosen sequential injection; and
// 2. We have an even number of cylinders; and
// 3. Thue engine only has half sync; 
// Then
//  change to semi-sequential fuelling *and* change back once sync is restored
TESTABLE_STATIC void matchFuelSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current) {
  if (isSwitchableCylinderCount(page2))
  {
    if (changeToFullSequentialInjection(page2, current.decoder.getStatus())) {
      changeFuellingToFullSequential(page2, current);
    } else if(changeToSemiSequentialInjection(page2, current.decoder.getStatus())) { 
      changeFuellingToSemiSequential(page2, page4, current);
    } else {
      // Injection layout matches current sync
      current.injLayout = page2.injLayout;
    }
  }
}

TESTABLE_STATIC table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);

TESTABLE_INLINE_STATIC uint16_t lookupInjectorAngle(const statuses &current)
{
  uint16_t injAngle = table2D_getValue(&injectorAngleTable, current.RPMdiv100);
  // Do not combine min() & table2D_getValue() - if min() is a macro, we could call table2D_getValue twice
  return min(uint16_t(CRANK_ANGLE_MAX_INJ), injAngle);
}

TESTABLE_INLINE_STATIC void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask, uint16_t injAngle, injectorAngleCalcCache *pCache) noexcept
{
  if( (schedule.pw != 0U) && (BIT_CHECK(injChannelMask, INJ1_CMD_BIT+channel-1U)) )
  {
    uint32_t timeOut = calculateInjectorTimeout(schedule, crankAngle, 
                                                _calculateOpenAngle(schedule, updatePwAngleCache(schedule.pw, pCache), injAngle));
    if (timeOut>0U)
    {
      // Only queue up the next schedule if the maximum time between squirts (Based on CRANK_ANGLE_MAX_INJ) is less than the max timer period
      setSchedule(schedule, timeOut, schedule.pw, angleToTimeMicroSecPerDegree((uint16_t)CRANK_ANGLE_MAX_INJ) < MAX_TIMER_PERIOD);
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

  switch (current.numPrimaryInjOutputs)
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
    if ((index>current.numPrimaryInjOutputs) && (index<=getTotalInjChannelCount(current))) \
    { \
      fuelSchedule ## index .pw = pulse_widths.secondary; \
    }

  if (current.numSecondaryInjOutputs>0U)
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

BEGIN_LTO_ALWAYS_INLINE(void) applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, const config4 &page4, const config6 &page6, statuses &current) {
  matchFuelSchedulersToSyncState(page2, page4, current);
  zeroAllChannels();
  assignPrimaryPws(pulse_widths, page6, current);
  assignSecondaryPws(pulse_widths, current);
}
END_LTO_INLINE()

static void __attribute__((optimize("Os"))) resetFuelSchedules(void)
{
  fuelSchedule1.reset();
  fuelSchedule2.reset();
  fuelSchedule3.reset();
  fuelSchedule4.reset();
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
  FUEL2_TIMER_ENABLE();
  FUEL3_TIMER_ENABLE();
  FUEL4_TIMER_ENABLE();
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
  FUEL2_TIMER_DISABLE();
  FUEL3_TIMER_DISABLE();
  FUEL4_TIMER_DISABLE();
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
    if ( getTotalInjChannelCount(current) >= 1U ) { setSchedule(fuelSchedule1, PRIMING_DELAY, primingValue, false); }
#if (INJ_CHANNELS >= 2)
    if ( getTotalInjChannelCount(current) >= 2U ) { setSchedule(fuelSchedule2, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 3)
    if ( getTotalInjChannelCount(current) >= 3U ) { setSchedule(fuelSchedule3, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 4)
    if ( getTotalInjChannelCount(current) >= 4U ) { setSchedule(fuelSchedule4, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 5)
    if ( getTotalInjChannelCount(current) >= 5U ) { setSchedule(fuelSchedule5, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 6)
    if ( getTotalInjChannelCount(current) >= 6U ) { setSchedule(fuelSchedule6, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 7)
    if ( getTotalInjChannelCount(current) >= 7U) { setSchedule(fuelSchedule7, PRIMING_DELAY, primingValue, false); }
#endif
#if (INJ_CHANNELS >= 8)
    if ( getTotalInjChannelCount(current) >= 8U ) { setSchedule(fuelSchedule8, PRIMING_DELAY, primingValue, false); }
#endif
  }
}

/** @brief Limit all injection schedule channel angles to 0-CRANK_ANGLE_MAX_INJ */
static __attribute__((optimize("Os"))) void clampInjectionChannelAngles(void)
{
  fuelSchedule1.channelDegrees = injectorLimits(fuelSchedule1.channelDegrees);
#if INJ_CHANNELS>=2
  fuelSchedule2.channelDegrees = injectorLimits(fuelSchedule2.channelDegrees);
#endif
#if INJ_CHANNELS>=3
  fuelSchedule3.channelDegrees = injectorLimits(fuelSchedule3.channelDegrees);
#endif
#if INJ_CHANNELS>=4
  fuelSchedule4.channelDegrees = injectorLimits(fuelSchedule4.channelDegrees);
#endif
#if INJ_CHANNELS>=5
  fuelSchedule5.channelDegrees = injectorLimits(fuelSchedule5.channelDegrees);
#endif
#if INJ_CHANNELS>=6
  fuelSchedule6.channelDegrees = injectorLimits(fuelSchedule6.channelDegrees);
#endif
#if INJ_CHANNELS>=7
  fuelSchedule7.channelDegrees = injectorLimits(fuelSchedule7.channelDegrees);
#endif
#if INJ_CHANNELS>=8
  fuelSchedule8.channelDegrees = injectorLimits(fuelSchedule8.channelDegrees);
#endif
}

static __attribute__((optimize("Os"))) void initFuelScheduleAngles(statuses &current, const config2 &page2, const config10 &page10)
{
  CRANK_ANGLE_MAX_INJ = (page2.strokes == FOUR_STROKE ? 720 : 360) / current.nSquirts;

  current.numPrimaryInjOutputs = 1; // Disable all injectors expect channel 1
  current.numSecondaryInjOutputs = 0;

  switch (page2.nCylinders) {
  case 1:
      fuelSchedule1.channelDegrees = 0;
      current.numPrimaryInjOutputs = 1;

      if ( (page2.injLayout == INJ_SEQUENTIAL) && (page2.strokes == FOUR_STROKE) )
      {
        CRANK_ANGLE_MAX_INJ = 720;
        current.nSquirts = 1;
      }

      //Check if injector staging is enabled
      if(page10.stagingEnabled == true)
      {
        current.numSecondaryInjOutputs = 1;
        fuelSchedule2.channelDegrees = fuelSchedule1.channelDegrees;
      }
      break;

  case 2:
      fuelSchedule1.channelDegrees = 0;
      current.numPrimaryInjOutputs = 2;

      if ( (page2.injLayout == INJ_SEQUENTIAL) && (page2.strokes == FOUR_STROKE) )
      {
        CRANK_ANGLE_MAX_INJ = 720;
        current.nSquirts = 1;
      }
      //The below are true regardless of whether this is running sequential or not
      if (page2.engineType == EVEN_FIRE ) { fuelSchedule2.channelDegrees = 180; }
      else { fuelSchedule2.channelDegrees = page2.oddfire2; }

      if (!page2.injTiming) 
      { 
        //For simultaneous, all squirts happen at the same time
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 0; 
      }

      //Check if injector staging is enabled
      if(page10.stagingEnabled == true)
      {
        current.numSecondaryInjOutputs = 2;

        fuelSchedule3.channelDegrees = fuelSchedule1.channelDegrees;
        //Phase this either 180 or 360 degrees out from inj3 (In reality this will always be 180 as you can't have sequential and staged currently)
        fuelSchedule4.channelDegrees = fuelSchedule3.channelDegrees + (uint16_t)(CRANK_ANGLE_MAX_INJ / 2U); 
      }
      break;

  case 3:
      current.numPrimaryInjOutputs = 3;

      //For alternating injection, the squirt occurs at different times for each channel
      if( (page2.injLayout == INJ_SEMISEQUENTIAL) || (page2.injLayout == INJ_PAIRED) )
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;

        if(page2.injType == INJ_TYPE_PORT)
        { 
          //Force nSquirts to 2 for individual port injection. This prevents TunerStudio forcing the value to 3 even when this isn't wanted. 
          current.nSquirts = 2;
          if(page2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = 360; }
          else { CRANK_ANGLE_MAX_INJ = 180; }
        }
        
        //Adjust the injection angles based on the number of squirts
        if (current.nSquirts > 2)
        {
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / current.nSquirts;
          fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2) / current.nSquirts;
        }

        if (!page2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0; 
        } 
      }
      else if (page2.injLayout == INJ_SEQUENTIAL)
      {
        current.nSquirts = 1;

        if(page2.strokes == TWO_STROKE)
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 120;
          fuelSchedule3.channelDegrees = 240;
          CRANK_ANGLE_MAX_INJ = 360;
        }
        else
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 240;
          fuelSchedule3.channelDegrees = 480;
          CRANK_ANGLE_MAX_INJ = 720;
        }
      }
      else
      {
        //Should never happen, but default values
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;
      }

      //Check if injector staging is enabled
      if(page10.stagingEnabled == true)
      {
        #if INJ_CHANNELS >= 6
          current.numSecondaryInjOutputs = 3;

          fuelSchedule4.channelDegrees = fuelSchedule1.channelDegrees;
          fuelSchedule5.channelDegrees = fuelSchedule2.channelDegrees;
          fuelSchedule6.channelDegrees = fuelSchedule3.channelDegrees;
        #else
          //Staged output is on channel 4
          current.numSecondaryInjOutputs = 1;
          fuelSchedule4.channelDegrees = fuelSchedule1.channelDegrees;
        #endif
      }
      break;
  case 4:
      fuelSchedule1.channelDegrees = 0;
      current.numPrimaryInjOutputs = 2;

      //For alternating injection, the squirt occurs at different times for each channel
      if( (page2.injLayout == INJ_SEMISEQUENTIAL) || (page2.injLayout == INJ_PAIRED) || (page2.strokes == TWO_STROKE) )
      {
        fuelSchedule2.channelDegrees = 180;

        if (!page2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0; 
        }
        else if (current.nSquirts > 2)
        {
          //Adjust the injection angles based on the number of squirts
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / current.nSquirts;
        }
        else { } //Do nothing, default values are correct
      }
      else if (page2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule2.channelDegrees = 180;
        fuelSchedule3.channelDegrees = 360;
        fuelSchedule4.channelDegrees = 540;

        current.numPrimaryInjOutputs = 4;

        CRANK_ANGLE_MAX_INJ = 720;
        current.nSquirts = 1;
      }
      else
      {
        //Should never happen, but default values
        current.numPrimaryInjOutputs = 2;
      }

      //Check if injector staging is enabled
      if(page10.stagingEnabled == true)
      {
        current.numPrimaryInjOutputs = 4;

        if( (page2.injLayout == INJ_SEQUENTIAL) || (page2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 4 cylinders semi/sequential requires 8 total channels
          #if INJ_CHANNELS >= 8
            current.numSecondaryInjOutputs = 4;

            fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
            fuelSchedule6.channelDegrees = fuelSchedule2.channelDegrees;
            fuelSchedule7.channelDegrees = fuelSchedule3.channelDegrees;
            fuelSchedule8.channelDegrees = fuelSchedule4.channelDegrees;
          #else
            //This is an invalid config as there are not enough outputs to support sequential + staging
            //Put the staging output to the non-existent channel 5
            #if (INJ_CHANNELS >= 5)
            current.numSecondaryInjOutputs = 1;
            fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
            #endif
          #endif
        }
        else
        {
          fuelSchedule3.channelDegrees = fuelSchedule1.channelDegrees;
          fuelSchedule4.channelDegrees = fuelSchedule2.channelDegrees;
        }
      }

      break;
  case 5:
      current.numPrimaryInjOutputs = 4; //Is updated below to 5 if there are enough channels

      //For alternating injection, the squirt occurs at different times for each channel
      if( (page2.injLayout == INJ_SEMISEQUENTIAL) || (page2.injLayout == INJ_PAIRED) || (page2.strokes == TWO_STROKE) )
      {
        if (!page2.injTiming) 
        { 
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0;
          fuelSchedule4.channelDegrees = 0;
#if (INJ_CHANNELS >= 5)
          fuelSchedule5.channelDegrees = 0; 
#endif
        }
        else
        {
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 72;
          fuelSchedule3.channelDegrees = 144;
          fuelSchedule4.channelDegrees = 216;
#if (INJ_CHANNELS >= 5)
          fuelSchedule5.channelDegrees = 288;
#endif

          //Divide by current.nSquirts ?
        }
      }
  #if INJ_CHANNELS >= 5
      else if (page2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 144;
        fuelSchedule3.channelDegrees = 288;
        fuelSchedule4.channelDegrees = 432;
        fuelSchedule5.channelDegrees = 576;

        current.numPrimaryInjOutputs = 5;

        CRANK_ANGLE_MAX_INJ = 720;
        current.nSquirts = 1;
      }
  #endif

  #if INJ_CHANNELS >= 6
        if(page10.stagingEnabled == true) { current.numSecondaryInjOutputs = 1; }
  #endif
      break;
  case 6:
      current.numPrimaryInjOutputs = 3;

      //For alternating injection, the squirt occurs at different times for each channel
      if( (page2.injLayout == INJ_SEMISEQUENTIAL) || (page2.injLayout == INJ_PAIRED) )
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;
        if (!page2.injTiming)
        {
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0;
        }
        else if (current.nSquirts > 2)
        {
          //Adjust the injection angles based on the number of squirts
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / current.nSquirts;
          fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2) / current.nSquirts;
        }
      }

  #if INJ_CHANNELS >= 6
      if (page2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 120;
        fuelSchedule3.channelDegrees = 240;
        fuelSchedule4.channelDegrees = 360;
        fuelSchedule5.channelDegrees = 480;
        fuelSchedule6.channelDegrees = 600;

        current.numPrimaryInjOutputs = 6;

        CRANK_ANGLE_MAX_INJ = 720;
        current.nSquirts = 1;
      }
      else if(page10.stagingEnabled == true) //Check if injector staging is enabled
      {
        current.numSecondaryInjOutputs = 3;

        if( (page2.injLayout == INJ_SEQUENTIAL) || (page2.injLayout == INJ_SEMISEQUENTIAL) )
        {
          //Staging with 6 cylinders semi/sequential requires 7 total channels
          #if INJ_CHANNELS >= 7
            current.numSecondaryInjOutputs = 4;

            fuelSchedule5.channelDegrees = fuelSchedule1.channelDegrees;
            fuelSchedule6.channelDegrees = fuelSchedule2.channelDegrees;
            fuelSchedule7.channelDegrees = fuelSchedule3.channelDegrees;
            // TODO: this makes no sense!!
            fuelSchedule8.channelDegrees = fuelSchedule4.channelDegrees;
          #else
            //This is an invalid config as there are not enough outputs to support sequential + staging
            //No staging output will be active
          #endif
        }
      }
  #endif
      break;
  case 8:
      current.numPrimaryInjOutputs = 4;

      //For alternating injection, the squirt occurs at different times for each channel
      if( (page2.injLayout == INJ_SEMISEQUENTIAL) || (page2.injLayout == INJ_PAIRED) )
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 90;
        fuelSchedule3.channelDegrees = 180;
        fuelSchedule4.channelDegrees = 270;

        if (!page2.injTiming)
        {
          //For simultaneous, all squirts happen at the same time
          fuelSchedule1.channelDegrees = 0;
          fuelSchedule2.channelDegrees = 0;
          fuelSchedule3.channelDegrees = 0;
          fuelSchedule4.channelDegrees = 0;
        }
        else if (current.nSquirts > 2)
        {
          //Adjust the injection angles based on the number of squirts
          fuelSchedule2.channelDegrees = (fuelSchedule2.channelDegrees * 2) / current.nSquirts;
          fuelSchedule3.channelDegrees = (fuelSchedule3.channelDegrees * 2) / current.nSquirts;
          fuelSchedule4.channelDegrees = (fuelSchedule4.channelDegrees * 2) / current.nSquirts;
        }
      }

  #if INJ_CHANNELS >= 8
      else if (page2.injLayout == INJ_SEQUENTIAL)
      {
        fuelSchedule1.channelDegrees = 0;
        fuelSchedule2.channelDegrees = 90;
        fuelSchedule3.channelDegrees = 180;
        fuelSchedule4.channelDegrees = 270;
        fuelSchedule5.channelDegrees = 360;
        fuelSchedule6.channelDegrees = 450;
        fuelSchedule7.channelDegrees = 540;
        fuelSchedule8.channelDegrees = 630;

        current.numPrimaryInjOutputs = 8;

        CRANK_ANGLE_MAX_INJ = 720;
        current.nSquirts = 1;
      }
  #endif

      break;
  default: //Handle this better!!!
      fuelSchedule1.channelDegrees = 0;
      fuelSchedule2.channelDegrees = 180;
      break;
  }

  //Special case:
  //3 or 5 squirts per cycle MUST be tracked over 720 degrees. This is because the angles for them (Eg 720/3=240) are not evenly divisible into 360
  //This is ONLY the case on 4 stroke systems
  if( (current.nSquirts == 3) || (current.nSquirts == 5) )
  {
    if(page2.strokes == FOUR_STROKE) { CRANK_ANGLE_MAX_INJ = (720U / current.nSquirts); }
  }
}

static __attribute__((optimize("Os"))) uint8_t calulateDefaultSquirts(const config2 &page2)
{
  uint8_t nSquirts = 2U;
  if (page2.divider != 0)
  { 
    nSquirts = page2.nCylinders / page2.divider; //The number of squirts being requested. This is manually overridden below for sequential setups (Due to TS req_fuel calc limitations)
  }

  //Safety check. Should never happen as TS will give an error, but leave in case tune is manually altered etc. 
  return max((uint8_t)1, nSquirts);
}

void __attribute__((optimize("Os"))) initialiseFuelSchedules(statuses &current, const config2 &page2, const config4 &page4, const config10 &page10)
{
  resetFuelSchedules();
  setupCallbacks(page2.injLayout, page2.nCylinders, page4.inj4cylPairing);
  current.injLayout = page2.injLayout;

  current.nSquirts = calulateDefaultSquirts(page2);
  initFuelScheduleAngles(current, page2, page10);
  clampInjectionChannelAngles();
}