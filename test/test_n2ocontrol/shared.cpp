#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"

void setup_n20_tune(uint8_t enableMode)
{
    configPage10.n2o_arming_pin = TEST_N2OARM_PIN;
    configPage10.n2o_enable = enableMode;
    configPage10.n2o_maxAFR = 100;
    configPage10.n2o_maxMAP = 50;
    configPage10.n2o_minCLT = temperatureAddOffset(77);
    configPage10.n2o_minTPS = 88;
    configPage10.n2o_pin_polarity = LOW;
    
    configPage10.n2o_stage1_adderMin = 1;
    configPage10.n2o_stage1_adderMax = 15;
    configPage10.n2o_stage1_minRPM = RPM_COARSE.toRaw(3000);
    configPage10.n2o_stage1_maxRPM = RPM_COARSE.toRaw(4000);
    configPage10.n2o_stage1_pin = TEST_N2O1_PIN;
    // configPage10.n2o_stage1_retard

    configPage10.n2o_stage2_adderMin = configPage10.n2o_stage1_adderMin+1;
    configPage10.n2o_stage2_adderMax = configPage10.n2o_stage2_adderMin+10;
    configPage10.n2o_stage2_minRPM = configPage10.n2o_stage1_maxRPM+1;
    configPage10.n2o_stage2_maxRPM = configPage10.n2o_stage2_minRPM+10;
    configPage10.n2o_stage2_pin = TEST_N2O2_PIN;
}

void setup_rpm_overlap_tune(uint8_t enableMode)
{
    setup_n20_tune(enableMode);
    configPage10.n2o_stage2_minRPM = configPage10.n2o_stage1_maxRPM-5;
    configPage10.n2o_stage2_maxRPM = configPage10.n2o_stage2_minRPM+10;
}