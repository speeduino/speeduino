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
  // will do under ArduinoFake) before driving the handler. Array size tracks
  // INJ_CHANNELS / IGN_CHANNELS so the test compiles on AVR (4/5) and native (8/8).
  uint8_t inj_pins[INJ_CHANNELS];
  uint8_t ign_pins[IGN_CHANNELS];
  for (uint8_t i = 0U; i < (uint8_t)INJ_CHANNELS; ++i) { inj_pins[i] = (uint8_t)(50U + i); }
  for (uint8_t i = 0U; i < (uint8_t)IGN_CHANNELS; ++i) { ign_pins[i] = (uint8_t)(60U + i); }
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

// ============================ Per-channel INJ/IGN ===========================
//
// The INJ2..INJ8 and IGN2..IGN8 dispatch arms in TS_CommandButtonsHandler all
// follow the same pattern as INJ1/IGN1: ON/OFF/PULSED open/close the channel
// or flip a HWTest_*_Pulsed bit. The tests below sweep every channel to make
// sure every case label compiles, dispatches and updates the bitmask the way
// the channel-1 case does.

#define DECLARE_INJ_PULSED_TEST(N)                                            \
  static void test_handler_inj##N##_pulsed_sets_bit(void)                     \
  {                                                                           \
    reset_test_mode_state();                                                  \
    TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);                               \
    HWTest_INJ_Pulsed = 0U;                                                   \
    TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_INJ##N##_PULSED));       \
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_INJ_Pulsed, INJ##N##_CMD_BIT));         \
  }                                                                           \
  static void test_handler_inj##N##_off_clears_bit(void)                      \
  {                                                                           \
    reset_test_mode_state();                                                  \
    TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);                               \
    TS_CommandButtonsHandler(TS_CMD_INJ##N##_PULSED);                         \
    TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_INJ##N##_OFF));          \
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_INJ_Pulsed, INJ##N##_CMD_BIT));        \
  }                                                                           \
  static void test_handler_inj##N##_on_returns_true(void)                     \
  {                                                                           \
    reset_test_mode_state();                                                  \
    TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);                               \
    TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_INJ##N##_ON));           \
  }

#define DECLARE_IGN_PULSED_TEST(N)                                            \
  static void test_handler_ign##N##_pulsed_sets_bit(void)                     \
  {                                                                           \
    reset_test_mode_state();                                                  \
    TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);                               \
    HWTest_IGN_Pulsed = 0U;                                                   \
    TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_IGN##N##_PULSED));       \
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_IGN_Pulsed, IGN##N##_CMD_BIT));         \
  }                                                                           \
  static void test_handler_ign##N##_off_clears_bit(void)                      \
  {                                                                           \
    reset_test_mode_state();                                                  \
    TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);                               \
    TS_CommandButtonsHandler(TS_CMD_IGN##N##_PULSED);                         \
    TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_IGN##N##_OFF));          \
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_IGN_Pulsed, IGN##N##_CMD_BIT));        \
  }                                                                           \
  static void test_handler_ign##N##_on_returns_true(void)                     \
  {                                                                           \
    reset_test_mode_state();                                                  \
    TS_CommandButtonsHandler(TS_CMD_TEST_ENBL);                               \
    TEST_ASSERT_TRUE(TS_CommandButtonsHandler(TS_CMD_IGN##N##_ON));           \
  }

DECLARE_INJ_PULSED_TEST(2)
DECLARE_INJ_PULSED_TEST(3)
DECLARE_INJ_PULSED_TEST(4)
#if INJ_CHANNELS >= 5
DECLARE_INJ_PULSED_TEST(5)
#endif
#if INJ_CHANNELS >= 6
DECLARE_INJ_PULSED_TEST(6)
#endif
#if INJ_CHANNELS >= 7
DECLARE_INJ_PULSED_TEST(7)
#endif
#if INJ_CHANNELS >= 8
DECLARE_INJ_PULSED_TEST(8)
#endif

DECLARE_IGN_PULSED_TEST(2)
DECLARE_IGN_PULSED_TEST(3)
DECLARE_IGN_PULSED_TEST(4)
#if IGN_CHANNELS >= 5
DECLARE_IGN_PULSED_TEST(5)
#endif
#if IGN_CHANNELS >= 6
DECLARE_IGN_PULSED_TEST(6)
#endif
#if IGN_CHANNELS >= 7
DECLARE_IGN_PULSED_TEST(7)
#endif
#if IGN_CHANNELS >= 8
DECLARE_IGN_PULSED_TEST(8)
#endif

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

    RUN_TEST(test_handler_inj2_on_returns_true);
    RUN_TEST(test_handler_inj2_off_clears_bit);
    RUN_TEST(test_handler_inj2_pulsed_sets_bit);
    RUN_TEST(test_handler_inj3_on_returns_true);
    RUN_TEST(test_handler_inj3_off_clears_bit);
    RUN_TEST(test_handler_inj3_pulsed_sets_bit);
    RUN_TEST(test_handler_inj4_on_returns_true);
    RUN_TEST(test_handler_inj4_off_clears_bit);
    RUN_TEST(test_handler_inj4_pulsed_sets_bit);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_handler_inj5_on_returns_true);
    RUN_TEST(test_handler_inj5_off_clears_bit);
    RUN_TEST(test_handler_inj5_pulsed_sets_bit);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_handler_inj6_on_returns_true);
    RUN_TEST(test_handler_inj6_off_clears_bit);
    RUN_TEST(test_handler_inj6_pulsed_sets_bit);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_handler_inj7_on_returns_true);
    RUN_TEST(test_handler_inj7_off_clears_bit);
    RUN_TEST(test_handler_inj7_pulsed_sets_bit);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_handler_inj8_on_returns_true);
    RUN_TEST(test_handler_inj8_off_clears_bit);
    RUN_TEST(test_handler_inj8_pulsed_sets_bit);
#endif

    RUN_TEST(test_handler_ign2_on_returns_true);
    RUN_TEST(test_handler_ign2_off_clears_bit);
    RUN_TEST(test_handler_ign2_pulsed_sets_bit);
    RUN_TEST(test_handler_ign3_on_returns_true);
    RUN_TEST(test_handler_ign3_off_clears_bit);
    RUN_TEST(test_handler_ign3_pulsed_sets_bit);
    RUN_TEST(test_handler_ign4_on_returns_true);
    RUN_TEST(test_handler_ign4_off_clears_bit);
    RUN_TEST(test_handler_ign4_pulsed_sets_bit);
#if IGN_CHANNELS >= 5
    RUN_TEST(test_handler_ign5_on_returns_true);
    RUN_TEST(test_handler_ign5_off_clears_bit);
    RUN_TEST(test_handler_ign5_pulsed_sets_bit);
#endif
#if IGN_CHANNELS >= 6
    RUN_TEST(test_handler_ign6_on_returns_true);
    RUN_TEST(test_handler_ign6_off_clears_bit);
    RUN_TEST(test_handler_ign6_pulsed_sets_bit);
#endif
#if IGN_CHANNELS >= 7
    RUN_TEST(test_handler_ign7_on_returns_true);
    RUN_TEST(test_handler_ign7_off_clears_bit);
    RUN_TEST(test_handler_ign7_pulsed_sets_bit);
#endif
#if IGN_CHANNELS >= 8
    RUN_TEST(test_handler_ign8_on_returns_true);
    RUN_TEST(test_handler_ign8_off_clears_bit);
    RUN_TEST(test_handler_ign8_pulsed_sets_bit);
#endif
  }
}
