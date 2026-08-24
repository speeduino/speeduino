#include "../test_utils.h"

#include "shared.h"
#include "units.h"

void assert_ac_off(const test_context &context)
{
  SET_UNITY_FILENAME()
  {
    TEST_ASSERT_TRUE(context.page15.airConCompPol==airConState.compPin._pin.isPinHigh());
    TEST_ASSERT_TRUE(!airConState.standAloneFanIsEnabled || context.page15.airConFanPol==airConState.fanPin._pin.isPinHigh());
    TEST_ASSERT_FALSE(context.current.airconCompressorOn); 
    TEST_ASSERT_FALSE(context.current.airconFanOn);
  }
}

constexpr uint8_t TEST_ACREQUEST_PIN = 11;
constexpr uint8_t TEST_ACCOMP_PIN = 12;

test_context setup_ac_tune(void)
{
    test_context context;
    context.pins.pinAirConComp = TEST_ACCOMP_PIN;
    context.pins.pinAirConRequest = TEST_ACREQUEST_PIN;

    context.page15.airConEnable = true;
    context.page15.airConCompPol = false;
    context.page15.airConFanEnabled = false; // See issue #1544
    context.page15.airConAfterStartDelay = 17;
    context.page15.airConClTempCut = TEMPERATURE.toRaw(100);
    context.page15.airConTPSCut = 75;
    context.page15.airConTPSCutTime = 13;
    context.page15.airConMinRPMdiv10 = RPM_MEDIUM.toRaw(500);
    context.page15.airConMaxRPMdiv100 = RPM_COARSE.toRaw(3000);
    context.page15.airConRPMCutTime = 11;
    context.page15.airConCompOnDelay = 7;

    return context;
}
