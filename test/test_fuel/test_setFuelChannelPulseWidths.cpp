#include <unity.h>
#include "../test_utils.h"
#include "pw_calcs.h"

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

static void test_No_Secondary_PW(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 100, 0 /* Zero signal no staging */};
  page2.nCylinders = 2;

  setFuelChannelPulseWidths(8, pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW8);
}

static void test_Cylinders_1(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 1;

  setFuelChannelPulseWidths(1, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW2);
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

  setFuelChannelPulseWidths(2, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW4);
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

  setFuelChannelPulseWidths(3, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW4);
#if INJ_CHANNELS >= 6
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
#else
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
#endif
}

static void test_Cylinders_4_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 4;
  page2.injLayout = INJ_PAIRED;

  setFuelChannelPulseWidths(4, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW4);
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

  setFuelChannelPulseWidths(4, pulseWidths, page2, current);
 #if INJ_CHANNELS >= 8  
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW8);
#else
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
#endif
}

static void test_Cylinders_5_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 5;
  page2.injLayout = INJ_PAIRED;

  setFuelChannelPulseWidths(5, pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
#if INJ_CHANNELS >= 5
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
  #if INJ_CHANNELS >= 6
    TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
  #else
    TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  #endif
#else
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
#endif
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}


static void test_Cylinders_5_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 5;
  page2.injLayout = INJ_SEQUENTIAL;

  setFuelChannelPulseWidths(5, pulseWidths, page2, current);

  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
#if INJ_CHANNELS >= 5
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  #if INJ_CHANNELS >= 6
    TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
  #else
    TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  #endif
#else
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
#endif
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_6_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 6;
  page2.injLayout = INJ_PAIRED;

  setFuelChannelPulseWidths(6, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
#if INJ_CHANNELS >= 6
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
#else
  TEST_ASSERT_EQUAL_UINT16(0, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
#endif
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
}

static void test_Cylinders_6_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 6;
  page2.injLayout = INJ_SEQUENTIAL;

  setFuelChannelPulseWidths(6, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
#if INJ_CHANNELS >= 8
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW8);
#else
  TEST_ASSERT_EQUAL_UINT16(0, current.PW4);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
#endif
}

static void test_Cylinders_8_paired(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_PAIRED;

  setFuelChannelPulseWidths(8, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
#if INJ_CHANNELS >= 8
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW8);
#else
  TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
  TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
#endif  
}

static void test_Cylinders_8_sequential(void) {
  statuses current = getRandomPW();
  config2 page2 = {};
  pulseWidths pulseWidths = { 333, 777 };
  page2.nCylinders = 8;
  page2.injLayout = INJ_SEQUENTIAL;

  setFuelChannelPulseWidths(8, pulseWidths, page2, current);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW1);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW2);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW3);
  TEST_ASSERT_EQUAL_UINT16(pulseWidths.primary, current.PW4);
// #if INJ_CHANNELS >= 8
//   TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW5);
//   TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW6);
//   TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW7);
//   TEST_ASSERT_EQUAL_UINT16(pulseWidths.secondary, current.PW8);
// #else
//   TEST_ASSERT_EQUAL_UINT16(0, current.PW5);
//   TEST_ASSERT_EQUAL_UINT16(0, current.PW6);
//   TEST_ASSERT_EQUAL_UINT16(0, current.PW7);
//   TEST_ASSERT_EQUAL_UINT16(0, current.PW8);
// #endif  
}

void testsetFuelChannelPulseWidths(void)
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