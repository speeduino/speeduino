
#include "../test_utils.h"
#include "globals.h"
#include "shared.h"
#include "units.h"

static constexpr table3d_axis_t TEST_VVT_AXIS_X[8] = {
  7000U/100U,
  6000U/100U,
  5000U/100U,
  4000U/100U,
  3000U/100U,
  2000U/100U,
  1000U/100U,
  800U/100U
};

static constexpr table3d_axis_t TEST_VVT_AXIS_Y[8] = {
  100U/2U,
  80U/2U,
  60U/2U,
  50U/2U,
  40U/2U,
  30U/2U,
  20U/2U,
  10U/2U
};

void populate_vvt_tables(table3d_value_t vvt1Value, table3d_value_t vvt2Value)
{
  populate_table_axis_P(vvtTable.axisX.begin(), TEST_VVT_AXIS_X);
  populate_table_axis_P(vvtTable.axisY.begin(), TEST_VVT_AXIS_Y);
  fill_table_values(vvtTable, vvt1Value);

  populate_table_axis_P(vvt2Table.axisX.begin(), TEST_VVT_AXIS_X);
  populate_table_axis_P(vvt2Table.axisY.begin(), TEST_VVT_AXIS_Y);
  fill_table_values(vvt2Table, vvt2Value);
}

static void setup_vvt_basic_tune(uint8_t mode, uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
  pinNumbers.pinVVT_1 = TEST_VVT1_PIN;
  pinNumbers.pinVVT_2 = TEST_VVT2_PIN;

  configPage6.vvtEnabled = true;
  configPage6.vvtFreq = 1U;
  configPage4.vvtMinClt = temperatureAddOffset(60);
  configPage4.vvtDelay = 0U;
  configPage6.vvtLoadSource = loadSource;
  configPage6.vvtMode = mode;
  configPage4.TrigPattern = 0U;

  configPage10.vvt2Enabled = vvt2Enabled;
  configPage10.wmiEnabled = wmiEnabled;
  configPage10.vvtCLMinAng = INT8_MIN+5;
  configPage10.vvtCLMaxAng = UINT8_MAX-5;
  configPage10.vvtCLholdDuty = 100U;
}

void setup_vvt_openloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
    setup_vvt_basic_tune(VVT_MODE_OPEN_LOOP, loadSource, vvt2Enabled, wmiEnabled);
    populate_vvt_tables(150U, 150U);
}

void setup_vvt_onoff_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
    setup_vvt_basic_tune(VVT_MODE_ONOFF, loadSource, vvt2Enabled, wmiEnabled);
}

void setup_vvt_closedloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
  setup_vvt_basic_tune(VVT_MODE_CLOSED_LOOP, loadSource, vvt2Enabled, wmiEnabled);
  configPage6.vvtPWMdir = 1U;
  configPage4.vvt2PWMdir = configPage6.vvtPWMdir;
  configPage10.vvtCLholdDuty = 120U;
  configPage10.vvtCLKP = 5;
  configPage10.vvtCLKI = 4;
  configPage10.vvtCLKD = 3;
  configPage10.vvtCLminDuty = 0;
  configPage10.vvtCLmaxDuty = UINT8_MAX;
  populate_vvt_tables(150U, 150U);
}
