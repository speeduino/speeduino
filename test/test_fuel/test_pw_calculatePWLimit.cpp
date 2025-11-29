#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"

extern uint16_t calculatePWLimit(const config2 &page2, const statuses &current, uint32_t revTime);

//Tests the PW Limit calculation for a normal scenario
void test_calculatePWLimit_90pct(void)
{
  config2 page2 = {};
  statuses current = {};

  page2.strokes = TWO_STROKE;
  page2.dutyLim = 90;
  current.nSquirts = 1;

  //Duty limit of 90% for 10,000uS should give 9,000
  TEST_ASSERT_INT32_WITHIN(4, 9000, calculatePWLimit(page2, current, 10000UL));
}

static void test_calculatePWLimit_squirts(void)
{
  config2 page2 = {};
  statuses current = {};

  page2.strokes = TWO_STROKE;
  page2.dutyLim = 90;

  //Duty limit of 90% for 10,000uS should give 9,000
  current.nSquirts = 1;
  TEST_ASSERT_INT32_WITHIN(4, 9000, calculatePWLimit(page2, current, 10000UL));

  current.nSquirts = 2;
  TEST_ASSERT_INT32_WITHIN(4, 4500, calculatePWLimit(page2, current, 10000UL));
  current.nSquirts = 3;
  TEST_ASSERT_INT32_WITHIN(4, 3000, calculatePWLimit(page2, current, 10000UL));
  current.nSquirts = 4;
  TEST_ASSERT_INT32_WITHIN(4, 2250, calculatePWLimit(page2, current, 10000UL));
  current.nSquirts = 5;
  TEST_ASSERT_INT32_WITHIN(4, 1800, calculatePWLimit(page2, current, 10000UL));
}

//Tests the PW Limit calculation when the revolution time is greater than the max UINT16 value
//Occurs at approx. 915rpm
void test_calculatePWLimit_Long_Revolution(void)
{
  config2 page2 = {};
  statuses current = {};

  page2.strokes = TWO_STROKE;
  page2.dutyLim = 90;
  current.nSquirts = 1;

  //Duty limit of 90% for 100,000uS should give 90,000, but as this would overflow the PW value, this should default to UINT16 Max
  TEST_ASSERT_EQUAL(UINT16_MAX, calculatePWLimit(page2, current, 100000UL));  
}


void testCalculatePWLimit(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculatePWLimit_Long_Revolution);
    RUN_TEST_P(test_calculatePWLimit_90pct);
    RUN_TEST_P(test_calculatePWLimit_squirts);
  }
}