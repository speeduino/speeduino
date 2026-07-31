#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"
#include "src/controllers/vvt/VvtOutputChannel.h"

extern VvtOutputChannel vvtChannel1;
extern VvtOutputChannel vvtChannel2;

static void test_wmi_enabled(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);

    initialiseAuxPWM();

    TEST_ASSERT_FALSE(currentStatus.wmiTankEmpty);
    TEST_ASSERT_EQUAL(0, currentStatus.wmiPW);
    TEST_ASSERT_EQUAL(0, vvtChannel1.targetDuty);
    TEST_ASSERT_EQUAL(0, vvtChannel2.targetDuty);
}

static void test_wmi_disabled(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    configPage10.wmiEnabled = false; 

    currentStatus.wmiPW = 99;
    initialiseAuxPWM();

    TEST_ASSERT_EQUAL(0, currentStatus.wmiPW);
}


static void test_vvt_enabled(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);

    configPage6.vvtEnabled = true;
    currentStatus.wmiPW = 99;
    initialiseAuxPWM();

    TEST_ASSERT_EQUAL(0, currentStatus.wmiPW);
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