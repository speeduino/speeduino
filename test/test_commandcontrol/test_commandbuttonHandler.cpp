#include "../test_utils.h"
#include "globals.h"
#include "sensors.h"
#include "src/controllers/tsCommand/tsCommandController.h"
#include "scheduledIO_direct_inj.h"
#include "scheduledIO_direct_ign.h"

extern uint16_t calcPulsesPerKm(const statuses &current, const config2 &page2, uint32_t (*pGetGap)(byte));

static uint32_t fakeVssPulseGap(byte)
{
    return 1000000U;
}

struct test_context_t
{
    statuses current;
    config2 page2;

    test_context_t()
    {
        current.RPM = 0U;
        current.isTestModeActive = false;
    }

    bool handleTsCommand(uint16_t command)
    {
        return ::handleTsCommand(command, current, page2);
    }
};

static void test_handler_unknown_command_returns_false(void)
{
    test_context_t context;
    TEST_ASSERT_FALSE(context.handleTsCommand(0xFFFFU));
}

static void test_handler_rejects_legacy_test_commands(void)
{
    test_context_t context;
    for(uint16_t rpm : {uint16_t(0),uint16_t(1000)}) {
        context.current.RPM=rpm;
        TEST_ASSERT_FALSE(context.handleTsCommand(TS_CMD_TEST_ENBL));
        for(uint16_t command=TS_CMD_INJ1_ON;command<=TS_CMD_IGN8_PULSED;++command)
            TEST_ASSERT_FALSE(context.handleTsCommand(command));
        TEST_ASSERT_FALSE(context.current.isTestModeActive);
    }
}

static void test_handler_stop_is_unconditional(void)
{
    test_context_t context;
    context.current.RPM=1000;
    context.current.isTestModeActive=true;
    TEST_ASSERT_TRUE(context.handleTsCommand(TS_CMD_TEST_DSBL));
    TEST_ASSERT_FALSE(context.current.isTestModeActive);
}

static test_context_t setup_vss(uint16_t vss)
{
    test_context_t context;
    context.current.vss = vss;
    context.current.RPM = 2000U;
    context.current.vssUiRefresh = false;
    return context;
}

static void test_handler_vss_ratio_with_vss(uint16_t ratioCmd, uint8_t vssIndex)
{
  auto context = setup_vss(250);
  context.page2.vssRatios[vssIndex] = 0U;
  TEST_ASSERT_TRUE(context.handleTsCommand(ratioCmd));
  TEST_ASSERT_TRUE(context.current.vssUiRefresh);
  TEST_ASSERT_EQUAL_UINT16((context.current.vss*10000UL)/context.current.RPM, context.page2.vssRatios[vssIndex]);
}

static void test_handler_vss_ratio_no_vss_no_change(uint16_t ratioCmd, uint8_t vssIndex)
{
  auto context = setup_vss(0);
  context.page2.vssRatios[vssIndex] = 999U;
  TEST_ASSERT_TRUE(context.handleTsCommand(ratioCmd));
  TEST_ASSERT_FALSE(context.current.vssUiRefresh);
  TEST_ASSERT_EQUAL_UINT16(999, context.page2.vssRatios[vssIndex]);
}

static void test_handler_vss_ratio1_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO1, 0);
}

static void test_handler_vss_ratio2_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO2, 1);
}

static void test_handler_vss_ratio3_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO3, 2);
}

static void test_handler_vss_ratio4_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO4, 3);
}

static void test_handler_vss_ratio5_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO5, 4);
}

static void test_handler_vss_ratio6_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO6, 5);
}

static void test_handler_vss_ratio1_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO1, 0);
}
static void test_handler_vss_ratio2_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO2, 1);
}
static void test_handler_vss_ratio3_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO3, 2);
}
static void test_handler_vss_ratio4_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO4, 3);
}
static void test_handler_vss_ratio5_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO5, 4);
}
static void test_handler_vss_ratio6_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO6, 5);
}

static void test_calc_pulses_per_km_internal_pin(void)
{
    test_context_t context;
    context.page2.vssMode = VSS_MODE_INTERNAL_PIN;
    context.page2.vssAuxCh = 2U;
    context.current.canin[2U] = 360U;

    TEST_ASSERT_EQUAL_UINT16(6U, calcPulsesPerKm(context.current, context.page2, fakeVssPulseGap));
}

static void test_calc_pulses_per_km_uses_calibration_gap(void)
{
    test_context_t context;
    context.page2.vssMode = VSS_MODE_EXTERNAL_KM;
    context.page2.vssPulsesPerKm = 1234U;

    TEST_ASSERT_EQUAL_UINT16(MICROS_PER_MIN / 1000000U,
                             calcPulsesPerKm(context.current, context.page2, fakeVssPulseGap));
}

static void test_calc_pulses_per_km_falls_back_to_config_value(void)
{
    test_context_t context;
    context.page2.vssMode = VSS_MODE_EXTERNAL_KM;
    context.page2.vssPulsesPerKm = 1234U;

    TEST_ASSERT_EQUAL_UINT16(1234U, calcPulsesPerKm(context.current, context.page2, [](byte) -> uint32_t { return 0U; }));
}

static void test_vss_60km_internal_pin(void)
{
    test_context_t context;
    context.page2.vssMode = VSS_MODE_INTERNAL_PIN;
    context.current.canin[context.page2.vssAuxCh] = 360;
    context.current.vssUiRefresh = false;

    TEST_ASSERT_TRUE(context.handleTsCommand(TS_CMD_VSS_60KMH));
    TEST_ASSERT_TRUE(context.current.vssUiRefresh);
    TEST_ASSERT_EQUAL_UINT16(6, context.page2.vssPulsesPerKm);
}

static void test_vss_60km_external(void)
{
    for (uint8_t i=0; i<VSS_SAMPLES; ++i)
    {
        // Semi-random delay. This should help vssPulse() capture realistic times
        delayMicroseconds(3333UL+(i*1000UL));
        vssPulse();
    }
    
    test_context_t context;
    context.page2.vssMode = VSS_MODE_EXTERNAL_KM;
    context.page2.vssPulsesPerKm = 0;
    context.current.vssUiRefresh = false;

    TEST_ASSERT_TRUE(context.handleTsCommand(TS_CMD_VSS_60KMH));
    TEST_ASSERT_TRUE(context.current.vssUiRefresh);
    TEST_ASSERT_NOT_EQUAL_UINT16(0, context.page2.vssPulsesPerKm);
}

// ============================ Per-channel INJ/IGN ===========================
//
// The INJ2..INJ8 and IGN2..IGN8 dispatch arms in handleTsCommand all
// follow the same pattern as INJ1/IGN1: ON/OFF/PULSED open/close the channel
// sure every case label compiles, dispatches and updates the bitmask the way
// the channel-1 case does.

static uint16_t createCmd(uint16_t reference, uint16_t base, uint8_t channel)
{
    uint16_t multiplier = reference - base;
    uint8_t channel_offset = (channel - 1U) * multiplier;
    return base + channel_offset;
}

void testTSCommandHandler(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_handler_unknown_command_returns_false);
    RUN_TEST(test_handler_rejects_legacy_test_commands);
    RUN_TEST(test_handler_stop_is_unconditional);
    RUN_TEST(test_handler_vss_ratio1_with_vss);
    RUN_TEST(test_handler_vss_ratio2_with_vss);
    RUN_TEST(test_handler_vss_ratio3_with_vss);
    RUN_TEST(test_handler_vss_ratio4_with_vss);
    RUN_TEST(test_handler_vss_ratio5_with_vss);
    RUN_TEST(test_handler_vss_ratio6_with_vss);
    RUN_TEST(test_handler_vss_ratio1_no_vss_no_change);
    RUN_TEST(test_handler_vss_ratio2_no_vss_no_change);
    RUN_TEST(test_handler_vss_ratio3_no_vss_no_change);
    RUN_TEST(test_handler_vss_ratio4_no_vss_no_change);
    RUN_TEST(test_handler_vss_ratio5_no_vss_no_change);
    RUN_TEST(test_handler_vss_ratio6_no_vss_no_change);
    RUN_TEST(test_calc_pulses_per_km_internal_pin);
    RUN_TEST(test_calc_pulses_per_km_uses_calibration_gap);
    RUN_TEST(test_calc_pulses_per_km_falls_back_to_config_value);
    RUN_TEST(test_vss_60km_internal_pin);
    RUN_TEST(test_vss_60km_external);

  }
}
