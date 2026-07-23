#include <unity.h>
#include "shared.h"
#include "globals.h"
#include "units.h"

void assert_ac_off(void)
{
    TEST_ASSERT_TRUE(configPage15.airConCompPol==aircon_comp_pin._pin.isPinHigh());
    TEST_ASSERT_TRUE(configPage15.airConFanPol==aircon_fan_pin._pin.isPinHigh());
    TEST_ASSERT_FALSE(currentStatus.airconCompressorOn); 
    TEST_ASSERT_FALSE(currentStatus.airconFanOn);
}

void setup_ac_tune(void)
{
    pinNumbers.pinAirConComp = TEST_ACCOMP_PIN;
    pinNumbers.pinAirConRequest = TEST_ACREQUEST_PIN;

    configPage15.airConEnable = true;
    configPage15.airConCompPol = false;
    configPage15.airConFanEnabled = false; // See issue #1544
    configPage15.airConAfterStartDelay = 17;
    configPage15.airConClTempCut = TEMPERATURE.toRaw(100);
    configPage15.airConTPSCut = 75;
    configPage15.airConTPSCutTime = 13;
    configPage15.airConMinRPMdiv10 = RPM_MEDIUM.toRaw(500);
    configPage15.airConMaxRPMdiv100 = RPM_COARSE.toRaw(3000);
    configPage15.airConRPMCutTime = 11;
    configPage15.airConCompOnDelay = 7;
}
