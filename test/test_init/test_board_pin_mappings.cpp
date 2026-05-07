#include <unity.h>
#include "globals.h"
#include "init.h"
#include "pages.h"
#include "../test_utils.h"

void prepareForInitialiseAll(uint8_t boardId);

// Each native-compatible case in setPinMapping() assigns the core injector,
// coil, trigger and fuel-pump pins. For coverage purposes we only need to
// dispatch the switch and verify the case body actually ran (i.e. left the
// pin globals in a non-default state). We deliberately keep the assertions
// loose because the goal is exercising the case branches across as many
// board layouts as possible — the existing initialiseAll() tests already
// pin down the exact wiring for the V0.3, V0.4 and MX5 boards.

static void assert_core_pins_assigned(const char *label)
{
  // pinInjector1 / pinCoil1 are always set inside every native-compatible
  // case in setPinMapping(). If we land in a default/empty case they stay
  // zero, which is also what setTuneToEmpty() leaves them at.
  TEST_ASSERT_NOT_EQUAL_MESSAGE(0U, pinInjector1, label);
  TEST_ASSERT_NOT_EQUAL_MESSAGE(0U, pinCoil1, label);
}

static void apply_board(uint8_t boardId)
{
  setTuneToEmpty();
  configPage2.pinMapping = boardId;
  setPinMapping(boardId);
}

static void test_setPinMapping_board1_v02(void)
{
  apply_board(1U);
  assert_core_pins_assigned("Board 1 (V0.2)");
}

static void test_setPinMapping_board2_v03(void)
{
  apply_board(2U);
  assert_core_pins_assigned("Board 2 (V0.3)");
}

static void test_setPinMapping_board3_v04(void)
{
  apply_board(3U);
  assert_core_pins_assigned("Board 3 (V0.4)");
  // Spot-check a known-good wiring: pinTachOut on V0.4 is pin 49.
  TEST_ASSERT_EQUAL_UINT8(49U, pinTachOut);
}

static void test_setPinMapping_board6_mx5_pnp(void)
{
  apply_board(6U);
  assert_core_pins_assigned("Board 6 (MX5 2001-05 PNP)");
  // Specific pin assignments listed in init.cpp for case 6.
  TEST_ASSERT_EQUAL_UINT8(44U, pinInjector1);
  TEST_ASSERT_EQUAL_UINT8(42U, pinCoil1);
}

static void test_setPinMapping_board8(void)
{
  apply_board(8U);
  assert_core_pins_assigned("Board 8");
}

static void test_setPinMapping_board9_mx5_8995(void)
{
  apply_board(9U);
  assert_core_pins_assigned("Board 9 (MX5 89-95)");
}

static void test_setPinMapping_board10(void)
{
  apply_board(10U);
  assert_core_pins_assigned("Board 10");
}

static void test_setPinMapping_board14_levin(void)
{
  apply_board(14U);
  assert_core_pins_assigned("Board 14 (Levin)");
}

static void test_setPinMapping_board20(void)
{
  apply_board(20U);
  assert_core_pins_assigned("Board 20");
}

static void test_setPinMapping_board30(void)
{
  apply_board(30U);
  assert_core_pins_assigned("Board 30");
}

static void test_setPinMapping_board31_bmw_pnp(void)
{
  apply_board(31U);
  assert_core_pins_assigned("Board 31 (BMW PnP)");
}

static void test_setPinMapping_board40_ua4c(void)
{
  apply_board(40U);
  assert_core_pins_assigned("Board 40 (UA4C)");
}

static void test_setPinMapping_board41(void)
{
  apply_board(41U);
  assert_core_pins_assigned("Board 41");
}

static void test_setPinMapping_board42_blitzbox(void)
{
  apply_board(42U);
  assert_core_pins_assigned("Board 42 (Blitzbox BL49sp)");
}

static void test_setPinMapping_board45(void)
{
  apply_board(45U);
  assert_core_pins_assigned("Board 45");
}

static void test_setPinMapping_default_triggerTeeth_guard(void)
{
  // setPinMapping() bumps configPage4.triggerTeeth to 4 if it was 0 to avoid
  // a downstream divide-by-zero. Verify the guard fires.
  setTuneToEmpty();
  configPage4.triggerTeeth = 0U;
  setPinMapping(3U);
  TEST_ASSERT_EQUAL_UINT8(4U, configPage4.triggerTeeth);
}

static void test_setPinMapping_default_triggerTeeth_preserved(void)
{
  // If triggerTeeth is already non-zero, the guard should not change it.
  setTuneToEmpty();
  configPage4.triggerTeeth = 24U;
  setPinMapping(3U);
  TEST_ASSERT_EQUAL_UINT8(24U, configPage4.triggerTeeth);
}

static void test_setPinMapping_resets_output_controls_to_direct(void)
{
  // setPinMapping() unconditionally resets injector & ignition output control
  // back to OUTPUT_CONTROL_DIRECT before applying the per-board overrides.
  setTuneToEmpty();
  injectorOutputControl = 0xFFU;
  ignitionOutputControl = 0xFFU;
  setPinMapping(3U);  // V0.4 is direct-drive
  TEST_ASSERT_EQUAL_UINT8(OUTPUT_CONTROL_DIRECT, injectorOutputControl);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT_CONTROL_DIRECT, ignitionOutputControl);
}

void testBoardPinMappings(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_setPinMapping_board1_v02);
    RUN_TEST_P(test_setPinMapping_board2_v03);
    RUN_TEST_P(test_setPinMapping_board3_v04);
    RUN_TEST_P(test_setPinMapping_board6_mx5_pnp);
    RUN_TEST_P(test_setPinMapping_board8);
    RUN_TEST_P(test_setPinMapping_board9_mx5_8995);
    RUN_TEST_P(test_setPinMapping_board10);
    RUN_TEST_P(test_setPinMapping_board14_levin);
    RUN_TEST_P(test_setPinMapping_board20);
    RUN_TEST_P(test_setPinMapping_board30);
    RUN_TEST_P(test_setPinMapping_board31_bmw_pnp);
    RUN_TEST_P(test_setPinMapping_board40_ua4c);
    RUN_TEST_P(test_setPinMapping_board41);
    RUN_TEST_P(test_setPinMapping_board42_blitzbox);
    RUN_TEST_P(test_setPinMapping_board45);
    RUN_TEST_P(test_setPinMapping_default_triggerTeeth_guard);
    RUN_TEST_P(test_setPinMapping_default_triggerTeeth_preserved);
    RUN_TEST_P(test_setPinMapping_resets_output_controls_to_direct);
  }
}
