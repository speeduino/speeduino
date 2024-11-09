#include <unity.h>
#include "../test_utils.h"
#include "pw_test_context.h"
#include "pw_calcs.h"

extern pulseWidths applyStagingToPw(uint16_t primaryPW, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, statuses &current);

static pulseWidths applyStagingToPw(uint16_t primaryPW, uint16_t pwLimit, uint16_t injOpenTime, ComputePulseWidthsContext &context) {
  return applyStagingToPw(primaryPW, pwLimit, injOpenTime, context.page2, context.page10, context.current);
}

static ComputePulseWidthsContext getStageContext(uint8_t mode) {
  auto context = getBasicPwContext();

  context.page10.stagingEnabled = true;
  context.page2.nCylinders = INJ_CHANNELS-1;
  context.page2.injType = INJ_TYPE_PORT;
  context.page10.stagingMode = mode;
  context.page10.stagedInjSizePri = 250; // 3.0x
  context.page10.stagedInjSizeSec = 500; // 1.5x
  context.current.fuelLoad = 50;
  BIT_CLEAR(context.current.status4, BIT_STATUS4_STAGING_ACTIVE);

  return context;
}

static void test_applyStagingToPw_auto(void) {
  auto context = getStageContext(STAGING_MODE_AUTO);
  auto subject = applyStagingToPw(1500, 1000, 0U, context);
  TEST_ASSERT_EQUAL(1000U, subject.primary);
  TEST_ASSERT_EQUAL(1750U, subject.secondary);
  TEST_ASSERT_BIT_HIGH(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);
}

static void test_applyStagingToPw_auto_inactive(void) {
  auto context = getStageContext(STAGING_MODE_AUTO);
  auto subject = applyStagingToPw(500, 1500, 0U, context);
  TEST_ASSERT_EQUAL(1500U, subject.primary);
  TEST_ASSERT_EQUAL(0U, subject.secondary);
  TEST_ASSERT_BIT_LOW(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);
}

static void test_applyStagingToPw_disabled(void) {
  auto context = getStageContext(STAGING_MODE_AUTO);
  context.page10.stagingEnabled = false;
  auto subject = applyStagingToPw(1500, 1000, 0U, context);
  TEST_ASSERT_EQUAL(1000U, subject.primary);
  TEST_ASSERT_EQUAL(0U, subject.secondary);
  TEST_ASSERT_BIT_LOW(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);
}

static void test_applyStagingToPw_toomanycylinders(void) {
  auto context = getStageContext(STAGING_MODE_AUTO);
  context.page2.nCylinders = INJ_CHANNELS + 1U;
  auto subject = applyStagingToPw(1500, 1000, 0U, context);
  TEST_ASSERT_EQUAL(1000U, subject.primary);
  TEST_ASSERT_EQUAL(0U, subject.secondary);
  TEST_ASSERT_BIT_LOW(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);

  // Cylinder count should be ignored if using throttle body injection
  context.page2.injType = INJ_TYPE_TBODY;
  subject = applyStagingToPw(1500, 1000, 0U, context);
  TEST_ASSERT_EQUAL(1000U, subject.primary);
  TEST_ASSERT_EQUAL(1750U, subject.secondary);
  TEST_ASSERT_BIT_HIGH(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);
}

static void test_applyStagingToPw_table(uint8_t split, uint16_t expectedPrimary, uint16_t expectedSecondary) {
  auto context = getStageContext(STAGING_MODE_TABLE);

  context.current.fuelLoad = 50;  
  context.current.RPM = 3500;

  TEST_DATA_P table3d_axis_t xAxis[] = { 900, 1600, 2500, 3500, 4700, 5900, 6750, 7000};
  TEST_DATA_P table3d_axis_t yAxis[] = { 16,  31,   43,   52,   63,   75,   81,   103 };
  populate_table_axis_P(stagingTable.axisX.begin(), xAxis);
  populate_table_axis_P(stagingTable.axisY.begin(), yAxis);
  fill_table_values(stagingTable, split);

  auto subject = applyStagingToPw(1500, 1501, 0U, context);
  TEST_ASSERT_INT16_WITHIN(30, expectedPrimary, subject.primary);
  TEST_ASSERT_INT16_WITHIN(30, expectedSecondary, subject.secondary);
  if (expectedSecondary!=0) {
    TEST_ASSERT_BIT_HIGH(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);
  } else {
    TEST_ASSERT_BIT_LOW(BIT_STATUS4_STAGING_ACTIVE, context.current.status4);
  }
}

static void test_applyStagingToPw_table_split0(void) {
  test_applyStagingToPw_table(0, 4500, 0);
}

static void test_applyStagingToPw_table_split33(void) {
  // Secondary injectors are configured to be twice the flow rate
  // of the primaries, so the need half the pulse width
  test_applyStagingToPw_table(33, 3000, 750);
}

static void test_applyStagingToPw_table_split66(void) {
  // Secondary injectors are configured to be twice the flow rate
  // of the primaries, so the need half the pulse width
  test_applyStagingToPw_table(66, 1500, 1500);
}

static void test_applyStagingToPw_table_split100(void) {
  // Secondary injectors are configured to be twice the flow rate
  // of the primaries, so the need half the pulse width
  test_applyStagingToPw_table(100, 0, 2250);
}

void testApplyStagingToPw(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_applyStagingToPw_auto);
    RUN_TEST_P(test_applyStagingToPw_auto_inactive);
    RUN_TEST_P(test_applyStagingToPw_disabled);
    RUN_TEST_P(test_applyStagingToPw_toomanycylinders);
    RUN_TEST_P(test_applyStagingToPw_table_split0);
    RUN_TEST_P(test_applyStagingToPw_table_split33);   
    RUN_TEST_P(test_applyStagingToPw_table_split66);
    RUN_TEST_P(test_applyStagingToPw_table_split100);
  }
}