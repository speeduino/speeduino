
#include "../test_utils.h"
#include "shared.h"
#include "units.h"
#include "globals.h"

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

static test_context_t setup_vvt_basic_tune(uint8_t mode, uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
  constexpr uint8_t TEST_VVT1_PIN = 19U;
  constexpr uint8_t TEST_VVT2_PIN = 20U;

  test_context_t context;
  context.pins.pinVVT_1 = TEST_VVT1_PIN;
  context.pins.pinVVT_2 = TEST_VVT2_PIN;

  context.page6.vvtEnabled = true;
  context.page6.vvtFreq = 1U;
  context.page4.vvtMinClt = temperatureAddOffset(60);
  context.page4.vvtDelay = 0U;
  context.page6.vvtLoadSource = loadSource;
  context.page6.vvtMode = mode;
  context.page4.TrigPattern = 0U;

  context.page10.vvt2Enabled = vvt2Enabled;
  context.page10.wmiEnabled = wmiEnabled;
  context.page10.vvtCLMinAng = INT8_MIN+5;
  context.page10.vvtCLMaxAng = UINT8_MAX-5;
  context.page10.vvtCLholdDuty = 100U;

  return context;
}

test_context_t setup_vvt_openloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
    populate_vvt_tables(150U, 150U);
    return setup_vvt_basic_tune(VVT_MODE_OPEN_LOOP, loadSource, vvt2Enabled, wmiEnabled);
}

test_context_t setup_vvt_onoff_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
    return setup_vvt_basic_tune(VVT_MODE_ONOFF, loadSource, vvt2Enabled, wmiEnabled);
}

test_context_t setup_vvt_closedloop_tune(uint8_t loadSource, bool vvt2Enabled, bool wmiEnabled)
{
  auto context = setup_vvt_basic_tune(VVT_MODE_CLOSED_LOOP, loadSource, vvt2Enabled, wmiEnabled);
  context.page6.vvtPWMdir = 1U;
  context.page4.vvt2PWMdir = context.page6.vvtPWMdir;
  context.page10.vvtCLholdDuty = 120U;
  context.page10.vvtCLKP = 5;
  context.page10.vvtCLKI = 4;
  context.page10.vvtCLKD = 3;
  context.page10.vvtCLminDuty = 0;
  context.page10.vvtCLmaxDuty = UINT8_MAX;
  populate_vvt_tables(150U, 150U);
  return context;
}
