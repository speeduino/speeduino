#include <unity.h>
#include "table3d.h"
#include "../test_utils.h"

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

void test3DTableUtils()
{
  SET_UNITY_FILENAME() {

    RUN_TEST(test_xyPair_equality);
    RUN_TEST(test_invalidateCache);
  }  
}