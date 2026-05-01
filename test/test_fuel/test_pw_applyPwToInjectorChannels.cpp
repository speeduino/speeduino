#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"
#include "scheduler.h"
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

#define TEST_PW(index, current, expected, isIndexValid) \
  if ((isIndexValid)) { \
    TEST_ASSERT_UINT16_WITHIN(1, expected, fuelSchedule##index.pw); \
  } else { \
    TEST_ASSERT_EQUAL_UINT16(0, fuelSchedule##index.pw); \
  }
#if INJ_CHANNELS >= 2
  #define TEST_PW2(current, expected, isIndexValid) TEST_PW(2, current, expected, isIndexValid)
#else
  #define TEST_PW2(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 3
  #define TEST_PW3(current, expected, isIndexValid) TEST_PW(3, current, expected, isIndexValid)
#else
  #define TEST_PW3(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 4
  #define TEST_PW4(current, expected, isIndexValid) TEST_PW(4, current, expected, isIndexValid)
#else
  #define TEST_PW4(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 4
  #define TEST_PW4(current, expected, isIndexValid) TEST_PW(4, current, expected, isIndexValid)
#else
  #define TEST_PW4(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 5
  #define TEST_PW5(current, expected, isIndexValid) TEST_PW(5, current, expected, isIndexValid)
#else
  #define TEST_PW5(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 6
  #define TEST_PW6(current, expected, isIndexValid) TEST_PW(6, current, expected, isIndexValid)
#else
  #define TEST_PW6(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 7
  #define TEST_PW7(current, expected, isIndexValid) TEST_PW(7, current, expected, isIndexValid)
#else
  #define TEST_PW7(current, expected, isIndexValid)
#endif
#if INJ_CHANNELS >= 8
  #define TEST_PW8(current, expected, isIndexValid) TEST_PW(8, current, expected, isIndexValid)
#else
  #define TEST_PW8(current, expected, isIndexValid)
#endif

static void test_No_Secondary_PW(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 100, 0 /* Zero signal no staging */};
  page2.nCylinders = 2;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);
  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.primary, INJ_CHANNELS >= 7);
  TEST_PW8(current, pulseWidths.primary, INJ_CHANNELS >= 8);

  current.maxInjOutputs = INJ_CHANNELS/2;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);
  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS/2 >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS/2 >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS/2 >= 4);
  TEST_PW5(current, pulseWidths.primary, INJ_CHANNELS/2 >= 5);
  TEST_PW6(current, pulseWidths.primary, INJ_CHANNELS/2 >= 6);
  TEST_PW7(current, pulseWidths.primary, INJ_CHANNELS/2 >= 7);
  TEST_PW8(current, pulseWidths.primary, INJ_CHANNELS/2 >= 8);
}

static void test_Cylinders_1(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 1;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.secondary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.secondary, false);
  TEST_PW4(current, pulseWidths.secondary, false);
  TEST_PW5(current, pulseWidths.secondary, false);
  TEST_PW6(current, pulseWidths.secondary, false);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}

static void test_Cylinders_2(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 2;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.secondary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, false);
  TEST_PW6(current, pulseWidths.secondary, false);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}


static void test_Cylinders_3(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 3;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}

static void test_Cylinders_4_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 4;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.secondary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, false);
  TEST_PW6(current, pulseWidths.secondary, false);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}

static void test_Cylinders_4_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 4;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, INJ_CHANNELS >= 7);
  TEST_PW8(current, pulseWidths.secondary, INJ_CHANNELS >= 8);
}

static void test_Cylinders_5_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 5;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}


static void test_Cylinders_5_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 5;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}

static void test_Cylinders_6_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 6;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}

static void test_Cylinders_6_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 6;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, INJ_CHANNELS >= 7);
  TEST_PW8(current, pulseWidths.secondary, INJ_CHANNELS >= 8);
}

static void test_Cylinders_8_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.secondary, INJ_CHANNELS >= 7);
  TEST_PW8(current, pulseWidths.secondary, INJ_CHANNELS >= 8);
}

static void test_Cylinders_8_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.primary, INJ_CHANNELS >= 7);
  TEST_PW8(current, pulseWidths.primary, INJ_CHANNELS >= 8);
}

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

static void test_Cylinders_2_sequential_trimmed(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 2;
  page2.injLayout = INJ_SEQUENTIAL;

  page6.fuelTrimEnabled = 1U;
  zeroTrimTables();
  setupTrimTable(trimTables[0], -50);
  setupTrimTable(trimTables[1], 50);
  setupTrimTable(trimTables[2], 33);
  setupTrimTable(trimTables[3], -33);
  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary/2U, true);
  TEST_PW2(current, (pulseWidths.primary*3U)/2U, INJ_CHANNELS >= 3);
  // Secondary channels do NOT have trims applied
  TEST_PW3(current, pulseWidths.secondary, INJ_CHANNELS >= 3);
  TEST_PW4(current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.secondary, false);
  TEST_PW6(current, pulseWidths.secondary, false);
  TEST_PW7(current, pulseWidths.secondary, false);
  TEST_PW8(current, pulseWidths.secondary, false);
}

static void test_Cylinders_8_sequential_trimmed(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  config6 page6 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_SEQUENTIAL;
  
  page6.fuelTrimEnabled = 1U;
  zeroTrimTables();
  setupTrimTable(trimTables[0], -50);
  setupTrimTable(trimTables[1], 0);
  setupTrimTable(trimTables[2], 50);
  setupTrimTable(trimTables[3], -33);
  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, page6, current);

  TEST_PW(1, current, pulseWidths.primary/2U, true);
  TEST_PW2(current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW3(current, (pulseWidths.primary*3U)/2U, INJ_CHANNELS >= 3);
  TEST_PW4(current, (pulseWidths.primary*2U)/3U, INJ_CHANNELS >= 4);
  TEST_PW5(current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW6(current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW7(current, pulseWidths.primary, INJ_CHANNELS >= 7);
  TEST_PW8(current, pulseWidths.primary, INJ_CHANNELS >= 8);
}

void testApplyPwToInjectorChannels(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_No_Secondary_PW);
    RUN_TEST_P(test_Cylinders_1);
    RUN_TEST_P(test_Cylinders_2);
    RUN_TEST_P(test_Cylinders_3);
    RUN_TEST_P(test_Cylinders_4_paired);
    RUN_TEST_P(test_Cylinders_4_sequential);
    RUN_TEST_P(test_Cylinders_5_paired);
    RUN_TEST_P(test_Cylinders_5_sequential);
    RUN_TEST_P(test_Cylinders_6_paired);
    RUN_TEST_P(test_Cylinders_6_sequential);
    RUN_TEST_P(test_Cylinders_8_paired);
    RUN_TEST_P(test_Cylinders_8_sequential);
    RUN_TEST_P(test_Cylinders_2_sequential_trimmed);
    RUN_TEST_P(test_Cylinders_8_sequential_trimmed);
  }
}
