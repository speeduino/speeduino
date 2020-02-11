#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "missing_tooth.h"


void test_setup_36_1()
{
    //Setup a 36-1 wheel
    configPage4.triggerTeeth = 36;
    configPage4.triggerMissingTeeth = 1;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    triggerSetup_missingTooth();
}

void test_missingtooth_newIgn_1()
{

    test_setup_36_1();
    configPage4.sparkMode = IGN_MODE_WASTED;
    ignition1EndAngle = 350; //Set 10 degrees advance
    configPage4.triggerAngle = 0; //No trigger offset
    
    triggerSetEndTeeth_missingTooth();
    TEST_ASSERT_EQUAL(ignition1EndTooth, 34);

}
void test_missingtooth_newIgn_2()
{

}
void test_missingtooth_newIgn_3()
{

}
void test_missingtooth_newIgn_4()
{

}