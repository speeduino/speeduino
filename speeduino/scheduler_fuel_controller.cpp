#include "scheduler_fuel_controller.h"
#include "scheduledIO_inj.h"

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
