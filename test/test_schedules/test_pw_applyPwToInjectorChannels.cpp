#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"
#include "scheduler_fuel_controller.h"
#include "units.h"

static statuses getRandomPW(void) {
  statuses current = {};
  
  randomSeed(analogRead(0));

  fuelSchedule1.pw = random(3, UINT16_MAX);
#if INJ_CHANNELS >= 2
  fuelSchedule2.pw = random(3, UINT16_MAX);
#endif
#if INJ_CHANNELS >= 3
  fuelSchedule3.pw = random(3, UINT16_MAX);
#endif
#if INJ_CHANNELS >= 4
  fuelSchedule4.pw = random(3, UINT16_MAX);
#endif
#if INJ_CHANNELS >= 5
  fuelSchedule5.pw = random(3, UINT16_MAX);
#endif
#if INJ_CHANNELS >= 6
  fuelSchedule6.pw = random(3, UINT16_MAX);
#endif
#if INJ_CHANNELS >= 7
  fuelSchedule7.pw = random(3, UINT16_MAX);
#endif
#if INJ_CHANNELS >= 8
  fuelSchedule8.pw = random(3, UINT16_MAX);
#endif

  return current;
}

static statuses setPrimarySecondaryChannels(statuses current, uint8_t primary, uint8_t secondary) {
  current.numPrimaryInjOutputs = INJ_CHANNELS>=primary ? primary : INJ_CHANNELS;
  current.numSecondaryInjOutputs = INJ_CHANNELS>=(primary+secondary) ? secondary : (INJ_CHANNELS>primary ? INJ_CHANNELS-primary : 0U);
  return current;
}

#define TEST_PW(index, expected, isIndexValid) \
  if ((isIndexValid)) { \
    TEST_ASSERT_UINT16_WITHIN(1, expected, fuelSchedule##index.pw); \
  } else { \
    TEST_ASSERT_EQUAL_UINT16(0, fuelSchedule##index.pw); \
  }
#if INJ_CHANNELS >= 2
  #define TEST_PW2(expected, isIndexValid) TEST_PW(2, expected, isIndexValid)
#else
  #define TEST_PW2(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 3
  #define TEST_PW3(expected, isIndexValid) TEST_PW(3, expected, isIndexValid)
#else
  #define TEST_PW3(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 4
  #define TEST_PW4(expected, isIndexValid) TEST_PW(4, expected, isIndexValid)
#else
  #define TEST_PW4(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 4
  #define TEST_PW4(expected, isIndexValid) TEST_PW(4, expected, isIndexValid)
#else
  #define TEST_PW4(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 5
  #define TEST_PW5(expected, isIndexValid) TEST_PW(5, expected, isIndexValid)
#else
  #define TEST_PW5(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 6
  #define TEST_PW6(expected, isIndexValid) TEST_PW(6, expected, isIndexValid)
#else
  #define TEST_PW6(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 7
  #define TEST_PW7(expected, isIndexValid) TEST_PW(7, expected, isIndexValid)
#else
  #define TEST_PW7(expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 8
  #define TEST_PW8(expected, isIndexValid) TEST_PW(8, expected, isIndexValid)
#else
  #define TEST_PW8(expected, isIndexValid)
#endif

static void setupTrimTable(trimTable3d &trimTable, int8_t percent)
{
  fill_table_values(trimTable, FUEL_TRIM.toRaw(percent));
}

static void zeroTrimTables(void)
{
  for (uint8_t i=0; i<_countof(trimTables); ++i)
  {
    setupTrimTable(trimTables[i], 0);
  }
}

static inline uint16_t getExpectedChannelPw(const statuses &current, const pulseWidths &widths, uint8_t channel, uint8_t trimPct)
{
  if (channel<=current.numPrimaryInjOutputs) {
    return percentageApprox((uint8_t)(100U+trimPct), widths.primary);
  }
  if (channel<=getTotalInjChannelCount(current)) {
    // Secondary channels do NOT have trims applied
    return widths.secondary;
  }
  return 0U;
}

static uint8_t primaries;
static uint8_t secondaries;

static void test_noTrim_inner(void)
{
  statuses current = setPrimarySecondaryChannels(getRandomPW(), primaries, secondaries);
  config2 page2 = {};
  config4 page4 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };

  char szMsg[128];
  sprintf(szMsg, "cp:%" PRIu8 " cs:%" PRIu8, current.numPrimaryInjOutputs, current.numSecondaryInjOutputs);
  TEST_MESSAGE(szMsg);
  applyPwToInjectorChannels(pulseWidths, page2, page4, page6, current);
  TEST_PW(1, getExpectedChannelPw(current, pulseWidths, 1, 0U), true);
  TEST_PW2(getExpectedChannelPw(current, pulseWidths, 2, 0U), true);
  TEST_PW3(getExpectedChannelPw(current, pulseWidths, 3, 0U), true);
  TEST_PW4(getExpectedChannelPw(current, pulseWidths, 4, 0U), true);
  TEST_PW5(getExpectedChannelPw(current, pulseWidths, 5, 0U), true);
  TEST_PW6(getExpectedChannelPw(current, pulseWidths, 6, 0U), true);
  TEST_PW7(getExpectedChannelPw(current, pulseWidths, 7, 0U), true);
  TEST_PW8(getExpectedChannelPw(current, pulseWidths, 8, 0U), true); 
}

static void test_withTrim_inner(void)
{
  statuses current = setPrimarySecondaryChannels(getRandomPW(), primaries, secondaries);
  config2 page2 = {};
  config4 page4 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page6.fuelTrimEnabled = true;
  zeroTrimTables();
  setupTrimTable(trimTables[0], -50);
#if INJ_CHANNELS >= 2
  setupTrimTable(trimTables[1], 50);
#endif
#if INJ_CHANNELS >= 3
  setupTrimTable(trimTables[2], 33);
#endif
#if INJ_CHANNELS >= 4
  setupTrimTable(trimTables[3], -33);
#endif

  char szMsg[128];
  sprintf(szMsg, "cp:%" PRIu8 " cs:%" PRIu8, current.numPrimaryInjOutputs, current.numSecondaryInjOutputs);
  TEST_MESSAGE(szMsg);
  applyPwToInjectorChannels(pulseWidths, page2, page4, page6, current);
  TEST_PW(1, getExpectedChannelPw(current, pulseWidths, 1, -50), true);
  TEST_PW2(getExpectedChannelPw(current, pulseWidths, 2, 50), true);
  TEST_PW3(getExpectedChannelPw(current, pulseWidths, 3, 33), true);
  TEST_PW4(getExpectedChannelPw(current, pulseWidths, 4, -33), true);
  TEST_PW5(getExpectedChannelPw(current, pulseWidths, 5, 0U), true);
  TEST_PW6(getExpectedChannelPw(current, pulseWidths, 6, 0U), true);
  TEST_PW7(getExpectedChannelPw(current, pulseWidths, 7, 0U), true);
  TEST_PW8(getExpectedChannelPw(current, pulseWidths, 8, 0U), true); 
}

void testApplyPwToInjectorChannels(void)
{
  SET_UNITY_FILENAME() {
    for (primaries=1; primaries<=INJ_CHANNELS; ++primaries)
    {
      for (secondaries=0; secondaries<=INJ_CHANNELS; ++secondaries)
      {    
        char szName[128];
        sprintf(szName, "test_noTrim_inner_p%" PRIu8 "_s%" PRIu8, primaries, secondaries);
        UnityDefaultTestRun(test_noTrim_inner, szName, __LINE__);

        sprintf(szName, "test_withTrim_inner_p%" PRIu8 "_s%" PRIu8, primaries, secondaries);
        UnityDefaultTestRun(test_withTrim_inner, szName, __LINE__);
      }
    }
  }
}
