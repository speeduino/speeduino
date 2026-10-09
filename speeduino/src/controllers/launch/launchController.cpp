#include "launchController.h"
#include "units.h"
#include "src/pins/inputPin.h"
#include "unit_testing.h"
#include "bit_manip.h"

TESTABLE_STATIC inputPin_t launchPin;

void __attribute__((optimize("Os"))) initialiseLaunchControl(statuses &current, config6 &page6, const pinNumbers_t &pins)
{
  launchPin.setPin(pins.pinLaunch);
  page6.flatSEnable = page6.flatSEnable && launchPin.isValid();
  page6.launchEnabled = page6.launchEnabled && launchPin.isValid();
  current.launchStatus = launchStatus_t();
}

static void updateClutchState(statuses &current, const config6 &page6)
{
  // Only read the shared clutch input when a function using it is enabled.
  if (page6.flatSEnable || page6.launchEnabled)
  {
    current.launchStatus.previousClutchTrigger = current.launchStatus.clutchTrigger;

    current.launchStatus.clutchTrigger = (page6.launchHiLo == launchPin.isPinHigh());

    // Capture RPM on engagement, then retain it while the clutch is held or released.
    if (current.launchStatus.clutchTrigger && !current.launchStatus.previousClutchTrigger)
    {
      current.launchStatus.clutchEngagedRPM = current.RPM;
    }
  }
}

static bool isLaunchArmed(const statuses &current, const config6 &page6, const config10 &page10)
{
  return page6.launchEnabled
      && current.launchStatus.clutchTrigger
      && (current.launchStatus.clutchEngagedRPM < RPM_COARSE.toUser(page6.flatSArm))
      && (current.TPS >= page10.lnchCtrlTPS);
}

static bool isFlatShiftActive(const statuses &current, const config6 &page6)
{
  return page6.flatSEnable 
      && current.launchStatus.clutchTrigger
      && (current.launchStatus.clutchEngagedRPM >= RPM_COARSE.toUser(page6.flatSArm))
      ;
}

static uint16_t getHardCutRpmLimit(uint16_t baseRpm, const config2 &page2, const config15 &page15)
{
  if (page2.hardCutType == HARD_CUT_ROLLING)
  {
    baseRpm += SIGNED_RPM_MEDIUM.toUser(page15.rollingProtRPMDelta[0]);
  }
  return baseRpm;
}

static bool withinLaunchSpeedLimit(const statuses &current, const config2 &page2, const config10 &page10)
{
  return (page2.vssMode == VSS_MODE_OFF) || (current.vss < page10.lnchCtrlVss);
}

static bool aboveHardLaunchRpmLimit(const statuses &current, const config2 &page2, const config6 &page6, const config15 &page15)
{
  const uint16_t launchRpmLimit = getHardCutRpmLimit(RPM_COARSE.toUser(page6.lnchHardLim), page2, page15);
  return current.RPM > launchRpmLimit;
}

static bool aboveSoftLaunchRpmLimit(const statuses &current, const config6 &page6)
{
  return current.RPM > RPM_COARSE.toUser(page6.lnchSoftLim);
}

static bool aboveFlatShiftHardRpmLimit(const statuses &current, const config2 &page2, const config15 &page15)
{
  const uint16_t flatRpmLimit = getHardCutRpmLimit(current.launchStatus.clutchEngagedRPM, page2, page15);
  return (current.RPM > flatRpmLimit);
}

static bool aboveFlatShiftSoftRpmLimit(const statuses &current, const config6 &page6)
{
  return current.RPM > (current.launchStatus.clutchEngagedRPM - RPM_COARSE.toUser(page6.flatSSoftWin));
}

TESTABLE_STATIC void updateLaunchFlagsCore(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const config15 &page15)
{
  updateClutchState(current, page6);

  bool isLaunching = isLaunchArmed(current, page6, page10)
                  && withinLaunchSpeedLimit(current, page2, page10);
  current.launchStatus.launchingHard =   isLaunching
                                      && aboveHardLaunchRpmLimit(current, page2, page6, page15);
  current.launchStatus.launchingSoft =   isLaunching
                                      && aboveSoftLaunchRpmLimit(current, page6);

  bool isFlatShifting =  !isLaunching 
                      && isFlatShiftActive(current, page6);
  current.launchStatus.flatShiftingHard =  isFlatShifting
                                        && aboveFlatShiftHardRpmLimit(current, page2, page15);
  current.launchStatus.flatShiftingSoft =  isFlatShifting
                                        && aboveFlatShiftSoftRpmLimit(current, page6);
}

// LCOV_EXCL_START
void updateLaunchAndFlatShift(statuses &current, const config2 &page2, const config6 &page6, const config10 &page10, const config15 &page15)
{
  if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_10HZ)) 
  {
    updateLaunchFlagsCore(current, page2, page6, page10, page15);
  }
}
// LCOV_EXCL_STOP