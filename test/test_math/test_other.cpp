#include <unity.h>
#include "test_fp_support.h"
#include "maths.h"
#include "../test_utils.h"

static void test_nudge_i16(void)
{
  // value below min should be nudged upwards
  TEST_ASSERT_EQUAL_INT16(8, nudge((int16_t)10, (int16_t)20, (int16_t)5, (int16_t)3));

  // value above max should be nudged downwards
  TEST_ASSERT_EQUAL_INT16(23, nudge((int16_t)10, (int16_t)20, (int16_t)25, (int16_t)2));

  // value within range should be unchanged
  TEST_ASSERT_EQUAL_INT16(15, nudge((int16_t)10, (int16_t)20, (int16_t)15, (int16_t)5));

  // value equal to min should be unchanged
  TEST_ASSERT_EQUAL_INT16(10, nudge((int16_t)10, (int16_t)20, (int16_t)10, (int16_t)4));

  // value equal to max should be unchanged
  TEST_ASSERT_EQUAL_INT16(20, nudge((int16_t)10, (int16_t)20, (int16_t)20, (int16_t)4));
}

// Tests for clamp()
static void test_clamp(void)
{
  // int8_t tests
  TEST_ASSERT_EQUAL_INT8(10, clamp((int8_t)0, (int8_t)10, (int8_t)127));
  TEST_ASSERT_EQUAL_INT8(80, clamp((int8_t)80, (int8_t)10, (int8_t)127));
  TEST_ASSERT_EQUAL_INT8(120, clamp((int8_t)127, (int8_t)10, (int8_t)120));

  // uint8_t tests
  TEST_ASSERT_EQUAL_UINT8(10, clamp((uint8_t)0, (uint8_t)10, (uint8_t)200));
  TEST_ASSERT_EQUAL_UINT8(150, clamp((uint8_t)150, (uint8_t)10, (uint8_t)200));
  TEST_ASSERT_EQUAL_UINT8(200, clamp((uint8_t)250, (uint8_t)10, (uint8_t)200));
  TEST_ASSERT_EQUAL_CHAR((unsigned char)10, clamp((unsigned char)0, (unsigned char)10, (unsigned char)200));
  TEST_ASSERT_EQUAL_UINT8((unsigned char)150, clamp((unsigned char)150, (unsigned char)10, (unsigned char)200));
  TEST_ASSERT_EQUAL_UINT8((unsigned char)200, clamp((unsigned char)250, (unsigned char)10, (unsigned char)200));

  // int16_t tests
  TEST_ASSERT_EQUAL_INT16(10, clamp((int16_t)5, (int16_t)10, (int16_t)20));
  TEST_ASSERT_EQUAL_INT16(15, clamp((int16_t)15, (int16_t)10, (int16_t)20));
  TEST_ASSERT_EQUAL_INT16(20, clamp((int16_t)25, (int16_t)10, (int16_t)20));

  // uint16_t tests
  TEST_ASSERT_EQUAL_UINT16(10, clamp((uint16_t)5, (uint16_t)10, (uint16_t)20));
  TEST_ASSERT_EQUAL_UINT16(15, clamp((uint16_t)15, (uint16_t)10, (uint16_t)20));
  TEST_ASSERT_EQUAL_UINT16(20, clamp((uint16_t)25, (uint16_t)10, (uint16_t)20));

  // int32_t tests
  TEST_ASSERT_EQUAL_INT32(10, clamp((int32_t)0, (int32_t)10, (int32_t)200));
  TEST_ASSERT_EQUAL_INT32(150, clamp((int32_t)150, (int32_t)10, (int32_t)200));
  TEST_ASSERT_EQUAL_INT32(200, clamp((int32_t)250, (int32_t)10, (int32_t)200));

  // uint32_t tests
  TEST_ASSERT_EQUAL_UINT32(10, clamp((uint32_t)0, (uint32_t)10, (uint32_t)200));
  TEST_ASSERT_EQUAL_UINT32(150, clamp((uint32_t)150, (uint32_t)10, (uint32_t)200));
  TEST_ASSERT_EQUAL_UINT32(200, clamp((uint32_t)250, (uint32_t)10, (uint32_t)200));
}

void testOther(void) {
  SET_UNITY_FILENAME() {
    RUN_TEST(test_nudge_i16);
    RUN_TEST(test_clamp);
  }
}