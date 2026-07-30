#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"
#include "src/controllers/vvt/VvtOutputChannel.h"

extern VvtOutputChannel vvtChannel1;
extern VvtOutputChannel vvtChannel2;
extern uint16_t vvt_pwm_max_count;
extern bool vvtTimeHold;
extern uint8_t vvtCounter;
extern bool vvtIsHot;
extern uint32_t vvtWarmTime;

static void reset_init_postconditions(void)
{
    vvt_pwm_max_count = 0;
    vvtChannel1.pin.setPinLow();
    vvtChannel2.pin.setPinLow();
    currentStatus.wmiTankEmpty = true;
    currentStatus.wmiPW = 99;
    currentStatus.vvt1.duty = 99;
    currentStatus.vvt2.duty = 99;
    currentStatus.vvt1.angle = 99;
    currentStatus.vvt2.angle = 99;
    currentStatus.vvt1.angleError = true;
    currentStatus.vvt2.angleError = true;
    vvtTimeHold = true;
    vvtCounter = 99;
    vvtWarmTime = 99;
}

static void assert_init_postconditions(void)
{
    TEST_ASSERT_NOT_EQUAL(0, vvt_pwm_max_count);
    TEST_ASSERT_TRUE(vvtChannel1.pin.isValid());
    TEST_ASSERT_FALSE(vvtChannel1.pin.isPinHigh());
    TEST_ASSERT_TRUE(vvtChannel2.pin.isValid());
    TEST_ASSERT_FALSE(vvtChannel2.pin.isPinHigh());
    TEST_ASSERT_FALSE(currentStatus.wmiTankEmpty);
    TEST_ASSERT_EQUAL(0, currentStatus.wmiPW);
    TEST_ASSERT_EQUAL(0, currentStatus.vvt1.duty);
    TEST_ASSERT_EQUAL(0, currentStatus.vvt2.duty);
    TEST_ASSERT_EQUAL(0, currentStatus.vvt1.angle);
    TEST_ASSERT_EQUAL(0, currentStatus.vvt2.angle);
    TEST_ASSERT_FALSE(currentStatus.vvt1.angleError);
    TEST_ASSERT_FALSE(currentStatus.vvt2.angleError);
    TEST_ASSERT_FALSE(vvtTimeHold);
    TEST_ASSERT_EQUAL(0, vvtCounter);
    TEST_ASSERT_EQUAL(0, vvtWarmTime);
}

static void test_init(void)
{
    // For completeness, we need to run all tests with and without WMI enabled
    for (bool wmi : (bool[2]){ false, true }) 
    {
        // For completeness, we need to run all tests with and without VVT2 enabled
        for (bool vvt2 : (bool[2]){ false, true }) 
        {
            // For completeness, we need to run all tests against TPS and MAP
            for (uint8_t loadSource = VVT_LOAD_MAP; loadSource<=VVT_LOAD_TPS; ++loadSource)
            {
                setup_vvt_onoff_tune(loadSource, vvt2, wmi);
                reset_init_postconditions();
                initialiseAuxPWM();
                assert_init_postconditions();

                setup_vvt_onoff_tune(loadSource, vvt2, wmi);
                configPage6.vvtEnabled = false;
                reset_init_postconditions();
                initialiseAuxPWM();
                assert_init_postconditions();
            }
        }
    }
}

static void test_vvtIsHot(void)
{
    setup_vvt_onoff_tune(VVT_LOAD_MAP, false, false);

    currentStatus.coolant = temperatureRemoveOffset(configPage4.vvtMinClt)-1;
    vvtIsHot = true;
    initialiseAuxPWM();
    TEST_ASSERT_FALSE(vvtIsHot);

    currentStatus.coolant = temperatureRemoveOffset(configPage4.vvtMinClt);
    vvtIsHot = false;
    initialiseAuxPWM();
    TEST_ASSERT_TRUE(vvtIsHot);

    currentStatus.coolant = temperatureRemoveOffset(configPage4.vvtMinClt)+1;
    vvtIsHot = false;
    initialiseAuxPWM();
    TEST_ASSERT_TRUE(vvtIsHot);
}

void testInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_init);
    RUN_TEST_P(test_vvtIsHot);
  }
}