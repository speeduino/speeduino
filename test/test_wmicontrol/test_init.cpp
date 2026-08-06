#include "../test_utils.h"
#include "globals.h"
#include "src/controllers/vvt/vvtController.h"
#include "units.h"
#include "shared.h"
#include "src/pwm/PwmOutputChannel.h"

extern PwmOutputChannel vvtChannel1;
extern PwmOutputChannel vvtChannel2;

static void test_wmi_enabled(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);

    context.initialise();

    TEST_ASSERT_FALSE(context.current.wmiTankEmpty);
    TEST_ASSERT_EQUAL(0, context.current.wmiPW);
    TEST_ASSERT_EQUAL(0, vvtChannel1.targetDuty);
    TEST_ASSERT_EQUAL(0, vvtChannel2.targetDuty);
}

static void test_wmi_disabled(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    configPage10.wmiEnabled = false; 

    context.current.wmiPW = 99;
    context.initialise();

    TEST_ASSERT_EQUAL(0, context.current.wmiPW);
}


static void test_vvt_enabled(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);

    configPage6.vvtEnabled = true;
    context.current.wmiPW = 99;
    context.initialise();

    TEST_ASSERT_EQUAL(0, context.current.wmiPW);
}

void testInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_wmi_enabled);
    RUN_TEST_P(test_wmi_disabled);
    RUN_TEST_P(test_vvt_enabled);
  }
}