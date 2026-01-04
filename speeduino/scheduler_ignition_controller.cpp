#include "scheduler_ignition_controller.h"
#include "scheduledIO_ign.h"

static inline bool isAnyIgnScheduleRunning(void) {
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

static inline void changeIgnitionToHalfSync(const config2 &page2, statuses &current)
{
  ATOMIC()
  {
    if (!isAnyIgnScheduleRunning()) {
      CRANK_ANGLE_MAX_IGN = 360;
      switch (page2.nCylinders)
      {
        case 4:
          setCallbacks(ignitionSchedule1, beginCoil1and3Charge, endCoil1and3Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and4Charge, endCoil2and4Charge);
          current.maxIgnOutputs = 2U;
          break;
              
        case 6:
          setCallbacks(ignitionSchedule1, beginCoil1and4Charge, endCoil1and4Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and5Charge, endCoil2and5Charge);
          setCallbacks(ignitionSchedule3, beginCoil3and6Charge, endCoil3and6Charge);
          current.maxIgnOutputs = 3U;
          break;

        case 8:
          setCallbacks(ignitionSchedule1, beginCoil1and5Charge, endCoil1and5Charge);
          setCallbacks(ignitionSchedule2, beginCoil2and6Charge, endCoil2and6Charge);
          setCallbacks(ignitionSchedule3, beginCoil3and7Charge, endCoil3and7Charge);
          setCallbacks(ignitionSchedule4, beginCoil4and8Charge, endCoil4and8Charge);
          current.maxIgnOutputs = 4U;
          break;
          
        default:
          break; //No actions required for other cylinder counts 
      }
    }
  }
}

static inline void changeIgnitionToFullSequential(const config2 &page2, statuses &current)
{
  ATOMIC()
  {
    if (!isAnyIgnScheduleRunning()) {
      CRANK_ANGLE_MAX_IGN = 720;
      current.maxIgnOutputs = min((uint8_t)IGN_CHANNELS, page2.nCylinders);
      switch (current.maxIgnOutputs)
      {
      case 4:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        break;

      case 6:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        break;

      case 8:
        setCallbacks(ignitionSchedule1, beginCoil1Charge, endCoil1Charge);
        setCallbacks(ignitionSchedule2, beginCoil2Charge, endCoil2Charge);
        setCallbacks(ignitionSchedule3, beginCoil3Charge, endCoil3Charge);
        setCallbacks(ignitionSchedule4, beginCoil4Charge, endCoil4Charge);
        break;

      default:
        break; //No actions required for other cylinder counts 
      }
    }
  }
}

void matchIgnitionSchedulersToSyncState(const config2 &page2, const config4 &page4, statuses &current)
{
  if (isFullSequentialIgnition(page4, current.decoder.getStatus()) && ( CRANK_ANGLE_MAX_IGN != 720 )) {
    changeIgnitionToFullSequential(page2, current);
  } else if(isSemiSequentialIgnition(page2, page4, current.decoder.getStatus()) && (CRANK_ANGLE_MAX_IGN != 360) ) { 
    changeIgnitionToHalfSync(page2, current);
  } else {
    // Ignition layout matches current sync - nothing to do but keep MISRA checker happy
  }
}
