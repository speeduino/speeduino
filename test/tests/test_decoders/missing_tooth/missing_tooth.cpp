#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "missing_tooth.h"
#include "../shared/missing_tooth_and_dual_wheel.h"


// This runs the shared test invoking with the proper setup.
// see: shared_test_variable_missing_tooth_patterns to identify the configuration that this test is running under.
void test_missingToothBatch()
{
    test_shared_variableMissingToothPatterns(triggerSetEndTeeth_missingTooth, false);
}

//************************************** Begin the old ignition setEndTooth tests shorter **************************************
void test_missingtooth_newIgn_36_1_trig0_1()
{
    //Test the set end tooth function. Conditions:
    //Trigger: 36-1
    //Advance: 10
    //triggerAngle=0
    test_shared_triggerSetup(36, 1, IGN_MODE_WASTED, 4);
    
    triggerSetEndTeeth_missingTooth();
    TEST_ASSERT_EQUAL(34, ignition1EndTooth);
}


void test_missingTooth()
{
  RUN_TEST(test_missingtooth_newIgn_36_1_trig0_1);
  RUN_TEST(test_missingToothBatch);
}