#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"
#include "statuses.h"

extern uint16_t calculateOpenTime(const config2 &page2, const statuses &current);

static void test_calculateOpenTime_Correct(void) {
  config2 page2 = {};
  statuses current = {};

  page2.injOpen = 100;

  current.batCorrection = 66; 
  TEST_ASSERT_EQUAL(page2.injOpen * current.batCorrection, calculateOpenTime(page2, current));
  current.batCorrection = 133; 
  TEST_ASSERT_EQUAL(page2.injOpen * current.batCorrection, calculateOpenTime(page2, current));
}

void testCalculateOpenTime(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateOpenTime_Correct);
  }
}