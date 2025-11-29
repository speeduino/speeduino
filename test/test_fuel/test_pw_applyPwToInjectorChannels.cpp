#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"
#include "globals.h"

extern void applyPwToInjectorChannels(const pulseWidths &pulse_widths, const config2 &page2, statuses &current);

static statuses getRandomPW(void) {
  statuses current = {};
  
  randomSeed(analogRead(0));

  current.PW1 = random(3, UINT16_MAX);  
  current.PW2 = random(3, UINT16_MAX);  
  current.PW3 = random(3, UINT16_MAX);  
  current.PW4 = random(3, UINT16_MAX);  
  current.PW5 = random(3, UINT16_MAX);  
  current.PW6 = random(3, UINT16_MAX);  
  current.PW7 = random(3, UINT16_MAX);  
  current.PW8 = random(3, UINT16_MAX);  

  return current;
}

#define TEST_PW(index, current, expected, isIndexValid) \
  if ((isIndexValid)) { \
    TEST_ASSERT_EQUAL_UINT16(expected, current.PW##index); \
  } else { \
    TEST_ASSERT_EQUAL_UINT16(0, current.PW##index); \
  }

static void test_No_Secondary_PW(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 100, 0 /* Zero signal no staging */};
  page2.nCylinders = 2;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW(7, current, pulseWidths.primary, INJ_CHANNELS >= 7);
  TEST_PW(8, current, pulseWidths.primary, INJ_CHANNELS >= 8);

  current.maxInjOutputs = INJ_CHANNELS/2;
  applyPwToInjectorChannels(pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS/2 >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS/2 >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS/2 >= 4);
  TEST_PW(5, current, pulseWidths.primary, INJ_CHANNELS/2 >= 5);
  TEST_PW(6, current, pulseWidths.primary, INJ_CHANNELS/2 >= 6);
  TEST_PW(7, current, pulseWidths.primary, INJ_CHANNELS/2 >= 7);
  TEST_PW(8, current, pulseWidths.primary, INJ_CHANNELS/2 >= 8);
}

static void test_Cylinders_1(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 1;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.secondary, INJ_CHANNELS >= 2);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_2(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 2;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.secondary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}


static void test_Cylinders_3(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 3;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_4_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 4;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.secondary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_4_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 4;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW(7, current, pulseWidths.secondary, INJ_CHANNELS >= 7);
  TEST_PW(8, current, pulseWidths.secondary, INJ_CHANNELS >= 8);
}

static void test_Cylinders_5_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 5;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}


static void test_Cylinders_5_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 5;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_6_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 6;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.secondary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_6_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 6;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW(7, current, pulseWidths.secondary, INJ_CHANNELS >= 7);
  TEST_PW(8, current, pulseWidths.secondary, INJ_CHANNELS >= 8);
}

static void test_Cylinders_8_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_PAIRED;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.secondary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.secondary, INJ_CHANNELS >= 6);
  TEST_PW(7, current, pulseWidths.secondary, INJ_CHANNELS >= 7);
  TEST_PW(8, current, pulseWidths.secondary, INJ_CHANNELS >= 8);
}

static void test_Cylinders_8_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_SEQUENTIAL;

  current.maxInjOutputs = INJ_CHANNELS;
  applyPwToInjectorChannels(pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_PW(2, current, pulseWidths.primary, INJ_CHANNELS >= 2);
  TEST_PW(3, current, pulseWidths.primary, INJ_CHANNELS >= 3);
  TEST_PW(4, current, pulseWidths.primary, INJ_CHANNELS >= 4);
  TEST_PW(5, current, pulseWidths.primary, INJ_CHANNELS >= 5);
  TEST_PW(6, current, pulseWidths.primary, INJ_CHANNELS >= 6);
  TEST_PW(7, current, pulseWidths.primary, INJ_CHANNELS >= 7);
  TEST_PW(8, current, pulseWidths.primary, INJ_CHANNELS >= 8);
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
  }
}
