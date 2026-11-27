#include "pwm.h"
#include "globals.h"
#include "unit_testing.h"
#include "units.h"

TESTABLE_CONSTEXPR table2D_u8_u8_10 iacStepTable(&configPage6.iacBins, &configPage6.iacOLStepVal);
TESTABLE_CONSTEXPR table2D_u8_u8_4 iacCrankStepsTable(&configPage6.iacCrankBins, &configPage6.iacCrankSteps);

namespace idleController {

namespace stepper {

uint16_t lookupCrankingSteps(const statuses &current)
{
  return IAC_STEPS.toUser(table2D_getValue(&iacCrankStepsTable, temperatureAddOffset(current.coolant)));
}

uint16_t lookupRunningSteps(const statuses &current)
{
  return IAC_STEPS.toUser(table2D_getValue(&iacStepTable, temperatureAddOffset(current.coolant)));
}

}
}