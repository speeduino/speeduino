#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"
#include "scheduler_fuel_controller.h"
#include "src/pins/outputPin.h"

extern inputPin_t wmiTankEmptyPin;
extern outputPin_t wmiIsEnabledPin;

static void setTankEmpty(bool empty)
{
    if (empty) {
        wmiTankEmptyPin._pin.setPinHigh();
    } else {
        wmiTankEmptyPin._pin.setPinLow();
    }
}
static void setup_calc_conditions(test_context_t &context)
{
    setTankEmpty(context.page10.wmiEmptyPolarity);
    context.current.TPS = context.page10.wmiTPS + 1; 
    context.current.setRpm(RPM_COARSE.toUser(context.page10.wmiRPM+1));
    context.current.MAP = (context.page10.wmiMAP + ((context.page10.wmiMAP2-context.page10.wmiMAP)/2U))*2U;
    context.current.IAT = temperatureRemoveOffset(context.page10.wmiIAT+1);
    context.current.rotationStatus = EngineRotationStatus::Running;
}

static void assert_tank_empty(test_context_t &context)
{
    setTankEmpty(!context.page10.wmiEmptyPolarity);
    context.current.wmiTankEmpty = false;
    context.wmiControl();
    TEST_ASSERT_TRUE(context.current.wmiTankEmpty);
}

static void test_tank_empty(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    context.initialise();
    
    assert_tank_empty(context);
    
    // Reverse polarity
    context.page10.wmiEmptyPolarity = !context.page10.wmiEmptyPolarity; 
    assert_tank_empty(context);
}

static void assert_tank_not_empty(test_context_t &context)
{
    setTankEmpty(context.page10.wmiEmptyPolarity);
    context.current.wmiTankEmpty = true;
    context.wmiControl();
    TEST_ASSERT_FALSE(context.current.wmiTankEmpty);
}

static void test_tank_not_empty(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    context.initialise();

    assert_tank_not_empty(context);

    // Reverse polarity
    context.page10.wmiEmptyPolarity = !context.page10.wmiEmptyPolarity; 
    assert_tank_not_empty(context);

    // Disabled 
    context.page10.wmiEmptyEnabled = false;
    assert_tank_not_empty(context);
}

static void assert_wmipw(test_context_t &context, uint8_t expected)
{
    context.current.wmiPW = 99; 
    context.wmiControl();
    TEST_ASSERT_EQUAL(expected, context.current.wmiPW);
    if (expected==0)
    {
        TEST_ASSERT_TRUE(wmiIsEnabledPin._pin.isPinLow());
    }
    else
    {
        TEST_ASSERT_TRUE(wmiIsEnabledPin._pin.isPinHigh());
    }
}

static void setup_assert_wmipw(test_context_t &context, uint8_t expected)
{
    setup_calc_conditions(context);
    assert_wmipw(context, expected);
}

static void test_calc_conditions(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    context.initialise();

    // Assert initial conditions
    setup_calc_conditions(context);
    assert_wmipw(context, 200);

    // TPS
    setup_calc_conditions(context);
    context.current.TPS = context.page10.wmiTPS; 
    assert_wmipw(context, 200);

    setup_calc_conditions(context);
    context.current.TPS = context.page10.wmiTPS - 1; 
    assert_wmipw(context, 0);
    
    // RPM
    setup_calc_conditions(context);
    context.current.setRpm(RPM_COARSE.toUser(context.page10.wmiRPM));
    assert_wmipw(context, 200);
    
    setup_calc_conditions(context);
    context.current.setRpm(RPM_COARSE.toUser(context.page10.wmiRPM-1));
    assert_wmipw(context, 0);
    
    // MAP
    setup_calc_conditions(context);
    context.current.MAP = context.page10.wmiMAP*2U;
    assert_wmipw(context, 200);
    
    setup_calc_conditions(context);
    context.current.MAP = (context.page10.wmiMAP*2U)-1;
    assert_wmipw(context, 0);
    
    // IAT
    setup_calc_conditions(context);
    context.current.IAT = temperatureRemoveOffset(context.page10.wmiIAT);
    assert_wmipw(context, 200);
    
    setup_calc_conditions(context);
    context.current.IAT = temperatureRemoveOffset(context.page10.wmiIAT)-1U;
    assert_wmipw(context, 0);

    // Engine rotating
    setup_calc_conditions(context);
    context.current.rotationStatus = EngineRotationStatus::Cranking;
    assert_wmipw(context, 0);

    setup_calc_conditions(context);
    context.current.rotationStatus = EngineRotationStatus::Stopped;
    assert_wmipw(context, 0);
}

static void test_mode_simple(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    context.initialise();

    setup_assert_wmipw(context, 200);
}

static void test_mode_proportional(void)
{
    auto context = setup_wmi_tune(WMI_MODE_PROPORTIONAL);
    context.initialise();

    setup_assert_wmipw(context, 100);

    // Clamp <=200
    context.page10.wmiMAP2 = context.page10.wmiMAP+10;
    assert_wmipw(context, 200);
}

static void test_mode_ol(void)
{
    auto context = setup_wmi_tune(WMI_MODE_OPENLOOP);
    context.initialise();

    fill_table_values(wmiTable, 33);
    populate_table_axis(wmiTable.axisX, (table3d_axis_t)10);
    populate_table_axis(wmiTable.axisY, (table3d_axis_t)10);

    setup_assert_wmipw(context, wmiTable.values[0]);

    // Clamp <=200
    fill_table_values(wmiTable, 255);
    context.page10.wmiMAP2 = context.page10.wmiMAP+10;
    assert_wmipw(context, 200);
}

static void test_mode_cl(void)
{
    auto context = setup_wmi_tune(WMI_MODE_CLOSEDLOOP);
    context.initialise();

    populate_table_axis(wmiTable.axisX, (table3d_axis_t)10);
    populate_table_axis(wmiTable.axisY, (table3d_axis_t)10);
    fill_table_values(wmiTable, 177);
    fuelSchedule1.pw = abs(context.page10.wmiOffset)*2;

    setup_assert_wmipw(context, 29);

    // Test 0-200 clamp
    fill_table_values(wmiTable, 1);
    fuelSchedule1.pw = 1;
    assert_wmipw(context, 0);

    fill_table_values(wmiTable, 255);
    fuelSchedule1.pw = 255;
    assert_wmipw(context, 200);
}

static void test_mode_other(void)
{
    auto context = setup_wmi_tune(WMI_MODE_CLOSEDLOOP*2U);
    context.initialise();

    setup_assert_wmipw(context, 0);
}

static void test_disabled(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    context.page10.wmiEnabled = false;    
    context.initialise();

    setup_assert_wmipw(context, 0);
}

static void test_vvt2_enabled(void)
{
    auto context = setup_wmi_tune(WMI_MODE_SIMPLE);
    context.page10.vvt2Enabled = true;
    context.initialise();
    TEST_ASSERT_FALSE(context.page10.vvt2Enabled);

    setup_assert_wmipw(context, 200);
}

void testWmiControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_tank_empty);
    RUN_TEST_P(test_tank_not_empty);
    RUN_TEST_P(test_mode_simple);
    RUN_TEST_P(test_mode_proportional);
    RUN_TEST_P(test_mode_ol);
    RUN_TEST_P(test_mode_cl);
    RUN_TEST_P(test_mode_other);
    RUN_TEST_P(test_disabled);
    RUN_TEST_P(test_vvt2_enabled);
    RUN_TEST_P(test_calc_conditions);
  }
}