#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "dual_wheel.h"
#include "../shared/missing_tooth_and_dual_wheel.h"

// This runs the shared test invoking with the proper setup.
// see: shared_test_variable_missing_tooth_patterns to identify the configuration that this test is running under.
void test_dualWheelBatch()
{
    test_shared_variableMissingToothPatterns(triggerSetEndTeeth_DualWheel, true);
}

//************************************** Begin the new ignition setEndTooth tests **************************************
void test_dualwheel_newIgn_12_1_trig0_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 12/1
    //Advance: 10
    //triggerAngle=0
    test_shared_triggerSetup(12, 1, IGN_MODE_WASTED, 1);

    // Override some global variables that the shared setup creates.
    ignition1EndAngle = 360 - 10; 
    configPage4.triggerAngle = 0; 
    triggerSetEndTeeth_DualWheel();
    TEST_ASSERT_EQUAL(11, ignition1EndTooth);

    //Test again with 0 degrees advance
    ignition1EndAngle = 360 - 0; //Set 0 degrees advance
    triggerSetEndTeeth_DualWheel();
    TEST_ASSERT_EQUAL(12, ignition1EndTooth);

    //Test again with 35 degrees advance
    ignition1EndAngle = 360 - 35; //Set 35 degrees advance
    triggerSetEndTeeth_DualWheel();
    TEST_ASSERT_EQUAL(10, ignition1EndTooth);
}

void test_dualWheel()
{
    RUN_TEST(test_dualwheel_newIgn_12_1_trig0_1);
    RUN_TEST(test_dualWheelBatch);
}