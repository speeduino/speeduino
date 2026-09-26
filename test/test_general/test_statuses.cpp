#include "../test_utils.h"
#include "statuses.h"
#include "config_pages.h"
#include "crankMaths.h"

static decoder_features_t fakeDecoderFeatures = {};
static decoder_features_t fakeGetFeatures(void) noexcept
{
    return fakeDecoderFeatures;
}

static void test_isFixedCrankingIgnitionTimingActive(void)
{
    statuses subject = {};
    config4 page4 = {};

    subject.decoder.getFeatures = fakeGetFeatures;
    fakeDecoderFeatures.hasFixedCrankingTiming = false;

    page4.ignCranklock = false;
    TEST_ASSERT_FALSE(subject.isFixedCrankingIgnitionTimingActive(page4));
    
    page4.ignCranklock = true;
    
    subject.rotationStatus = EngineRotationStatus::Stopped;
    TEST_ASSERT_FALSE(subject.isFixedCrankingIgnitionTimingActive(page4));
    
    subject.rotationStatus = EngineRotationStatus::Running;
    TEST_ASSERT_FALSE(subject.isFixedCrankingIgnitionTimingActive(page4));
    
    subject.rotationStatus = EngineRotationStatus::Cranking;
    TEST_ASSERT_FALSE(subject.isFixedCrankingIgnitionTimingActive(page4));

    fakeDecoderFeatures.hasFixedCrankingTiming = true;
    TEST_ASSERT_TRUE(subject.isFixedCrankingIgnitionTimingActive(page4));
}

static void test_setRevolutionTime_derives_rpm(void)
{
    statuses subject = {};

    subject.setRevolutionTime(60000UL); // 1000 RPM
    TEST_ASSERT_EQUAL_UINT32(60000UL, subject.revolutionTime);
    TEST_ASSERT_EQUAL_UINT16(1000U, subject.RPM);
    TEST_ASSERT_EQUAL_UINT8(10U, subject.RPMdiv100);

    // RPM is rounded to the closest integer: 60000000/9000 = 6666.67
    subject.setRevolutionTime(9000UL);
    TEST_ASSERT_EQUAL_UINT32(9000UL, subject.revolutionTime);
    TEST_ASSERT_EQUAL_UINT16(6667U, subject.RPM);
    TEST_ASSERT_EQUAL_UINT8(67U, subject.RPMdiv100);
}

static void test_setRevolutionTime_clamps_rpm(void)
{
    statuses subject = {};

    subject.setRevolutionTime(MIN_REVOLUTION_TIME/2U);
    TEST_ASSERT_EQUAL_UINT32(MIN_REVOLUTION_TIME/2U, subject.revolutionTime);
    TEST_ASSERT_EQUAL_UINT16(MAX_RPM, subject.RPM);
}

static void test_setRevolutionTime_updates_angle_converter(void)
{
    statuses subject = {};

    subject.setRevolutionTime(60000UL);
    TEST_ASSERT_UINT32_WITHIN(1U, 60000UL, angleToTime(360U));

    subject.setRevolutionTime(30000UL);
    TEST_ASSERT_UINT32_WITHIN(1U, 30000UL, angleToTime(360U));
}

static void test_setRevolutionTime_zero(void)
{
    statuses subject = {};

    subject.setRevolutionTime(60000UL);
    subject.setRevolutionTime(0UL);
    TEST_ASSERT_EQUAL_UINT32(0UL, subject.revolutionTime);
    TEST_ASSERT_EQUAL_UINT16(0U, subject.RPM);
    TEST_ASSERT_EQUAL_UINT8(0U, subject.RPMdiv100);
    // The last known angle conversion is retained
    TEST_ASSERT_UINT32_WITHIN(1U, 60000UL, angleToTime(360U));
}

void testStatuses(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_setRevolutionTime_derives_rpm);
        RUN_TEST_P(test_setRevolutionTime_clamps_rpm);
        RUN_TEST_P(test_setRevolutionTime_updates_angle_converter);
        RUN_TEST_P(test_setRevolutionTime_zero);
    }
}
