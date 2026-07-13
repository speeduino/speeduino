#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"
#include "statuses.h"
#include "decoders.h"

extern uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current);

static void test_calculateRequiredFuel_2stroke(void) {
  config2 page2 = {};
  statuses current = {};
  page2.reqFuel = 11; // ms*10?
  page2.strokes = TWO_STROKE;

  current.injLayout = INJ_PAIRED;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
  current.injLayout = INJ_SEQUENTIAL;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
}

static void test_calculateRequiredFuel_4stroke(void) {
  config2 page2 = {};
  statuses current = {};
  page2.reqFuel = 11; // ms*10?
  page2.strokes = FOUR_STROKE;

  current.injLayout = INJ_PAIRED;
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, current));
  current.injLayout = INJ_SEQUENTIAL;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, current));
}


void testCalculateRequiredFuel(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateRequiredFuel_2stroke);
    RUN_TEST_P(test_calculateRequiredFuel_4stroke);
  }
}