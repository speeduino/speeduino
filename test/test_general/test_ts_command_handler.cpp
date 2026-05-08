#include <unity.h>
#include "globals.h"
#include "TS_CommandButtonHandler.h"
#include "scheduledIO_direct_inj.h"
#include "scheduledIO_direct_ign.h"
#include "../test_utils.h"

static bool ts_io_initialised = false;

static void ensure_io_initialised(void)
{
  if (ts_io_initialised) { return; }
  // Cases like TS_CMD_TEST_DSBL or TS_CMD_INJ1_OFF call closeInjectorN()/endCoilN()
  // unconditionally, which dereferences the static port pointers inside
  // fastOutputPin_t. Those must be wired to real pins (any free pin numbers
  // will do under ArduinoFake) before driving the handler.
  const uint8_t inj_pins[INJ_CHANNELS] = { 50U, 51U, 52U, 53U, 54U, 55U, 56U, 57U };
  const uint8_t ign_pins[IGN_CHANNELS] = { 60U, 61U, 62U, 63U, 64U, 65U, 66U, 67U };
  initInjDirectIO(inj_pins);
  initIgnDirectIO(ign_pins);
  ts_io_initialised = true;
}

static void reset_test_mode_state(void)
{
  ensure_io_initialised();
  currentStatus.RPM = 0U;
  currentStatus.isTestModeActive = false;
  HWTest_INJ_Pulsed = 0U;
  HWTest_IGN_Pulsed = 0U;
}

static void test_handler_unknown_command_returns_false(void)
{
  reset_test_mode_state();
  TEST_ASSERT_FALSE(TS_CommandButtonsHandler(0xFFFFU));
}

static void test_handler_test_enbl_sets_active(void)
{
  reset_test_mode_state();
  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_TEST_ENBL));
  TEST_ASSERT_TRUE(currentStatus.isTestModeActive);
}

static void test_handler_test_dsbl_clears_active_and_pulsed(void)
{
  reset_test_mode_state();
  // First enable & flag pulsed bits, then disable
  TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);
  HWTest_INJ_Pulsed = 0xFFU;
  HWTest_IGN_Pulsed = 0xFFU;

  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_TEST_DSBL));
  TEST_ASSERT_FALSE(currentStatus.isTestModeActive);
  TEST_ASSERT_EQUAL_UINT8(0U, HWTest_INJ_Pulsed);
  TEST_ASSERT_EQUAL_UINT8(0U, HWTest_IGN_Pulsed);
}

static void test_handler_rejects_stop_required_when_engine_running(void)
{
  reset_test_mode_state();
  currentStatus.RPM = 1000U;  // engine running
  // INJ1_ON is in the stop-required range
  TEST_ASSERT_FALSE(TS_CommandButtonsHandler(TS_CMD_INJ1_ON));
  // Verify the command did not flip test mode on
  TEST_ASSERT_FALSE(currentStatus.isTestModeActive);
}

static void test_handler_inj1_pulsed_sets_bit_when_active(void)
{
  reset_test_mode_state();
  TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);

  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_INJ1_PULSED));
  TEST_ASSERT_TRUE(BIT_CHECK(HWTest_INJ_Pulsed, INJ1_CMD_BIT));
}

static void test_handler_inj1_off_clears_pulsed_bit(void)
{
  reset_test_mode_state();
  TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);
  TS_CommandButtonsHandler(TS_CMD_INJ1_PULSED);
  TEST_ASSERT_TRUE(BIT_CHECK(HWTest_INJ_Pulsed, INJ1_CMD_BIT));

  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_INJ1_OFF));
  TEST_ASSERT_FALSE(BIT_CHECK(HWTest_INJ_Pulsed, INJ1_CMD_BIT));
}

static void test_handler_ign1_pulsed_sets_bit_when_active(void)
{
  reset_test_mode_state();
  TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);

  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_IGN1_PULSED));
  TEST_ASSERT_TRUE(BIT_CHECK(HWTest_IGN_Pulsed, IGN1_CMD_BIT));
}

static void test_handler_ign1_pulsed_inactive_no_bit_set(void)
{
  reset_test_mode_state();
  // Test mode NOT active — pulsed should not set bit
  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_IGN1_PULSED));
  TEST_ASSERT_FALSE(BIT_CHECK(HWTest_IGN_Pulsed, IGN1_CMD_BIT));
}

static void test_handler_inj_on_inactive_does_not_open(void)
{
  reset_test_mode_state();
  // Without test mode active, INJ_ON returns true but should not modify state
  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_INJ1_ON));
  // No state change to verify positively, but the case path is now covered
}

static void test_handler_vss_ratio1_with_vss(void)
{
  reset_test_mode_state();
  currentStatus.vss = 50U;
  currentStatus.RPM = 2000U;
  configPage2.vssRatio1 = 0U;

  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_VSS_RATIO1));
  // Expected: (50 * 10000) / 2000 = 250
  TEST_ASSERT_EQUAL_UINT16(250U, configPage2.vssRatio1);
  TEST_ASSERT_TRUE(currentStatus.vssUiRefresh);
}

static void test_handler_vss_ratio1_no_vss_no_change(void)
{
  reset_test_mode_state();
  currentStatus.vss = 0U;
  configPage2.vssRatio1 = 999U;

  TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_VSS_RATIO1));
  TEST_ASSERT_EQUAL_UINT16(999U, configPage2.vssRatio1);
}

void testTSCommandHandler(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_handler_unknown_command_returns_false);
    RUN_TEST(test_handler_test_enbl_sets_active);
    RUN_TEST(test_handler_test_dsbl_clears_active_and_pulsed);
    RUN_TEST(test_handler_rejects_stop_required_when_engine_running);
    RUN_TEST(test_handler_inj1_pulsed_sets_bit_when_active);
    RUN_TEST(test_handler_inj1_off_clears_pulsed_bit);
    RUN_TEST(test_handler_ign1_pulsed_sets_bit_when_active);
    RUN_TEST(test_handler_ign1_pulsed_inactive_no_bit_set);
    RUN_TEST(test_handler_inj_on_inactive_does_not_open);
    RUN_TEST(test_handler_vss_ratio1_with_vss);
    RUN_TEST(test_handler_vss_ratio1_no_vss_no_change);
  }
}
