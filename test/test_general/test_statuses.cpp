#include "../test_utils.h"
#include "statuses.h"
#include "config_pages.h"

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

void testStatuses(void) 
{
    SET_UNITY_FILENAME() {
    }
}
