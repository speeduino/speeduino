#include "../../test_utils.h"
#include "src/decoders/missingTooth.h"
#include "scheduler_ignition_controller.h"

namespace decoders {
namespace missing_tooth {
extern void applyPerToothIgnition(const statuses &current, detail::state_t &decoderState, const config2 &page2, const config4 &page4);
}
}

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

static config4 test_setup_60_2(void)
{
    config4 page4;

    //Setup a 60-2 wheel
    page4.triggerTeeth = 60;
    page4.triggerMissingTeeth = 2;
    page4.TrigSpeed = CRANK_SPEED;
    page4.trigPatternSec = SEC_TRIGGER_SINGLE;

    return page4;
}

static void assert_setEndTeeth(uint8_t expected, decoders::detail::state_t &decoderState, const config4 &page4, IgnitionSchedule &schedule, uint8_t index, int8_t advance)
{
    schedule.dischargeAngle = 360 + advance; 
    decoders::missing_tooth::setEndTeeth(decoderState, page4);
    TEST_ASSERT_EQUAL(expected, decoderState.ignitionEndTeeth[index]);
}

static void test_missingtooth_newIgn_36_1()
{
    auto page4 = test_setup_36_1();
    page4.sparkMode = IGN_MODE_WASTED;
    auto decoderState = decoders::missing_tooth::intialise(page4);
    
    page4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(34, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(35, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(35, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = 90;
    assert_setEndTeeth(25, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(26, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(27, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = 180;
    assert_setEndTeeth(16, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(17, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(18, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = 270;
    assert_setEndTeeth(7, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(8, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(9, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = 360;
    assert_setEndTeeth(34, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(35, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(35, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = -90;
    assert_setEndTeeth(7, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(8, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(9, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = -180;
    assert_setEndTeeth(16, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(17, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(18, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = -270;
    assert_setEndTeeth(25, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(26, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(27, decoderState, page4, ignitionSchedule1, 0, 10);
    
    page4.triggerAngle = -360;
    assert_setEndTeeth(34, decoderState, page4, ignitionSchedule1, 0, -10);
    assert_setEndTeeth(35, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(35, decoderState, page4, ignitionSchedule1, 0, 10);
}

static void test_missingtooth_newIgn_60_2()
{
    auto page4 = test_setup_60_2();
    page4.sparkMode = IGN_MODE_WASTED;
    auto decoderState = decoders::missing_tooth::intialise(page4);
    
    page4.triggerAngle = 0; //No trigger offset
    assert_setEndTeeth(57, decoderState, page4, ignitionSchedule1, 0, -7);
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = 90;
    assert_setEndTeeth(43, decoderState, page4, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(44, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(45, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = 180;
    assert_setEndTeeth(28, decoderState, page4, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(29, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(30, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = 270;
    assert_setEndTeeth(13, decoderState, page4, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(14, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(15, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = 360;
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, -7);
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = -90;
    assert_setEndTeeth(13, decoderState, page4, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(14, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(15, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = -180;
    assert_setEndTeeth(28, decoderState, page4, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(29, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(30, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = -270;
    assert_setEndTeeth(43, decoderState, page4, ignitionSchedule1, 0, -6);
    assert_setEndTeeth(44, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(45, decoderState, page4, ignitionSchedule1, 0, 6);
    
    page4.triggerAngle = -360;
    assert_setEndTeeth(57, decoderState, page4, ignitionSchedule1, 0, -7);
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, 0);
    assert_setEndTeeth(58, decoderState, page4, ignitionSchedule1, 0, 6);
}

static void test_primary_per_tooth_ignition_adjusts_timing(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Running;
    current.startRevolutions = 2000;
    current.setRpm(3000);
    page2.perToothIgn = true;
    page2.strokes = FOUR_STROKE;
    page4.triggerAngle = 10;
    page4.triggerTeeth = 36;
    page4.sparkMode = IGN_MODE_WASTED;
    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 2;
    decoderState.triggerActualTeeth = 36;
    decoderState.triggerToothAngle = 10;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.ignitionEndTeeth[0] = 3;
 
    for (auto ignMode: { IGN_MODE_WASTED, IGN_MODE_SINGLE, IGN_MODE_WASTEDCOP, IGN_MODE_SEQUENTIAL, IGN_MODE_ROTARY})
    {
        page4.sparkMode = ignMode;
        for (auto rev: { true, false })
        {
            decoderState.revolutionOne = rev;
            for (auto trigSpeed: { CRANK_SPEED, CAM_SPEED})
            {
                page4.TrigSpeed = trigSpeed;
                decoderState.toothCurrentCount = 2;
                decoderState.ignitionEndTeeth[0] = decoderState.toothCurrentCount;
                if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (decoderState.revolutionOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
                {
                    decoderState.ignitionEndTeeth[0] += page4.triggerTeeth;
                }
                ignitionSchedule1._status = PENDING;
                ignitionSchedule1._compare = 100;
                ignitionSchedule1.chargeAngle = 9999;

                decoders::missing_tooth::applyPerToothIgnition(current, decoderState, page2, page4);
                TEST_ASSERT_NOT_EQUAL(100, ignitionSchedule1._compare);
           }
        }
    }
}

static void test_primary_skips_per_tooth_ignition_while_cranking(void)
{
    statuses current;
    decoders::detail::state_t decoderState{};
    config2 page2{};
    config4 page4{};

    setAngleConverterRevolutionTime(6000000UL);
    current.rotationStatus = EngineRotationStatus::Cranking;
    current.startRevolutions = 2000;
    current.setRpm(500);
    page2.perToothIgn = true;
    page4.triggerAngle = 10;
    decoderState.toothLastMinusOneToothTime = 800;
    decoderState.toothLastToothTime = 900;
    decoderState.toothCurrentCount = 2;
    decoderState.triggerActualTeeth = 36;
    decoderState.decoderStatus.syncStatus = SyncStatus::Full;
    decoderState.ignitionEndTeeth[0] = 3;
    ignitionSchedule1._status = PENDING;
    ignitionSchedule1._counter = 100;
    ignitionSchedule1._compare = 100;
    ignitionSchedule1.chargeAngle = 500;

    decoders::missing_tooth::triggerPrimary(1000, current, decoderState, page2, page4);

    TEST_ASSERT_EQUAL(100, ignitionSchedule1._compare);
}

void testMissingToothPerToothIgn(void)
{
    unity_filename_guard_t guard(__FILE__);

    RUN_TEST_P(test_missingtooth_newIgn_36_1);
    RUN_TEST_P(test_missingtooth_newIgn_60_2);
    RUN_TEST_P(test_primary_per_tooth_ignition_adjusts_timing);
    RUN_TEST_P(test_primary_skips_per_tooth_ignition_while_cranking);
}