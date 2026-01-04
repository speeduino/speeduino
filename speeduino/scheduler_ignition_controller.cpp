#include "scheduler_ignition_controller.h"
#include "scheduledIO_ign.h"
#include "schedule_calcs.hpp"
#include "globals.h"

constexpr table2D_u8_u8_8 rotarySplitTable(&configPage10.rotarySplitBins, &configPage10.rotarySplitValues);

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
}
END_LTO_INLINE()