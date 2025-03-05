#include <unity.h>
#include "../test_utils.h"
#include "pw_calcs.h"

extern pulseWidths calculateSecondaryPw(uint16_t primaryPw, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, const statuses &current);

struct testContext {
    config2 page2;
    config10 page10;
    statuses current;
};

static testContext getStageContext(uint8_t mode) {
    testContext context = {};
    
    context.page10.stagingEnabled = true;
    context.page10.stagingMode = mode;
    context.page2.nCylinders = INJ_CHANNELS/2U;
    context.page2.injType = INJ_TYPE_PORT;
    context.page10.stagedInjSizePri = 250;
    context.page10.stagedInjSizeSec = 500;

    return context;
}

static void test_calculateSecondaryPw_disabled(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);
 
    // Set up test conditions
    uint16_t primaryPw = 9000;
    uint16_t pwLimit = 9000;
    uint16_t injOpenTime = 1000;
    context.page10.stagingEnabled = false;
   
    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(primaryPw, result.primary);
    TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_calculateSecondaryPw_noprimary(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);

    // Set up test conditions
    uint16_t primaryPw = 0;
    uint16_t pwLimit = 25500;
    uint16_t injOpenTime = 100;
  
    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(primaryPw, result.primary);
    TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_calculateSecondaryPw_notenoughchannels(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);
    
    // Set up test conditions
    uint16_t primaryPw = 9000;
    uint16_t pwLimit = 9000;
    uint16_t injOpenTime = 1000;
    context.page2.nCylinders = INJ_CHANNELS+1;

    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(primaryPw, result.primary);
    TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_calculateSecondaryPw_notenoughchannels_tbody(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);
    
    // Set up test conditions
    uint16_t primaryPw = 9000;
    uint16_t pwLimit = 9000;
    uint16_t injOpenTime = 1000;
    context.page2.nCylinders = INJ_CHANNELS;
    context.page2.injType = INJ_TYPE_TBODY;

    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(primaryPw, result.primary);
    TEST_ASSERT_EQUAL(9000, result.secondary);
}

static void test_calculateSecondaryPw_auto_50pct(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);

    // Set up test conditions
    uint16_t primaryPw = 9000;
    uint16_t pwLimit = 9000;
    uint16_t injOpenTime = 1000;
  
    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(pwLimit, result.primary);
    TEST_ASSERT_EQUAL(9000, result.secondary);
}

static void test_calculateSecondaryPw_auto_33pct(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);

    // Set up test conditions
    uint16_t primaryPw = 7000;
    uint16_t pwLimit = 9000;
    uint16_t injOpenTime = 0;
  
    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(pwLimit, result.primary);
    TEST_ASSERT_EQUAL(6000, result.secondary);
}

static void test_calculateSecondaryPw_auto_inactive(void) {
    testContext context = getStageContext(STAGING_MODE_AUTO);

    // Set up test conditions
    uint16_t primaryPw = 3000;
    uint16_t pwLimit = 9000;
    uint16_t injOpenTime = 1000;
  
    auto result = calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, context.page2, context.page10, context.current);
    
    TEST_ASSERT_EQUAL(7000, result.primary);
    TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_calculateSecondaryPw_table(uint8_t split, uint16_t expectedPrimary, uint16_t expectedSecondary) {
  auto context = getStageContext(STAGING_MODE_TABLE);

  context.current.fuelLoad = 50;  
  context.current.RPM = 3500;

  TEST_DATA_P table3d_axis_t xAxis[] = { 900, 1600, 2500, 3500, 4700, 5900, 6750, 7000};
  TEST_DATA_P table3d_axis_t yAxis[] = { 16,  31,   43,   52,   63,   75,   81,   103 };
  populate_table_axis_P(stagingTable.axisX.begin(), xAxis);
  populate_table_axis_P(stagingTable.axisY.begin(), yAxis);
  fill_table_values(stagingTable, split);

  auto result = calculateSecondaryPw(1500, 1501, 0, context.page2, context.page10, context.current);
  TEST_ASSERT_INT16_WITHIN(30, expectedPrimary, result.primary);
  TEST_ASSERT_INT16_WITHIN(30, expectedSecondary, result.secondary);
}


static void test_calculateSecondaryPw_table_split0(void) {
  test_calculateSecondaryPw_table(0, 4500, 0);
}

static void test_calculateSecondaryPw_table_split33(void) {
  // Secondary injectors are configured to be twice the flow rate
  // of the primaries, so the need half the pulse width
  test_calculateSecondaryPw_table(33, 3000, 750);
}

static void test_calculateSecondaryPw_table_split66(void) {
  // Secondary injectors are configured to be twice the flow rate
  // of the primaries, so the need half the pulse width
  test_calculateSecondaryPw_table(66, 1500, 1500);
}

static void test_calculateSecondaryPw_table_split100(void) {
  // Secondary injectors are configured to be twice the flow rate
  // of the primaries, so the need half the pulse width
  test_calculateSecondaryPw_table(100, 0, 2250);
}

void testCalculateSecondaryPw(void) {
  SET_UNITY_FILENAME() {    
    RUN_TEST_P(test_calculateSecondaryPw_disabled);
    RUN_TEST_P(test_calculateSecondaryPw_noprimary);
    RUN_TEST_P(test_calculateSecondaryPw_notenoughchannels);
    RUN_TEST_P(test_calculateSecondaryPw_notenoughchannels_tbody);
    RUN_TEST_P(test_calculateSecondaryPw_auto_50pct);
    RUN_TEST_P(test_calculateSecondaryPw_auto_33pct);
    RUN_TEST_P(test_calculateSecondaryPw_auto_inactive);
    RUN_TEST_P(test_calculateSecondaryPw_table_split0);
    RUN_TEST_P(test_calculateSecondaryPw_table_split33);
    RUN_TEST_P(test_calculateSecondaryPw_table_split66);
    RUN_TEST_P(test_calculateSecondaryPw_table_split100);
  }
}
