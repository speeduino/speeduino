#include <unity.h>
#include "../test_utils.h"


extern uint16_t calculateOpenTime(const config2 &page2, const statuses &current);

static void test_calculateOpenTime_NoCorrect(void) {
  config2 page2 = {};
  statuses current = {};

  page2.battVCorMode = BATTV_COR_MODE_WHOLE;
  current.batCorrection = 66; // Should have no effect
  page2.injOpen = 100;
  TEST_ASSERT_EQUAL(page2.injOpen*100U, calculateOpenTime(page2, current));
}

static void test_calculateOpenTime_Correct(void) {
  config2 page2 = {};
  statuses current = {};

  page2.battVCorMode = BATTV_COR_MODE_OPENTIME;
  page2.injOpen = 100;

  current.batCorrection = 66; 
  TEST_ASSERT_EQUAL(page2.injOpen * current.batCorrection, calculateOpenTime(page2, current));
  current.batCorrection = 133; 
  TEST_ASSERT_EQUAL(page2.injOpen * current.batCorrection, calculateOpenTime(page2, current));
}

void testCalculateOpenTime(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateOpenTime_NoCorrect);
    RUN_TEST_P(test_calculateOpenTime_Correct);
  }
}