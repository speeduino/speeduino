#include "pwm.h"
#include "globals.h"
#include "unit_testing.h"
#include "units.h"

TESTABLE_CONSTEXPR table2D_u8_u8_4 iacCrankDutyTable(&configPage6.iacCrankBins, &configPage6.iacCrankDuty);
TESTABLE_CONSTEXPR table2D_u8_u8_10 iacPWMTable(&configPage6.iacBins, &configPage6.iacOLPWMVal);

namespace idleController {

namespace pwm {

uint8_t lookupCrankingLoad(const statuses &current)
{
  return table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(current.coolant));
}

uint8_t lookupRunningLoad(const statuses &current)
{
  return table2D_getValue(&iacPWMTable, temperatureAddOffset(current.coolant));
}

}
}