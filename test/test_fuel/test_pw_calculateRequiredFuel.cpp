#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"
#include "statuses.h"

extern uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current);

static void test_calculateRequiredFuel_2stroke(void) {
  config2 page2 = {};
  page2.reqFuel = 11; // ms*10?
  page2.strokes = TWO_STROKE;

  statuses current = {};
  page2.injLayout = INJ_PAIRED;
  BIT_CLEAR(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
  BIT_SET(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));

  page2.injLayout = INJ_SEQUENTIAL;
  BIT_CLEAR(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
  BIT_SET(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
}

static void test_calculateRequiredFuel_4stroke(void) {
  config2 page2 = {};
  page2.reqFuel = 11; // ms*10?
  page2.strokes = FOUR_STROKE;

  statuses current = {};

  page2.injLayout = INJ_PAIRED;
  BIT_CLEAR(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, current));
  BIT_SET(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, current));

  page2.injLayout = INJ_SEQUENTIAL;  
  BIT_CLEAR(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
  BIT_SET(current.status3, BIT_STATUS3_HALFSYNC);
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, current));
}


void testCalculateRequiredFuel(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateRequiredFuel_2stroke);
    RUN_TEST_P(test_calculateRequiredFuel_4stroke);
  }
}