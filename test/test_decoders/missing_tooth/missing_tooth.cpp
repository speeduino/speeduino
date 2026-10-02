#include <decoders.h>
#include <globals.h>
#include "../../test_utils.h"
#include "src/decoders/decoder_state.h"

static decoder_t test_setup_36_1()
{
    //Setup a 36-1 wheel
    configPage4.triggerTeeth = 36;
    configPage4.triggerMissingTeeth = 1;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return triggerSetup_missingTooth();
}

extern decoders::detail::state_t _decoderState;

static void test_getCrankAngle(void)
{
    decoder_t decoder = test_setup_36_1();

    auto run_case = [&](int toothCount, bool revOne, int delta, int trigAngle, int16_t expected) {
        _decoderState.toothLastToothTime = 2000;
        _decoderState.toothCurrentCount = toothCount;
        _decoderState.revolutionOne = revOne;
        _decoderState.decoderStatus.syncStatus = SyncStatus::Full;
        _decoderState.decoderStatus.toothAngleIsCorrect = true;
        configPage4.triggerAngle = trigAngle;
        CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
        setAngleConverterRevolutionTime(2000);
        TEST_ASSERT_EQUAL(expected, decoder.pGetCrankAngle(_decoderState.toothLastToothTime + delta));
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
    configPage4.TrigSpeed = CAM_SPEED;
    run_case(1, true, 100, 0, 0 + dt);
    configPage4.TrigSpeed = CRANK_SPEED;
    run_case(1, true, 100, 0, 360 + 0 + dt);
}


void testMissingTooth()
{
    extern void testMissingToothRevTime(void);
    extern void testMissingToothTriggers(void);
    extern void testMissingToothPerToothIgn(void);

    testMissingToothRevTime();
    testMissingToothTriggers();
    testMissingToothPerToothIgn();

    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_getCrankAngle);
    }
}