#include "decoders.h"
#include "crankMaths.h"
#include "../test_utils.h"
#include "globals.h"

static void test_getCrankAngle(void)
{
  extern decoder_status_t decoderStatus;
  extern volatile unsigned long toothLastToothTime;
  extern volatile uint8_t toothCurrentCount;
  extern volatile int revolutionOne;

  auto decoder = triggerSetup_24X();

  // helper to setup and call pGetCrankAngle
  auto run_case = [&](uint8_t toothNum, int16_t expected, bool revOne=false, int trigAngle=0) {
    toothLastToothTime = 2000;
    toothCurrentCount = toothNum;
    decoderStatus.toothAngleIsCorrect = true;
    revolutionOne = revOne ? 1 : 0;
    configPage4.triggerAngle = trigAngle;
    setAngleConverterRevolutionTime(2000);
    int16_t angle = decoder.pGetCrankAngle(toothLastToothTime + 100);
    TEST_ASSERT_INT16_WITHIN_MESSAGE(2, expected, angle, "24X Crank Angle");
  };

  // timeToAngle(100) ~= 18 deg with revolutionTime 2000
  const int dt_add = 18;

  // Test several teeth
  run_case(1, 12 + dt_add);   // toothAngles[0] = 12
  run_case(2, 18 + dt_add);   // toothAngles[1] = 18
  run_case(3, 33 + dt_add);   // toothAngles[2] = 33
  run_case(12, 177 + dt_add); // toothAngles[11] = 177
  run_case(24, 357 + dt_add); // toothAngles[23] = 357 -> 375

  // Cam tooth (0): should yield triggerAngle + dt_add
  run_case(0, 0 + dt_add);

  // When revolutionOne is set, result should be += 360
  run_case(0, 0 + dt_add + 360, true);

  // trigger angle offset
  run_case(1, 12 + dt_add + 10, false, 10);
}

void test24X(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_getCrankAngle);
  }
}