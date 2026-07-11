#include "../test_utils.h"
#include "globals.h"
#include "sensors.h"
#include "setup_oneMsInterval.h"

static void test_flex_low_freq_yields_zero_pct(void)
{
  setup_oneMsInterval();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;        // No filter, immediate
  flexCounter = 10U;                    // Below flexFreqLow
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 100U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.ethanolPct);
  TEST_ASSERT_EQUAL_UINT32(0UL, flexCounter);
}

static void test_flex_in_band_yields_pct(void)
{
  setup_oneMsInterval();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 75U;                    // 25% ethanol
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 0U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(25U, currentStatus.ethanolPct);
}


static void test_flex_over_range_error_yields_zero_pct(void)
{
  setup_oneMsInterval();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 200U;                   // > flexFreqHigh+1 and >= flexFreqLow+19 -> error
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 50U;       // Should drop to 0 (error condition)

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.ethanolPct);
  TEST_ASSERT_EQUAL_UINT32(0UL, flexCounter);
}

static void test_flex_off_by_one_correction(void)
{
  setup_oneMsInterval();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 51U;                    // tempEthPct = 1 -> off-by-one corrected to 0
  flexPulseWidth = 1000UL;
  currentStatus.ethanolPct = 75U;

  run_n_intervals(1000);
  TEST_ASSERT_EQUAL_UINT8(0U, currentStatus.ethanolPct);
}

static void test_flex_clamps_pulsewidth_low(void)
{
  setup_oneMsInterval();
  configPage2.flexEnabled = true;
  configPage2.flexFreqLow = 50U;
  configPage2.flexFreqHigh = 150U;
  configPage4.FILTER_FLEX = 0U;
  flexCounter = 100U;                   // 50% ethanol
  flexPulseWidth = 0UL;                  // Below 1000us -> clamp to 1000

  run_n_intervals(1000);
  // With pw clamped at 1000us: tempX100 = (4224*1000)/1024 - 8125 = 4125 - 8125 = -4000
  // fuelTemp = -4000/100 = -40C
  TEST_ASSERT_EQUAL_INT16(-40, currentStatus.fuelTemp);
}

void testFlex(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_flex_low_freq_yields_zero_pct);
    RUN_TEST(test_flex_in_band_yields_pct);
    RUN_TEST(test_flex_over_range_error_yields_zero_pct);
    RUN_TEST(test_flex_off_by_one_correction);
    RUN_TEST(test_flex_clamps_pulsewidth_low);
  }
}
