#include "../../test_utils.h"
#include "src/decoders/missingTooth.h"

using namespace decoders::missing_tooth;

static void test_secTriggerPattern(void)
{
    config4 page4{};    
    page4.triggerTeeth = 23;

    page4.trigPatternSec = SEC_TRIGGER_4_1;
    {
        auto subject = initialise(page4);
        TEST_ASSERT_EQUAL(416, subject.triggerSecFilterTime);
    }

    page4.trigPatternSec = SEC_TRIGGER_5_3_2;
    {
        auto subject = initialise(page4);
        TEST_ASSERT_EQUAL(3333, subject.triggerSecFilterTime);
    }
}

void testMissingToothInitialise(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_secTriggerPattern);
}