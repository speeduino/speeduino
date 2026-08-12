#include <unity.h>
#include "table3d.h"
#include "../test_utils.h"
#include "table3d_test_support.h"

static void test_xyPair_equality(void)
{
    xy_pair_t lhs;
    xy_pair_t rhs;

    lhs = { 1, 2 };
    rhs = { 1, 2 };
    TEST_ASSERT_TRUE(lhs==rhs);

    lhs = { 1, 0 };
    TEST_ASSERT_FALSE(lhs==rhs);

    lhs = { 0, 1 };
    TEST_ASSERT_FALSE(lhs==rhs);
}

static void test_invalidateCache(void)
{
  xy_pair_t lastLookup = { 33, 47 };
  table3DGetValueCache cacheA;
  cacheA.last_lookup = lastLookup;
  invalidate_cache(&cacheA);
  TEST_ASSERT_FALSE(lastLookup==cacheA.last_lookup);
}

extern row_col2d toTopRight(const xy_coord2d &axisCoords, const table3d_dim_t &axisSize);

static void test_toTopRight(void)
{
  auto subject = toTopRight({7,0},8);
  TEST_ASSERT_EQUAL(0, subject.row);
  TEST_ASSERT_EQUAL(7, subject.col);    

  subject = toTopRight({7,4},8);
  TEST_ASSERT_EQUAL(32, subject.row);
  TEST_ASSERT_EQUAL(7, subject.col);    

  subject = toTopRight({7,7},8);
  TEST_ASSERT_EQUAL(56, subject.row);
  TEST_ASSERT_EQUAL(7, subject.col);    

  subject = toTopRight({3,0},8);
  TEST_ASSERT_EQUAL(0, subject.row);
  TEST_ASSERT_EQUAL(3, subject.col);    

  subject = toTopRight({3,4},8);
  TEST_ASSERT_EQUAL(32, subject.row);
  TEST_ASSERT_EQUAL(3, subject.col);    

  subject = toTopRight({3,7},8);
  TEST_ASSERT_EQUAL(56, subject.row);
  TEST_ASSERT_EQUAL(3, subject.col);    

  subject = toTopRight({0,0},8);
  TEST_ASSERT_EQUAL(0, subject.row);
  TEST_ASSERT_EQUAL(0, subject.col);    

  subject = toTopRight({0,4},8);
  TEST_ASSERT_EQUAL(32, subject.row);
  TEST_ASSERT_EQUAL(0, subject.col);    

  subject = toTopRight({0,7},8);
  TEST_ASSERT_EQUAL(56, subject.row);
  TEST_ASSERT_EQUAL(0, subject.col);    
}

static void test_value_at(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  table3d_value_t lastValue = 0;
  for (uint8_t i=0; i<testTable.axisX.size()*testTable.axisY.size(); ++i)
  {
    table3d_value_t newValue = testTable.values[i];
    TEST_ASSERT_GREATER_THAN(lastValue, newValue);
    lastValue = newValue;
  }
}

void test3DTableUtils()
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_xyPair_equality);
    RUN_TEST_P(test_invalidateCache);
    RUN_TEST_P(test_toTopRight);
    RUN_TEST_P(test_value_at);
  }  
}