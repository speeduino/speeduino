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

static void assert_row(uint8_t rowNum, table_row_iterator row, const table3d8RpmLoad &testTable)
{
  TEST_ASSERT_EQUAL(row.size(), testTable.width());

  uint8_t axisSize = testTable.width();
  uint8_t colNum=0;
  while (!row.at_end())
  {
    uint8_t indexRowStart = rowNum * axisSize;
    uint16_t address = indexRowStart + colNum;    
    char szMsg[32];
    snprintf(szMsg, _countof(szMsg)-1, "[%" PRIu8 ",%" PRIu8 "][%" PRIu16 "]", rowNum, colNum, address);
    TEST_ASSERT_EQUAL_MESSAGE(testTable.values[address], *row, szMsg);
    ++row;
    ++colNum;
  }  
}

static void test_rows_begin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto rows = rows_begin(&testTable, testTable.type_key);
  uint8_t rowNum = 0;
  while (!rows.at_end())
  {
    assert_row(rowNum, *rows, testTable);
    ++rows;
    ++rowNum;
  }
}

static void test_x_rbegin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = x_rbegin(&testTable, testTable.type_key);
  for (auto it = testTable.axisX.crbegin(); it != testTable.axisX.crend(); ++it) {
    TEST_ASSERT_EQUAL(*it, *axis);
    ++axis;
  }  
}

static void test_x_begin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = x_begin(&testTable, testTable.type_key);
  for (auto it = testTable.axisX.cbegin(); it != testTable.axisX.cend(); ++it) {
    TEST_ASSERT_EQUAL(*it, *axis);
    ++axis;
  }  
}

static void test_y_rbegin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = y_rbegin(&testTable, testTable.type_key);
  for (auto it = testTable.axisY.crbegin(); it != testTable.axisY.crend(); ++it) {
    TEST_ASSERT_EQUAL(*it, *axis);
    ++axis;
  }
}

static void test_y_begin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = y_begin(&testTable, testTable.type_key);
  for (auto it = testTable.axisY.cbegin(); it != testTable.axisY.cend(); ++it) {
    TEST_ASSERT_EQUAL(*it, *axis);
    ++axis;
  }  
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

    RUN_TEST(test_xyPair_equality);
    RUN_TEST(test_invalidateCache);
    RUN_TEST(test_rows_begin);
    RUN_TEST(test_x_begin);
    RUN_TEST(test_x_rbegin);
    RUN_TEST(test_y_begin);
    RUN_TEST(test_y_rbegin);
    RUN_TEST_P(test_toTopRight);
    RUN_TEST_P(test_value_at);
  }  
}