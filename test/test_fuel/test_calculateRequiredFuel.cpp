#include <unity.h>
#include "../test_utils.h"


extern uint16_t calculateRequiredFuel(const config2 &page2);

static void test_calculateRequiredFuel_2stroke(void) {
  config2 page2;
  page2.reqFuel = 11; // ms*10?
  page2.strokes = TWO_STROKE;
  page2.nCylinders = 1;

  page2.injLayout = INJ_PAIRED;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2));
  page2.nCylinders = INJ_CHANNELS+1;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2));

  page2.nCylinders = 1;
  page2.injLayout = INJ_SEQUENTIAL;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2));
  page2.nCylinders = INJ_CHANNELS+1;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2));
}

static void test_calculateRequiredFuel_4stroke(void) {
  config2 page2;
  page2.reqFuel = 11; // ms*10?
  page2.strokes = FOUR_STROKE;
  page2.nCylinders = 1;

  page2.injLayout = INJ_PAIRED;  
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2));
  page2.nCylinders = INJ_CHANNELS+1;
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2));

  page2.nCylinders = 1;
  page2.injLayout = INJ_SEQUENTIAL;  
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2));
  page2.nCylinders = INJ_CHANNELS+1;
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2));
}


void testCalculateRequiredFuel(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateRequiredFuel_2stroke);
    RUN_TEST_P(test_calculateRequiredFuel_4stroke);
  }
}