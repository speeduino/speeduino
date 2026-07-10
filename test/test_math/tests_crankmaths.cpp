#include <unity.h>
#include "crankMaths.h"
#include "decoders.h"
#include "../test_utils.h"

extern void SetRevolutionTime(uint32_t revTime);

struct crankmaths_rev_testdata {
  uint16_t rpm;
  unsigned long revolutionTime;
  uint16_t angle;
  unsigned long expected;
} *crankmaths_rev_testdata_current;

void test_crankmaths_angletotime_revolution_execute() {
  crankmaths_rev_testdata *testdata = crankmaths_rev_testdata_current;
  SetRevolutionTime(testdata->revolutionTime);
  TEST_ASSERT_INT32_WITHIN(1, testdata->expected, angleToTimeMicroSecPerDegree(testdata->angle));
}

void testCrankMaths()
{
  SET_UNITY_FILENAME() {  
    constexpr byte testNameLength = 200;
    char testName[testNameLength];

    const crankmaths_rev_testdata crankmaths_rev_testdatas[] = {
      { .rpm = 50,    .revolutionTime = 1200000, .angle = 0,   .expected = 0 },
      { .rpm = 50,    .revolutionTime = 1200000, .angle = 25,  .expected = 83333 }, // 83333,3333
      { .rpm = 50,    .revolutionTime = 1200000, .angle = 720, .expected = 2400000 },
      { .rpm = 2500,  .revolutionTime = 24000,   .angle = 0,   .expected = 0 },
      { .rpm = 2500,  .revolutionTime = 24000,   .angle = 25,  .expected = 1667 }, // 1666,6666
      { .rpm = 2500,  .revolutionTime = 24000,   .angle = 720, .expected = 48000 },
      { .rpm = 20000, .revolutionTime = 3000,    .angle = 0,   .expected = 0 },
      { .rpm = 20000, .revolutionTime = 3000,    .angle = 25,  .expected = 208 }, // 208,3333
      { .rpm = 20000, .revolutionTime = 3000,    .angle = 720, .expected = 6000 }
    };

    for (auto testdata : crankmaths_rev_testdatas) {
      crankmaths_rev_testdata_current = &testdata;
      snprintf(testName, testNameLength, "crankmaths/angletotime/revolution/%urpm/%uangle", testdata.rpm, testdata.angle);
      UnityDefaultTestRun(test_crankmaths_angletotime_revolution_execute, testName, __LINE__);
    }
  }
}