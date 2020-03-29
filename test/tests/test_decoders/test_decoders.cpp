
#include <Arduino.h>
#include <unity.h>

#include "test_decoders.h"
#include "missing_tooth/missing_tooth.h"
#include "dual_wheel/dual_wheel.h"

void testDecoders()
{
  testMissingTooth();
  testDualWheel();
}