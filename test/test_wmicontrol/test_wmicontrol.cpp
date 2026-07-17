#include "../test_utils.h"
#include "globals.h"
#include "auxiliaries.h"
#include "units.h"
#include "shared.h"
#include "scheduler_fuel_controller.h"

static void setup_calc_conditions(void)
{
    digitalWrite(TANK_EMPTY_PIN, configPage10.wmiEmptyPolarity ? HIGH : LOW);
    // if( (currentStatus.TPS >= configPage10.wmiTPS)
    currentStatus.TPS = configPage10.wmiTPS + 1; 
    //  && (currentStatus.RPMdiv100 >= configPage10.wmiRPM) 
    currentStatus.setRpm(RPM_COARSE.toUser(configPage10.wmiRPM+1));
    //  && ( (currentStatus.MAP / 2U) >= configPage10.wmiMAP) 
    currentStatus.MAP = (configPage10.wmiMAP + ((configPage10.wmiMAP2-configPage10.wmiMAP)/2U))*2U;
    //  && ( temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT) )
    currentStatus.IAT = temperatureRemoveOffset(configPage10.wmiIAT+1);
}

static void assert_tank_empty(void)
{
    digitalWrite(TANK_EMPTY_PIN, configPage10.wmiEmptyPolarity ? LOW : HIGH);
    currentStatus.wmiTankEmpty = false;
    wmiControl();
    TEST_ASSERT_TRUE(currentStatus.wmiTankEmpty);
}

static void test_tank_empty(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    initialiseAuxPWM();
    
    assert_tank_empty();
    
    // Reverse polarity
    configPage10.wmiEmptyPolarity = !configPage10.wmiEmptyPolarity; 
    assert_tank_empty();
}

static void assert_tank_not_empty(void)
{
    digitalWrite(TANK_EMPTY_PIN, configPage10.wmiEmptyPolarity ? HIGH : LOW);
    currentStatus.wmiTankEmpty = true;
    wmiControl();
    TEST_ASSERT_FALSE(currentStatus.wmiTankEmpty);
}

static void test_tank_not_empty(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    initialiseAuxPWM();

    assert_tank_not_empty();

    // Reverse polarity
    configPage10.wmiEmptyPolarity = !configPage10.wmiEmptyPolarity; 
    assert_tank_not_empty();

    // Disabled 
    configPage10.wmiEmptyEnabled = false;
    assert_tank_not_empty();
}

static void assert_wmipw(uint8_t expected)
{
    currentStatus.wmiPW = 99;
    wmiControl();
    TEST_ASSERT_EQUAL(expected, currentStatus.wmiPW);
}

static void setup_assert_wmipw(uint8_t expected)
{
    setup_calc_conditions();
    assert_wmipw(expected);
}

static void test_calc_conditions(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    initialiseAuxPWM();

    // Assert initial conditions
    setup_calc_conditions();
    assert_wmipw(200);

    // TPS
    setup_calc_conditions();
    currentStatus.TPS = configPage10.wmiTPS; 
    assert_wmipw(200);

    setup_calc_conditions();
    currentStatus.TPS = configPage10.wmiTPS - 1; 
    assert_wmipw(0);
    
    // RPM
    setup_calc_conditions();
    currentStatus.setRpm(RPM_COARSE.toUser(configPage10.wmiRPM));
    assert_wmipw(200);
    
    setup_calc_conditions();
    currentStatus.setRpm(RPM_COARSE.toUser(configPage10.wmiRPM-1));
    assert_wmipw(0);
    
    // MAP
    setup_calc_conditions();
    currentStatus.MAP = configPage10.wmiMAP*2U;
    assert_wmipw(200);
    
    setup_calc_conditions();
    currentStatus.MAP = (configPage10.wmiMAP*2U)-1;
    assert_wmipw(0);
    
    // IAT
    setup_calc_conditions();
    currentStatus.IAT = temperatureRemoveOffset(configPage10.wmiIAT);
    assert_wmipw(200);
    
    setup_calc_conditions();
    currentStatus.IAT = temperatureRemoveOffset(configPage10.wmiIAT)-1U;
    assert_wmipw(0);
}

static void test_mode_simple(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    initialiseAuxPWM();

    setup_assert_wmipw(200);
}

static void test_mode_proportional(void)
{
    setup_wmi_tune(WMI_MODE_PROPORTIONAL);
    initialiseAuxPWM();

    setup_assert_wmipw(100);

    // Clamp <=200
    configPage10.wmiMAP2 = configPage10.wmiMAP+10;
    assert_wmipw(200);
}

static void test_mode_ol(void)
{
    setup_wmi_tune(WMI_MODE_OPENLOOP);
    initialiseAuxPWM();

    fill_table_values(wmiTable, 33);
    populate_table_axis(wmiTable.axisX.begin(), 10);
    populate_table_axis(wmiTable.axisY.begin(), 10);

    setup_assert_wmipw(wmiTable.values.values[0]);

    // Clamp <=200
    fill_table_values(wmiTable, 255);
    configPage10.wmiMAP2 = configPage10.wmiMAP+10;
    assert_wmipw(200);
}

static void test_mode_cl(void)
{
    setup_wmi_tune(WMI_MODE_CLOSEDLOOP);
    initialiseAuxPWM();

    populate_table_axis(wmiTable.axisX.begin(), 10);
    populate_table_axis(wmiTable.axisY.begin(), 10);
    fill_table_values(wmiTable, 177);
    fuelSchedule1.pw = abs(configPage10.wmiOffset)*2;

    setup_assert_wmipw(29);

    // Test 0-200 clamp
    fill_table_values(wmiTable, 1);
    fuelSchedule1.pw = 1;
    assert_wmipw(0);

    fill_table_values(wmiTable, 255);
    fuelSchedule1.pw = 255;
#if defined(__AVR__)
    assert_wmipw(212); // Overflow bug!!!
#else
    assert_wmipw(200);
#endif
}

static void test_mode_other(void)
{
    setup_wmi_tune(WMI_MODE_CLOSEDLOOP*2U);
    initialiseAuxPWM();

    setup_assert_wmipw(0);
}

static void test_disabled(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    configPage10.wmiEnabled = false;    
    initialiseAuxPWM();

    setup_assert_wmipw(99);
}

static void test_vvt_enabled(void)
{
    setup_wmi_tune(WMI_MODE_SIMPLE);
    configPage10.vvt2Enabled = true;
    initialiseAuxPWM();

    setup_assert_wmipw(99);
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
    RUN_TEST_P(test_vvt_enabled);
    RUN_TEST_P(test_calc_conditions);
  }
}