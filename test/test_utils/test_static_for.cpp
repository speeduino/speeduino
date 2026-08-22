#include "../test_utils.h"
#include "src/utils/static_for.hpp"

static void test_static_for(void)
{
    uint32_t sum = 0;

    auto accumulate = [&sum](uint8_t i) {
        sum = sum + ((uint16_t)i*(uint16_t)i);
    };
    static_for<133>(accumulate);
    TEST_ASSERT_EQUAL(775390, sum);
}

void testStaticFor(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_static_for);
  }
}
