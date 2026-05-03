#include "scheduler_fuel_controller.h"
#include "scheduledIO_inj.h"
#include "units.h"

static inline bool isSwitchableCylinderCount(const config2 &page2)
{
  return (page2.nCylinders==4U)
      || (page2.nCylinders==6U)
      || (page2.nCylinders==8U)
      ;
}

bool isSemiSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus)
{
  return (page2.injLayout == INJ_SEQUENTIAL) 
      && isSwitchableCylinderCount(page2)
      && decoderStatus.syncStatus==SyncStatus::Partial;
}

static inline bool isFullSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus)
{
  return (page2.injLayout == INJ_SEQUENTIAL) 
      && decoderStatus.syncStatus==SyncStatus::Full;
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
      
      setCallbacks(fuelSchedule1, openInjector1, closeInjector1);
      setCallbacks(fuelSchedule2, openInjector2, closeInjector2);
      setCallbacks(fuelSchedule3, openInjector3, closeInjector3);
      setCallbacks(fuelSchedule4, openInjector4, closeInjector4);
  #if INJ_CHANNELS >= 5
      setCallbacks(fuelSchedule5, openInjector5, closeInjector5);
  #endif
  #if INJ_CHANNELS >= 6
      setCallbacks(fuelSchedule6, openInjector6, closeInjector6);
  #endif
  #if INJ_CHANNELS >= 7
      setCallbacks(fuelSchedule7, openInjector7, closeInjector7);
  #endif
  #if INJ_CHANNELS >= 8
      setCallbacks(fuelSchedule8, openInjector8, closeInjector8);
  #endif
    }
  }
}

static inline void changeFuellingtoHalfSync(const config2 &page2, const config4 &page4, statuses &current)
{
  ATOMIC()
  {
    if( !isAnyFuelScheduleRunning() )
    {
      CRANK_ANGLE_MAX_INJ = 360;
      current.numPrimaryInjOutputs = page2.nCylinders/2U;
      switch (page2.nCylinders)
      {
        case 4:
          if(page4.inj4cylPairing == INJ_PAIR_13_24)
          {
            setCallbacks(fuelSchedule1, openInjector1and3, closeInjector1and3);
            setCallbacks(fuelSchedule2, openInjector2and4, closeInjector2and4);
          }
          else
          {
            setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
            setCallbacks(fuelSchedule2, openInjector2and3, closeInjector2and3);
          }
          break;
              
        case 6:
          setCallbacks(fuelSchedule1, openInjector1and4, closeInjector1and4);
          setCallbacks(fuelSchedule2, openInjector2and5, closeInjector2and5);
          setCallbacks(fuelSchedule3, openInjector3and6, closeInjector3and6);
          break;

        case 8:
          setCallbacks(fuelSchedule1, openInjector1and5, closeInjector1and5);
          setCallbacks(fuelSchedule2, openInjector2and6, closeInjector2and6);
          setCallbacks(fuelSchedule3, openInjector3and7, closeInjector3and7);
          setCallbacks(fuelSchedule4, openInjector4and8, closeInjector4and8);
          break;

        // LCOV_EXCL_BR_START
        default:
          break; //No actions required for other cylinder counts 
        // LCOV_EXCL_BR_STOP
      }
    }
  }
}

void matchFuelSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current) {
  if (isSwitchableCylinderCount(page2))
  {
    if (isFullSequentialInjection(page2, current.decoder.getStatus()) && ( CRANK_ANGLE_MAX_INJ != 720 )) {
      changeFuellingToFullSequential(page2, current);
    } else if(isSemiSequentialInjection(page2, current.decoder.getStatus()) && (CRANK_ANGLE_MAX_INJ != 360) ) { 
      changeFuellingtoHalfSync(page2, page4, current);
    } else {
      // Injection layout matches current sync - nothing to do but keep MISRA checker happy
    }
  }
}

static table2D_u8_u16_4 injectorAngleTable(&configPage2.injAngRPM, &configPage2.injAng);

static inline uint16_t lookupInjectorAngle(const statuses &current)
{
  uint16_t injAngle = table2D_getValue(&injectorAngleTable, current.RPMdiv100);
  // Do not combine min() & table2D_getValue() - if min() is a macro, we could call table2D_getValue twice
  return min(uint16_t(CRANK_ANGLE_MAX_INJ), injAngle);
}

static inline void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask, uint16_t injAngle, injectorAngleCalcCache *pCache)
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

BEGIN_LTO_ALWAYS_INLINE(uint16_t) __attribute__((flatten)) setFuelChannelSchedules(const statuses &current)
{
  uint16_t crankAngle = injectorLimits(current.decoder.getCrankAngle());
  byte injChannelMask = currentStatus.schedulerCutState.fuelChannels;
  uint16_t injAngle = lookupInjectorAngle(current);

  injectorAngleCalcCache angleCalcCache;
#define SET_FUEL_CHANNEL(channel) \
  setFuelChannelSchedule(fuelSchedule ##channel, UINT8_C(channel), crankAngle, injChannelMask, injAngle, &angleCalcCache);

#if INJ_CHANNELS >= 1
  SET_FUEL_CHANNEL(1)
#endif

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

BEGIN_LTO_ALWAYS_INLINE(void) applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config6 &page6, const statuses &current) {
  zeroAllChannels();
  assignPrimaryPws(pulse_widths, page6, current);
  assignSecondaryPws(pulse_widths, current);
}
END_LTO_INLINE()
