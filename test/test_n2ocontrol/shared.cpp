#include "../test_utils.h"
#include "globals.h"
#include "units.h"
#include "shared.h"

test_context_t setup_n20_tune(uint8_t enableMode)
{
    test_context_t context;

    constexpr uint8_t TEST_N2O1_PIN = 18U;
    constexpr uint8_t TEST_N2O2_PIN = 19U;
    constexpr uint8_t TEST_N2OARM_PIN = 20U;

    context.page10.n2o_arming_pin = TEST_N2OARM_PIN;
    context.page10.n2o_enable = enableMode;
    context.page10.n2o_maxAFR = 100;
    context.page10.n2o_maxMAP = 50;
    context.page10.n2o_minCLT = temperatureAddOffset(77);
    context.page10.n2o_minTPS = 88;
    context.page10.n2o_pin_polarity = LOW;
    
    context.page10.n2o_stage1_adderMin = 1;
    context.page10.n2o_stage1_adderMax = 15;
    context.page10.n2o_stage1_minRPM = RPM_COARSE.toRaw(3000);
    context.page10.n2o_stage1_maxRPM = RPM_COARSE.toRaw(4000);
    context.page10.n2o_stage1_pin = TEST_N2O1_PIN;
    // context.page10.n2o_stage1_retard

    context.page10.n2o_stage2_adderMin = context.page10.n2o_stage1_adderMin+1;
    context.page10.n2o_stage2_adderMax = context.page10.n2o_stage2_adderMin+10;
    context.page10.n2o_stage2_minRPM = context.page10.n2o_stage1_maxRPM+1;
    context.page10.n2o_stage2_maxRPM = context.page10.n2o_stage2_minRPM+10;
    context.page10.n2o_stage2_pin = TEST_N2O2_PIN;

    return context;
}

test_context_t setup_rpm_overlap_tune(uint8_t enableMode)
{
    auto context = setup_n20_tune(enableMode);

    context.page10.n2o_stage2_minRPM = context.page10.n2o_stage1_maxRPM-5;
    context.page10.n2o_stage2_maxRPM = context.page10.n2o_stage2_minRPM+10;

    return context;
}