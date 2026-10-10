#include "../../test_utils.h"
#include "src/decoders/missingTooth.h"

using namespace decoders::missing_tooth;

static void test_getRevolutionTime(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config4 page4{};    

    // 36-1 crank wheel
    page4.triggerTeeth = 36;
    page4.triggerMissingTeeth = 1;
    page4.TrigSpeed = CRANK_SPEED;

    // Ensure staging allows cranking calculation
    page4.StgCycles = 0;

    // --- Cranking path: current.RPM < crankRPM and not at tooth #1
    current.crankRPM = 400;
    current.setRpm(current.crankRPM/2U);
    current.startRevolutions = 0; // cranking
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    // Choose gap so that revTime = gap * 36 =~ 60000 -> gap ~= 1667
    decoderState.toothLastMinusOneToothTime = 1000UL;
    decoderState.toothLastToothTime = decoderState.toothLastMinusOneToothTime + 1667UL;
    decoderState.toothCurrentCount = 2;
    current.revolutionTime = 99999UL;
    TEST_ASSERT_EQUAL_UINT32(1667UL*36UL, getRevolutionTime(current, decoderState, page4));

    // --- If at tooth #1, cranking path should return current.revolutionTime
    decoderState.toothCurrentCount = 1;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    TEST_ASSERT_EQUAL_UINT32(99999UL, getRevolutionTime(current, decoderState, page4));

    // --- Running path: stdGetRevolutionTime should be used when not cranking
    current.setRpm(current.crankRPM*2U);
    current.startRevolutions = 1; // not cranking
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    current.revolutionTime = 12345UL;
    decoderState.toothOneMinusOneTime = 1000UL;
    decoderState.toothOneTime = decoderState.toothOneMinusOneTime + 60000UL; // revTime = 60000
    TEST_ASSERT_EQUAL_UINT32(60000UL, getRevolutionTime(current, decoderState, page4));

    // --- Fallback: when sync lost, return current.revolutionTime
    decoderState.decoderStatus.syncStatus = SyncStatus::None;
    TEST_ASSERT_EQUAL_UINT32(12345UL, getRevolutionTime(current, decoderState, page4));
}

void testMissingToothRevTime(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_getRevolutionTime);
    }
}