#include "../../test_utils.h"

void testDualWheel()
{
  extern void testDualWheelTriggers(void);
  extern void testDualWheelPerToothIgn(void);
  extern void testDualWheelCrankAngle(void);
  extern void testDualWheelRevTime(void);

  testDualWheelTriggers();
  testDualWheelPerToothIgn();
  testDualWheelCrankAngle();
  testDualWheelRevTime();
}