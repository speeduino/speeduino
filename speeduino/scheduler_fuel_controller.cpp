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
      current.maxInjOutputs = page2.nCylinders;
      
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
      current.maxInjOutputs = page2.nCylinders/2U;
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

uint16_t setInjectionAngles(const statuses &current)
{
  uint16_t injAngle = lookupInjectorAngle(current);

  injectorAngleCalcCache angleCalcCache;
  setOpenAngle(fuelSchedule1, injAngle, &angleCalcCache);
#if INJ_CHANNELS>=2
  setOpenAngle(fuelSchedule2, injAngle, &angleCalcCache);
#endif
#if INJ_CHANNELS>=3
  setOpenAngle(fuelSchedule3, injAngle, &angleCalcCache);
#endif
#if INJ_CHANNELS>=4
  setOpenAngle(fuelSchedule4, injAngle, &angleCalcCache);
#endif
#if INJ_CHANNELS>=5
  setOpenAngle(fuelSchedule5, injAngle, &angleCalcCache);
#endif
#if INJ_CHANNELS>=6
  setOpenAngle(fuelSchedule6, injAngle, &angleCalcCache);
#endif
#if INJ_CHANNELS>=7
  setOpenAngle(fuelSchedule7, injAngle, &angleCalcCache);
#endif
#if INJ_CHANNELS>=8
  setOpenAngle(fuelSchedule8, injAngle, &angleCalcCache);
#endif

  return injAngle;
}


static inline void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask)
{
  if( (schedule.pw != 0U) && (BIT_CHECK(injChannelMask, INJ1_CMD_BIT+channel-1U)) )
  {
    uint32_t timeOut = calculateInjectorTimeout(schedule, crankAngle);
    if (timeOut>0U)
    {
      setFuelScheduleDuration(schedule, timeOut, schedule.pw);
    }
  }
}

void __attribute__((flatten)) setFuelChannelSchedules(uint16_t crankAngle, byte injChannelMask)
{
#define SET_FUEL_CHANNEL(channel) \
  setFuelChannelSchedule(fuelSchedule ##channel, UINT8_C(channel), crankAngle, injChannelMask);

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
}

static inline void zeroAllPulseWidths(void) {
  fuelSchedule1.pw = 0U;
#if INJ_CHANNELS >= 2
  fuelSchedule2.pw = 0U;
#endif
#if INJ_CHANNELS >= 3
  fuelSchedule3.pw = 0U;
#endif
#if INJ_CHANNELS >= 4
  fuelSchedule4.pw = 0U;
#endif
#if INJ_CHANNELS >= 5
  fuelSchedule5.pw = 0U;
#endif
#if INJ_CHANNELS >= 6
  fuelSchedule6.pw = 0U;
#endif
#if INJ_CHANNELS >= 7
  fuelSchedule7.pw = 0U;
#endif
#if INJ_CHANNELS >= 8
  fuelSchedule8.pw = 0U;
#endif
}

static inline uint16_t applyFuelTrim(const trimTable3d &trimTable, uint16_t pw, const config6 &page6, const statuses &current)
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

void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, const config6 &page6, const statuses &current) {
  zeroAllPulseWidths();

  #define PULSEWIDTH_OR_ZERO(index, pulseWidth) (((current.maxInjOutputs) >= (uint8_t)(index)) ? (pulseWidth) : 0U)

  #define ASSIGN_PRIMARY_PW(index, pulseWidth) \
    fuelSchedule ## index .pw = applyFuelTrim(trimTables[index-1U], \
                                              PULSEWIDTH_OR_ZERO(index, pulseWidth), \
                                              page6, \
                                              current \
                                              );
  #define ASSIGN_SECONDARY_PW(index, pulseWidth) \
    fuelSchedule ## index .pw = PULSEWIDTH_OR_ZERO(index, pulseWidth);

  // The PW calcs already applied the logic to enable staging or not. If there is a valid
  // secondary PW, staging is enabled 
  if (pulse_widths.secondary!=0U) {
    //Allocate the primary and secondary pulse widths based on the fuel configuration
    switch (page2.nCylinders) {
      case 1:
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_SECONDARY_PW(2, pulse_widths.secondary);
#endif
        break;

      case 2:
        //Primary pulsewidth on channels 1 and 2, secondary on channels 3 and 4
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_SECONDARY_PW(3, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 4
        ASSIGN_SECONDARY_PW(4, pulse_widths.secondary);
#endif
        break;

      case 3:
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PRIMARY_PW(3, pulse_widths.primary);
#endif
        //6 channels required for 'normal' 3 cylinder staging support
#if INJ_CHANNELS >= 4
        ASSIGN_SECONDARY_PW(4, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 5
        ASSIGN_SECONDARY_PW(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
        ASSIGN_SECONDARY_PW(6, pulse_widths.secondary);
#endif
        break;

      case 4:
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
        if( (page2.injLayout == INJ_SEQUENTIAL) || (page2.injLayout == INJ_SEMISEQUENTIAL) )
        {
#if INJ_CHANNELS >= 3
          ASSIGN_PRIMARY_PW(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
          ASSIGN_PRIMARY_PW(4, pulse_widths.primary);
#endif
        // Staging with 4 cylinders semi/sequential requires 8 total channels
#if INJ_CHANNELS >= 5
          ASSIGN_SECONDARY_PW(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
          ASSIGN_SECONDARY_PW(6, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 7
          ASSIGN_SECONDARY_PW(7, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 8
          ASSIGN_SECONDARY_PW(8, pulse_widths.secondary);
#endif
        } else {
#if INJ_CHANNELS >= 3
          ASSIGN_SECONDARY_PW(3, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 4
          ASSIGN_SECONDARY_PW(4, pulse_widths.secondary);
#endif
        }
        break;
        
      case 5:
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PRIMARY_PW(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
        ASSIGN_PRIMARY_PW(4, pulse_widths.primary);
#endif
        //No easily supportable 5 cylinder staging option unless there are at least 5 channels
          if (page2.injLayout != INJ_SEQUENTIAL) {
#if INJ_CHANNELS >= 5
        ASSIGN_SECONDARY_PW(5, pulse_widths.secondary);
#endif
          } else {
#if INJ_CHANNELS >= 5
        ASSIGN_SECONDARY_PW(5, pulse_widths.primary);
#endif
          }
#if INJ_CHANNELS >= 6
        ASSIGN_SECONDARY_PW(6, pulse_widths.secondary);
#endif
        break;

      case 6:
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PRIMARY_PW(3, pulse_widths.primary);
#endif
        // 6 cylinder staging only if not sequential
        if (page2.injLayout != INJ_SEQUENTIAL) {
#if INJ_CHANNELS >= 4
        ASSIGN_SECONDARY_PW(4, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 5
        ASSIGN_SECONDARY_PW(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
        ASSIGN_SECONDARY_PW(6, pulse_widths.secondary);
#endif
        } else {
#if INJ_CHANNELS >= 4
          ASSIGN_PRIMARY_PW(4, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 5
          ASSIGN_PRIMARY_PW(5, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 6
          ASSIGN_PRIMARY_PW(6, pulse_widths.primary);
#endif
          //If there are 8 channels, then the 6 cylinder sequential option is available by using channels 7 + 8 for staging
#if INJ_CHANNELS >= 7
          ASSIGN_SECONDARY_PW(7, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 8
          ASSIGN_SECONDARY_PW(8, pulse_widths.secondary);
#endif
        }
        break;

      case 8:
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
        ASSIGN_PRIMARY_PW(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
        ASSIGN_PRIMARY_PW(4, pulse_widths.primary);
#endif
        //8 cylinder staging only if not sequential
        if (page2.injLayout != INJ_SEQUENTIAL)
        {
#if INJ_CHANNELS >= 5
          ASSIGN_SECONDARY_PW(5, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 6
          ASSIGN_SECONDARY_PW(6, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 7
          ASSIGN_SECONDARY_PW(7, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 8
          ASSIGN_SECONDARY_PW(8, pulse_widths.secondary);
#endif
        } else {
#if INJ_CHANNELS >= 5
        ASSIGN_PRIMARY_PW(5, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 6
        ASSIGN_PRIMARY_PW(6, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 7
        ASSIGN_PRIMARY_PW(7, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 8
        ASSIGN_PRIMARY_PW(8, pulse_widths.primary);
#endif          
        }
        break;

      default:
        //Assume 4 cylinder non-seq for default
        ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
        ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
          ASSIGN_SECONDARY_PW(3, pulse_widths.secondary);
#endif
#if INJ_CHANNELS >= 4
          ASSIGN_SECONDARY_PW(4, pulse_widths.secondary);
#endif
       break;
    }
  }
  else if (pulse_widths.primary!=0U)
  { 
    ASSIGN_PRIMARY_PW(1, pulse_widths.primary);
#if INJ_CHANNELS >= 2
    ASSIGN_PRIMARY_PW(2, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 3
    ASSIGN_PRIMARY_PW(3, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 4
    ASSIGN_PRIMARY_PW(4, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 5
    ASSIGN_PRIMARY_PW(5, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 6
    ASSIGN_PRIMARY_PW(6, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 7
    ASSIGN_PRIMARY_PW(7, pulse_widths.primary);
#endif
#if INJ_CHANNELS >= 8
    ASSIGN_PRIMARY_PW(8, pulse_widths.primary);
#endif
  } else {
    //No pulse widths to apply
  } 
}
