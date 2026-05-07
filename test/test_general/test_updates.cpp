#include <unity.h>
#include "globals.h"
#include "updates.h"
#include "pages.h"
#include "table3d.h"
#include "../test_utils.h"

// =========================== multiplyTableLoad ==============================

static void test_multiplyTableLoad_doubles_y_axis(void)
{
  // Seed the boostTable Y-axis with known values, then verify each was doubled.
  populate_table_axis(y_begin(&boostTable, boostTable.type_key), (table3d_axis_t)10);
  // Force a couple of distinct values too so we don't trivially pass with all zeros.
  table_axis_iterator it = y_begin(&boostTable, boostTable.type_key);
  uint8_t i = 0;
  while (!it.at_end())
  {
    *it = (table3d_axis_t)(5 + i);
    ++it; ++i;
  }

  multiplyTableLoad(&boostTable, boostTable.type_key, 2U);

  it = y_begin(&boostTable, boostTable.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)((5 + j) * 2), *it);
    ++it; ++j;
  }
}

static void test_multiplyTableLoad_by_one_is_identity(void)
{
  table_axis_iterator it = y_begin(&boostTable, boostTable.type_key);
  uint8_t i = 0;
  while (!it.at_end()) { *it = (table3d_axis_t)(20 + i); ++it; ++i; }

  multiplyTableLoad(&boostTable, boostTable.type_key, 1U);

  it = y_begin(&boostTable, boostTable.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)(20 + j), *it);
    ++it; ++j;
  }
}

// =========================== divideTableLoad ================================

static void test_divideTableLoad_halves_y_axis(void)
{
  table_axis_iterator it = y_begin(&boostTable, boostTable.type_key);
  uint8_t i = 0;
  while (!it.at_end()) { *it = (table3d_axis_t)((10 + i) * 2); ++it; ++i; }

  divideTableLoad(&boostTable, boostTable.type_key, 2U);

  it = y_begin(&boostTable, boostTable.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)(10 + j), *it);
    ++it; ++j;
  }
}

static void test_multiplyDivideTableLoad_round_trip(void)
{
  table_axis_iterator it = y_begin(&boostTable, boostTable.type_key);
  uint8_t i = 0;
  while (!it.at_end()) { *it = (table3d_axis_t)(7 + i); ++it; ++i; }

  multiplyTableLoad(&boostTable, boostTable.type_key, 4U);
  divideTableLoad(&boostTable, boostTable.type_key, 4U);

  it = y_begin(&boostTable, boostTable.type_key);
  uint8_t j = 0;
  while (!it.at_end())
  {
    TEST_ASSERT_EQUAL((table3d_axis_t)(7 + j), *it);
    ++it; ++j;
  }
}

// =========================== multiplyTableValue / divideTableValue ==========
//
// These walk the entire page-as-bytes (including non-table fields). Pages 1
// and 3 hold scalar config (no tables) so we work on a small table-free page
// to avoid dragging the whole 3D map into the assertion. Page 1 is used
// because getPageSize/setPageValue are page-aware.

static void test_multiplyTableValue_scales_each_byte(void)
{
  // Pick a small enough page and stamp it with known values.
  constexpr uint8_t pageNum = 1U;
  uint16_t count = getPageSize(pageNum);
  TEST_ASSERT_GREATER_THAN_UINT16(0U, count);

  for (uint16_t i = 0U; i < count; ++i) { setPageValue(pageNum, i, 2U); }

  multiplyTableValue(pageNum, 3U);

  for (uint16_t i = 0U; i < count; ++i)
  {
    TEST_ASSERT_EQUAL_UINT8(6U, getPageValue(pageNum, i));
  }
}

static void test_divideTableValue_scales_each_byte(void)
{
  constexpr uint8_t pageNum = 1U;
  uint16_t count = getPageSize(pageNum);

  for (uint16_t i = 0U; i < count; ++i) { setPageValue(pageNum, i, 12U); }

  divideTableValue(pageNum, 4U);

  for (uint16_t i = 0U; i < count; ++i)
  {
    TEST_ASSERT_EQUAL_UINT8(3U, getPageValue(pageNum, i));
  }
}

void testUpdates(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_multiplyTableLoad_doubles_y_axis);
    RUN_TEST(test_multiplyTableLoad_by_one_is_identity);
    RUN_TEST(test_divideTableLoad_halves_y_axis);
    RUN_TEST(test_multiplyDivideTableLoad_round_trip);
    RUN_TEST(test_multiplyTableValue_scales_each_byte);
    RUN_TEST(test_divideTableValue_scales_each_byte);
  }
}
