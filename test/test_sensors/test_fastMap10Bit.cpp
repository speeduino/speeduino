#include <unity.h>
#include "../test_utils.h"

extern int16_t fastMap10Bit(uint16_t value, int16_t rangeMin, int16_t rangeMax);

static void test_fastMap10Bit_negative_range(void) {
    TEST_ASSERT_EQUAL_INT(-1500, fastMap10Bit(0, -1500, -500));
    TEST_ASSERT_EQUAL_INT(-501, fastMap10Bit(1023, -1500, -500));
    // Greater than 10-bit input
    TEST_ASSERT_NOT_EQUAL_INT(-501, fastMap10Bit(1023*2, -1500, -500));
}

static void test_fastMap10Bit_positive_range(void) {
    TEST_ASSERT_EQUAL_INT(500, fastMap10Bit(0, 500, 1500));
    TEST_ASSERT_EQUAL_INT(1499, fastMap10Bit(1023, 500, 1500));
    // Greater than 10-bit input
    TEST_ASSERT_NOT_EQUAL_INT(1499, fastMap10Bit(1023*2, 500, 1500));
}

void test_fastMap10Bit(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_fastMap10Bit_negative_range);
    RUN_TEST(test_fastMap10Bit_positive_range);
  }    
}
