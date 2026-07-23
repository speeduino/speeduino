#include "../test_utils.h"
#include "shared.h"
#include "globals.h"

void setup_simplepid_tune(void)
{
    pinNumbers.pinBoost = TEST_BOOST_PIN;

    configPage6.boostEnabled = true;
    configPage6.boostMode = BOOST_MODE_SIMPLE;
    configPage2.boostMinDuty = 0; 
    configPage2.boostMaxDuty = 255;
    configPage10.boostIntv = 0;
    configPage10.boostSens = 1;
}

void setup_fullpid_tune(void)
{
    setup_simplepid_tune();
    configPage6.boostMode = BOOST_MODE_FULL;
    configPage6.boostKP = 5;
    configPage6.boostKI = 3;
    configPage6.boostKD = 1;
}