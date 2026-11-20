#include "../test_utils.h"
#include "src/utils/FractionalMath.h"

template <uint8_t denominator>
static void assert_fraction(const FastFraction<denominator> &fraction, uint32_t toConvert)
{
  TEST_ASSERT_EQUAL((toConvert*fraction.numerator_)/denominator, fraction*toConvert);
}

template <uint8_t denominator>
static void test_fraction(const FastFraction<denominator> &fraction)
{
  assert_fraction(fraction, 1);
  assert_fraction(fraction, UINT8_MAX);
  assert_fraction(fraction, UINT16_MAX);
}

static void test_fractions(void)
{
  TEST_ASSERT_EQUAL(1, ONE_HALF*2);
  test_fraction(ONE_HALF);

  TEST_ASSERT_EQUAL(1, ONE_QUARTER*4);
  test_fraction(ONE_QUARTER);

  TEST_ASSERT_EQUAL(1, ONE_EIGTH*8);
  test_fraction(ONE_EIGTH);

  TEST_ASSERT_EQUAL(1, ONE_SIXTEENTH*16);
  test_fraction(ONE_SIXTEENTH);  

  TEST_ASSERT_EQUAL(3, THREE_SIXTEENTHS*16);
  test_fraction(THREE_SIXTEENTHS);  

  TEST_ASSERT_EQUAL(5, FIVE_SIXTEENTHS*16);
  test_fraction(FIVE_SIXTEENTHS);  
  
  TEST_ASSERT_EQUAL(3, THREE_EIGTHS*8);
  test_fraction(THREE_EIGTHS);
  
  TEST_ASSERT_EQUAL(9, NINE_SIXTEENTHS*16);
  test_fraction(NINE_SIXTEENTHS);
  
  TEST_ASSERT_EQUAL(5, FIVE_EIGTHS*8);
  test_fraction(FIVE_EIGTHS);
  
  TEST_ASSERT_EQUAL(3, THREE_QUARTERS*4);
  test_fraction(THREE_QUARTERS);
  
  TEST_ASSERT_EQUAL(9, NINE_EIGTHS*8);
  test_fraction(NINE_EIGTHS);
  
  TEST_ASSERT_EQUAL(11, ELEVEN_EIGTHS*8);
  test_fraction(ELEVEN_EIGTHS);
  
  TEST_ASSERT_EQUAL(5, FIVE_QUARTERS*4);
  test_fraction(FIVE_QUARTERS);
  
  TEST_ASSERT_EQUAL(3, ONE_AND_HALF*2);
  test_fraction(ONE_AND_HALF);
}

void testFractions(void) {
  unity_filename_guard_t guard(__FILE__);

  RUN_TEST_P(test_fractions);
}