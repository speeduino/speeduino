#include "../../test_utils.h"

void testDualWheel()
{
  extern void testDualWheelPerToothIgn(void);
  extern void testDualWheelCrankAngle(void);
  extern void testDualWheelRevTime(void);

  testDualWheelPerToothIgn();
  testDualWheelCrankAngle();
  testDualWheelRevTime();
}