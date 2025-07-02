#include <unity.h>
#include "units.h"
#include "../test_utils.h"

template <typename T> struct numeric_limits; 
template <> struct numeric_limits<uint8_t> { 
    static constexpr uint8_t min = 0U;
    static constexpr uint8_t max = UINT8_MAX;
}; 
template <> struct numeric_limits<int8_t> { 
    static constexpr int8_t min = INT8_MIN;
    static constexpr int8_t max = INT8_MAX;
}; 
template <> struct numeric_limits<uint16_t> { 
    static constexpr uint16_t min = 0U;
    static constexpr uint16_t max = UINT16_MAX;
}; 
template <> struct numeric_limits<int16_t> { 
    static constexpr int16_t min = INT16_MIN;
    static constexpr int16_t max = INT16_MAX;
}; 

template <typename TUser,
          typename TRaw>
static void test_conversionFactor_t(void) {
    constexpr TRaw RAW_HALF = numeric_limits<TRaw>::min + (numeric_limits<TRaw>::max-numeric_limits<TRaw>::min)/2;
    constexpr TRaw RAW_TENTH = RAW_HALF/5;
    // constexpr TUser USER_HALF = numeric_limits<TUser>::min + (numeric_limits<TUser>::max-numeric_limits<TUser>::min)/2;
    // constexpr TUser USER_TENTH = USER_HALF/5;
    // constexpr TRaw RAW_USER_SCALE = (TRaw)((uint32_t)numeric_limits<TUser>::max/(uint32_t)numeric_limits<TRaw>::max);
    
    static_assert(sizeof(TUser)>=sizeof(TRaw), "Expected TUser to be >= TRaw");
    
    constexpr conversionFactor<TUser, TRaw> IDENTITY = {1, 0};
    TEST_ASSERT_EQUAL(0, IDENTITY.toUser(0));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max, IDENTITY.toUser(numeric_limits<TRaw>::max));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min, IDENTITY.toUser(numeric_limits<TRaw>::min));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max, IDENTITY.toRaw(numeric_limits<TRaw>::max));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min, IDENTITY.toRaw(numeric_limits<TRaw>::min));

    constexpr conversionFactor<TUser, TRaw> SCALE_ONLY = { 10U, 0};
    TEST_ASSERT_EQUAL(0, SCALE_ONLY.toUser(0));
    TEST_ASSERT_EQUAL((numeric_limits<TRaw>::max/10)*10, SCALE_ONLY.toUser(numeric_limits<TRaw>::max/10));
    TEST_ASSERT_EQUAL((numeric_limits<TRaw>::min/10)*10, SCALE_ONLY.toUser(numeric_limits<TRaw>::min/10));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max/10, SCALE_ONLY.toRaw(SCALE_ONLY.toUser(numeric_limits<TRaw>::max/10)));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min/10, SCALE_ONLY.toRaw(SCALE_ONLY.toUser(numeric_limits<TRaw>::min/10)));
    TEST_ASSERT_EQUAL(RAW_TENTH, SCALE_ONLY.toRaw(SCALE_ONLY.toUser(RAW_TENTH)));

    constexpr conversionFactor<TUser, TRaw> POSITIVE_TRANSLATE = {1, 33};
    TEST_ASSERT_EQUAL(33, POSITIVE_TRANSLATE.toUser(0));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max, POSITIVE_TRANSLATE.toUser(numeric_limits<TRaw>::max-33));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min+66, POSITIVE_TRANSLATE.toUser(numeric_limits<TRaw>::min+33));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max, POSITIVE_TRANSLATE.toRaw(POSITIVE_TRANSLATE.toUser(numeric_limits<TRaw>::max)));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min, POSITIVE_TRANSLATE.toRaw(POSITIVE_TRANSLATE.toUser(numeric_limits<TRaw>::min)));
    TEST_ASSERT_EQUAL(RAW_TENTH, POSITIVE_TRANSLATE.toRaw(POSITIVE_TRANSLATE.toUser(RAW_TENTH)));

    constexpr conversionFactor<TUser, TRaw> NEGATIVE_TRANSLATE = {1, -33};
    TEST_ASSERT_EQUAL(0, NEGATIVE_TRANSLATE.toUser(33));
    TEST_ASSERT_EQUAL(33, NEGATIVE_TRANSLATE.toRaw(0));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max-33, NEGATIVE_TRANSLATE.toUser(numeric_limits<TRaw>::max));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min, NEGATIVE_TRANSLATE.toUser(numeric_limits<TRaw>::min+33));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max, NEGATIVE_TRANSLATE.toRaw(NEGATIVE_TRANSLATE.toUser(numeric_limits<TRaw>::max)));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::min, NEGATIVE_TRANSLATE.toRaw(NEGATIVE_TRANSLATE.toUser(numeric_limits<TRaw>::min)));
    TEST_ASSERT_EQUAL(RAW_TENTH, NEGATIVE_TRANSLATE.toRaw(NEGATIVE_TRANSLATE.toUser(RAW_TENTH)));

    constexpr conversionFactor<TUser, TRaw> SCALE_AND_TRANSLATE = {10, -3};
    TEST_ASSERT_EQUAL(0, SCALE_AND_TRANSLATE.toUser(3));
    TEST_ASSERT_EQUAL((numeric_limits<TRaw>::max/10-3)*10, SCALE_AND_TRANSLATE.toUser(numeric_limits<TRaw>::max/10));
    TEST_ASSERT_EQUAL((numeric_limits<TRaw>::min/10)*10, SCALE_AND_TRANSLATE.toUser((numeric_limits<TRaw>::min/10)+3));
    TEST_ASSERT_EQUAL(numeric_limits<TRaw>::max/10, SCALE_AND_TRANSLATE.toRaw(SCALE_AND_TRANSLATE.toUser(numeric_limits<TRaw>::max/10)));
    TEST_ASSERT_EQUAL((numeric_limits<TRaw>::min/10)+3, SCALE_AND_TRANSLATE.toRaw(SCALE_AND_TRANSLATE.toUser((numeric_limits<TRaw>::min/10)+3)));
    TEST_ASSERT_EQUAL(RAW_TENTH, SCALE_AND_TRANSLATE.toRaw(SCALE_AND_TRANSLATE.toUser(RAW_TENTH)));
}

static void test_U8U8(void)
{
    test_conversionFactor_t<uint8_t, uint8_t>();
}

static void test_S8S8(void)
{
    test_conversionFactor_t<int8_t, int8_t>();
}

static void test_U16U8(void)
{
    test_conversionFactor_t<uint16_t, uint8_t>();
}

static void test_S16U8(void)
{
    test_conversionFactor_t<int16_t, uint8_t>();
}

void testUnitConversions(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_U8U8);
    RUN_TEST(test_S8S8);
    RUN_TEST(test_U16U8);
    RUN_TEST(test_S16U8);
  }
}