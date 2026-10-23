#include "../../test_utils.h"
#include "src/decoders/dualWheel.h"
#include "scheduler_ignition_controller.h"

using namespace decoders::shared;

static config4 setup_dualwheel_12_1(void)
{
    config4 page4 = {};

    //Setup a 12-1 wheel
    page4.triggerTeeth = 12;
    page4.TrigSpeed = CRANK_SPEED;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return page4;
}

static void test_getCrankAngle(void)
{
    auto page4 = setup_dualwheel_12_1();
    auto decoderState = decoders::dual_wheel::initialise(page4);

    auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
        decoderState.toothLastToothTime = 2000;
        decoderState.toothCurrentCount = toothCount;
        decoderState.revolutionOne = revOne;
        decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        decoderState.decoderStatus.toothAngleIsCorrect = true;
        page4.triggerAngle = trigAngle;
        CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
        setAngleConverterRevolutionTime(2000);
        TEST_ASSERT_EQUAL(expected, decoders::dual_wheel::getCrankAngle(decoderState.toothLastToothTime + delta, decoderState, page4));
    };

    // For a 12-tooth wheel triggerToothAngle = 30 degrees. timeToAngle(100) ~= 18 deg
    const int dt = 18;

    // Basic teeth 1..12 (no trigger offset, not on second revolution)
    run_case(1, false, 100, 0, 0 + dt);
    run_case(2, false, 100, 0, 30 + dt);
    run_case(3, false, 100, 0, 60 + dt);
    run_case(4, false, 100, 0, 90 + dt);
    run_case(5, false, 100, 0, 120 + dt);
    run_case(6, false, 100, 0, 150 + dt);
    run_case(7, false, 100, 0, 180 + dt);
    run_case(8, false, 100, 0, 210 + dt);
    run_case(9, false, 100, 0, 240 + dt);
    run_case(10, false, 100, 0, 270 + dt);
    run_case(11, false, 100, 0, 300 + dt);
    run_case(12, false, 100, 0, 330 + dt);

    // Secondary-last-tooth path: decoderState.toothCurrentCount == 0 treated as 12
    run_case(0, false, 100, 0, 330 + dt);

    // Trigger angle offset
    run_case(3, false, 100, 10, 60 + 10 + dt);

    // Revolution one true should add 360 degrees
    page4.TrigSpeed = CAM_SPEED;
    run_case(1, true, 100, 0, 0 + dt);
    page4.TrigSpeed = CRANK_SPEED;
    run_case(1, true, 100, 0, 360 + 0 + dt);
}

void testDualWheelCrankAngle(void)
{
   unity_filename_guard_t guard(__FILE__);
 
   RUN_TEST_P(test_getCrankAngle);
}