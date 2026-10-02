#include "../../test_utils.h"
#include "src/decoders/missingTooth.h"

static config4 test_setup_36_1(void)
{
    config4 page4;

    //Setup a 36-1 wheel
    page4.triggerTeeth = 36;
    page4.triggerMissingTeeth = 1;
    page4.TrigSpeed = CRANK_SPEED;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return page4;
}

static void test_getCrankAngle(void)
{
    auto page4 = test_setup_36_1();
    auto decoderState = decoders::missing_tooth::intialise(page4);

    auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
        decoderState.toothLastToothTime = 2000;
        decoderState.toothCurrentCount = toothCount;
        decoderState.revolutionOne = revOne;
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        decoderState.decoderStatus.toothAngleIsCorrect = true;
        page4.triggerAngle = trigAngle;
        CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
        setAngleConverterRevolutionTime(2000);
        TEST_ASSERT_EQUAL(expected, decoders::missing_tooth::getCrankAngle(decoderState.toothLastToothTime + delta, decoderState, page4));
    };

    // For 36-1 wheel: triggerToothAngle = 10 degrees. timeToAngle(100) ~= 18 deg
    const int dt = 18;

    // Basic teeth
    run_case(1, false, 100, 0, 0 + dt);
    run_case(2, false, 100, 0, 10 + dt);
    run_case(10, false, 100, 0, 90 + dt);
    run_case(35, false, 100, 0, 340 + dt);

    // Trigger angle offset
    run_case(1, false, 100, 90, 90 + dt);

    // Revolution one true should add 360 degrees
    page4.TrigSpeed = CAM_SPEED;
    run_case(1, true, 100, 0, 0 + dt);
    page4.TrigSpeed = CRANK_SPEED;
    run_case(1, true, 100, 0, 360 + 0 + dt);
}

void testMissingToothCrankAngle(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_getCrankAngle);
}