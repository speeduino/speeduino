#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"

extern uint16_t calculatePWLimit(const config2 &page2, const statuses &current);

static void assert_calculatePWLimit(const config2 &page2, statuses current, uint16_t multiplier)
{
  uint16_t baseline = percentage(page2.dutyLim, current.revolutionTime) * multiplier;
  TEST_ASSERT_INT32_WITHIN(4 * multiplier, baseline/current.nSquirts, calculatePWLimit(page2, current));
}

//Tests the PW Limit calculation for a normal scenario
void test_calculatePWLimit_90pct(void)
{
  config2 page2 = {};
  statuses current = {};

  page2.strokes = TWO_STROKE;
  page2.dutyLim = 90;
  current.nSquirts = 1;
  current.revolutionTime = 10000UL;

  assert_calculatePWLimit(page2, current, 1U);
}

static void assert_calculatePWLimit_squirts(const config2 &page2, statuses current, uint16_t multiplier)
{
  for (uint8_t squirts = 1U; squirts < 10U; squirts++)
  {
    current.nSquirts = squirts;
    assert_calculatePWLimit(page2, current, multiplier);
  }
}

static void test_calculatePWLimit_squirts_2stroke(void)
{
  config2 page2 = {};
  statuses current = {};

  page2.strokes = TWO_STROKE;
  page2.dutyLim = 90;
  current.revolutionTime = 10000UL;
  assert_calculatePWLimit_squirts(page2, current, 1U);
}

static void test_calculatePWLimit_squirts_4stroke(void)
{
  config2 page2 = {};
  statuses current = {};

  page2.strokes = FOUR_STROKE;
  page2.dutyLim = 90;
  current.revolutionTime = 10000UL;

  assert_calculatePWLimit_squirts(page2, current, 2U);
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
  current.revolutionTime = 100000UL;

  //Duty limit of 90% for 100,000uS should give 90,000, but as this would overflow the PW value, this should default to UINT16 Max
  TEST_ASSERT_EQUAL(UINT16_MAX, calculatePWLimit(page2, current));  
}


void testCalculatePWLimit(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculatePWLimit_Long_Revolution);
    RUN_TEST_P(test_calculatePWLimit_90pct);
    RUN_TEST_P(test_calculatePWLimit_squirts_2stroke);
    RUN_TEST_P(test_calculatePWLimit_squirts_4stroke);
  }
}