#include "../test_utils.h"
#include "globals.h"
#include "timers.h"

void setup_oneMsInterval(void)
{
  initialiseTimers();
  ms_counter = 0UL;
  runSecsX10 = 0;

  // Default the global config state to "do nothing" branches inside oneMSInterval().
  configPage2.tachoDiv = 0U;
  configPage2.tachoDuration = 6U;
  configPage2.fanEnable = 0U;
  configPage2.flexEnabled = false;
  configPage2.fpPrime = 0U;
  configPage2.primingDelay = 0U;
  configPage4.useDwellLim = 0U;
  configPage4.crankRPM = 40U;  // (* 10 = 400 RPM cranking)
  configPage13.hwTestInjDuration = 1U;
  configPage13.hwTestIgnDuration = 1U;

  currentStatus.RPM = 0U;
  currentStatus.runSecs = 0U;
  currentStatus.secl = 0U;
  currentStatus.loopsPerSecond = 0U;
  currentStatus.rpmDOT = 0;
  currentStatus.rotationStatus = EngineRotationStatus::Stopped;
  currentStatus.tachoSweepEnabled = false;
  currentStatus.tachoAlt = false;
  currentStatus.injPrimed = true;    // Skip injector priming branch
  currentStatus.initialisationComplete = true;
  currentStatus.isTestModeActive = false;
}
