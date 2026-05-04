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
  TEST_ASSERT_EQUAL(row.size(), testTable.values.row_size);

  uint8_t axisSize = testTable.values.row_size;
  uint8_t colNum=0;
  while (!row.at_end())
  {
    uint8_t indexRowStart = (axisSize - rowNum - UINT8_C(1))* axisSize;
    uint16_t address = indexRowStart + colNum;    
    char szMsg[32];
    sprintf(szMsg, "[%" PRIu8 ",%" PRIu8 "][%" PRIu16 "]", rowNum, colNum, address);
    TEST_ASSERT_EQUAL_MESSAGE(testTable.values.values[address], *row, szMsg);
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

static void test_x_begin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = x_begin(&testTable, testTable.type_key);
  uint8_t index = testTable.axisX.length-1U;
  while (!axis.at_end())
  {
    TEST_ASSERT_EQUAL(testTable.axisX.axis[index], *axis);
    --index; ++axis;
  }
}


static void test_x_rbegin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = x_rbegin(&testTable, testTable.type_key);
  uint8_t index = 0U;
  while (!axis.at_end())
  {
    TEST_ASSERT_EQUAL(testTable.axisX.axis[index], *axis);
    ++index; ++axis;
  }
}

static void test_y_begin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = y_begin(&testTable, testTable.type_key);
  uint8_t index = testTable.axisY.length-1U;
  while (!axis.at_end())
  {
    TEST_ASSERT_EQUAL(testTable.axisY.axis[index], *axis);
    --index; ++axis;
  }
}

static void test_y_rbegin(void)
{
  table3d8RpmLoad testTable = getDummyTable();

  auto axis = y_rbegin(&testTable, testTable.type_key);
  uint8_t index = 0U;
  while (!axis.at_end())
  {
    TEST_ASSERT_EQUAL(testTable.axisY.axis[index], *axis);
    ++index; ++axis;
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
  }  
}