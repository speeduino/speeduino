#include "../test_utils.h"
#include "globals.h"
#include "sensors.h"
#include "TS_CommandButtonHandler.h"
#include "scheduledIO_direct_inj.h"
#include "scheduledIO_direct_ign.h"

extern byte HWTest_INJ_Pulsed;
extern byte HWTest_IGN_Pulsed;

struct test_context_t
{
    statuses current;
    config2 page2;

    test_context_t()
    {
        current.RPM = 0U;
        current.isTestModeActive = false;
        HWTest_INJ_Pulsed = 0U;
        HWTest_IGN_Pulsed = 0U;
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

static void test_handler_test_enbl_sets_active(void)
{
    test_context_t context;
    TEST_ASSERT_TRUE(context.handleTsCommand(TS_CMD_TEST_ENBL));
    TEST_ASSERT_TRUE(context.current.isTestModeActive);
}

static void test_handler_test_dsbl_clears_active_and_pulsed(void)
{
    test_context_t context;
    // First enable & flag pulsed bits, then disable
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    HWTest_INJ_Pulsed = 0xFFU;
    HWTest_IGN_Pulsed = 0xFFU;

    TEST_ASSERT_TRUE(context.handleTsCommand(TS_CMD_TEST_DSBL));
    TEST_ASSERT_FALSE(context.current.isTestModeActive);
    TEST_ASSERT_EQUAL_UINT8(0U, HWTest_INJ_Pulsed);
    TEST_ASSERT_EQUAL_UINT8(0U, HWTest_IGN_Pulsed);
}

static void test_handler_rejects_stop_required_when_engine_running(void)
{
    test_context_t context;
    context.current.RPM = 1000U;  // engine running
    // INJ1_ON is in the stop-required range
    TEST_ASSERT_FALSE(context.handleTsCommand(TS_CMD_INJ1_ON));
    // Verify the command did not flip test mode on
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

static void test_handler_vss_ratio_with_vss(uint16_t ratioCmd, uint16_t config2::* pRatio)
{
  auto context = setup_vss(250);
  context.page2.*pRatio = 0U;
  TEST_ASSERT_TRUE(context.handleTsCommand(ratioCmd));
  TEST_ASSERT_TRUE(context.current.vssUiRefresh);
  TEST_ASSERT_EQUAL_UINT16((context.current.vss*10000UL)/context.current.RPM, context.page2.*pRatio);
}

static void test_handler_vss_ratio_no_vss_no_change(uint16_t ratioCmd, uint16_t config2::* pRatio)
{
  auto context = setup_vss(0);
  context.page2.*pRatio = 999U;
  TEST_ASSERT_TRUE(context.handleTsCommand(ratioCmd));
  TEST_ASSERT_FALSE(context.current.vssUiRefresh);
  TEST_ASSERT_EQUAL_UINT16(999, context.page2.*pRatio);
}

static void test_handler_vss_ratio1_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO1, &config2::vssRatio1);
}

static void test_handler_vss_ratio2_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO2, &config2::vssRatio2);
}

static void test_handler_vss_ratio3_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO3, &config2::vssRatio3);
}

static void test_handler_vss_ratio4_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO4, &config2::vssRatio4);
}

static void test_handler_vss_ratio5_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO5, &config2::vssRatio5);
}

static void test_handler_vss_ratio6_with_vss(void)
{
    test_handler_vss_ratio_with_vss(TS_CMD_VSS_RATIO6, &config2::vssRatio6);
}

static void test_handler_vss_ratio1_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO1, &config2::vssRatio1);
}
static void test_handler_vss_ratio2_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO2, &config2::vssRatio2);
}
static void test_handler_vss_ratio3_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO3, &config2::vssRatio3);
}
static void test_handler_vss_ratio4_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO4, &config2::vssRatio4);
}
static void test_handler_vss_ratio5_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO5, &config2::vssRatio5);
}
static void test_handler_vss_ratio6_no_vss_no_change(void)
{
    test_handler_vss_ratio_no_vss_no_change(TS_CMD_VSS_RATIO6, &config2::vssRatio6);
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
    vssPulse();
    delay(1);
    vssPulse();
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
// or flip a HWTest_*_Pulsed bit. The tests below sweep every channel to make
// sure every case label compiles, dispatches and updates the bitmask the way
// the channel-1 case does.

static uint16_t createCmd(uint16_t reference, uint16_t base, uint8_t channel)
{
    uint16_t multiplier = reference - base;
    uint8_t channel_offset = (channel - 1U) * multiplier;
    return base + channel_offset;
}

static void assert_inj_pulse(test_context_t &context, uint8_t channel)
{
    TEST_ASSERT_TRUE(context.handleTsCommand(createCmd(TS_CMD_INJ2_PULSED, TS_CMD_INJ1_PULSED, channel))); 
}

static void test_handler_inj_n_pulsed_sets_bit(uint8_t channel)
{
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    HWTest_INJ_Pulsed = 0U;
    assert_inj_pulse(context, channel); 
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_INJ_Pulsed, bit));
}

static void test_handler_inj_n_inactive_pulsed_nochange(uint8_t channel)
{
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_DSBL);

    HWTest_INJ_Pulsed = 0U;
    assert_inj_pulse(context, channel); 
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_INJ_Pulsed, bit));

    HWTest_INJ_Pulsed = 0xFFU;
    assert_inj_pulse(context, channel); 
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_INJ_Pulsed, bit));
}

static void test_handler_inj_n_off_clears_bit(uint8_t channel)
{
    uint16_t offCmd = createCmd(TS_CMD_INJ2_OFF, TS_CMD_INJ1_OFF, channel);
    uint8_t bit = channel - 1U;
    
    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    assert_inj_pulse(context, channel); 

    HWTest_INJ_Pulsed = 0xFFU;
    TEST_ASSERT_TRUE(context.handleTsCommand(offCmd));
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_INJ_Pulsed, bit));
}

static void test_handler_inj_n_off_inactive_nochange(uint8_t channel)
{
    uint16_t offCmd = createCmd(TS_CMD_INJ2_OFF, TS_CMD_INJ1_OFF, channel);
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_DSBL);

    HWTest_INJ_Pulsed = 0xFFU;
    TEST_ASSERT_TRUE(context.handleTsCommand(offCmd));
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_INJ_Pulsed, bit));

    HWTest_INJ_Pulsed = 0U;
    TEST_ASSERT_TRUE(context.handleTsCommand(offCmd));
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_INJ_Pulsed, bit));
}

static void test_handler_inj_n_on_returns_true(uint8_t channel)
{
    uint16_t onCmd = createCmd(TS_CMD_INJ2_ON, TS_CMD_INJ1_ON, channel);

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    TEST_ASSERT_TRUE(context.handleTsCommand(onCmd));

    context.handleTsCommand(TS_CMD_TEST_DSBL);
    TEST_ASSERT_TRUE(context.handleTsCommand(onCmd));
}

#define DECLARE_INJ_PULSED_TEST(N)                                            \
    static void test_handler_inj##N##_pulsed_sets_bit(void)  \
    { \
        test_handler_inj_n_pulsed_sets_bit(N); \
    } \
    static void test_handler_inj##N##_off_clears_bit(void)  \
    { \
        test_handler_inj_n_off_clears_bit(N); \
    } \
    static void test_handler_inj##N##_on_returns_true(void) \
    { \
        test_handler_inj_n_on_returns_true(N); \
    } \
    static void test_handler_inj##N##_inactive_pulsed_nochange(void) \
    { \
        test_handler_inj_n_inactive_pulsed_nochange(N); \
    } \
    static void test_handler_inj##N##_off_inactive_nochange(void) \
    { \
        test_handler_inj_n_off_inactive_nochange(N); \
    } \
    static void test_handler_inj##N(void) \
    { \
        RUN_TEST_P(test_handler_inj##N##_pulsed_sets_bit); \
        RUN_TEST_P(test_handler_inj##N##_off_clears_bit); \
        RUN_TEST_P(test_handler_inj##N##_on_returns_true); \
        RUN_TEST_P(test_handler_inj##N##_inactive_pulsed_nochange); \
        RUN_TEST_P(test_handler_inj##N##_off_inactive_nochange); \
    }

static void assert_ign_pulse(test_context_t &context, uint8_t channel)
{
    TEST_ASSERT_TRUE(context.handleTsCommand(createCmd(TS_CMD_IGN2_PULSED, TS_CMD_IGN1_PULSED, channel))); 
}

static void test_handler_ign_n_pulsed_sets_bit(uint8_t channel)
{
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    HWTest_IGN_Pulsed = 0U;
    assert_ign_pulse(context, channel);
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_IGN_Pulsed, bit));
}

static void test_handler_ign_n_inactive_pulsed_nochange(uint8_t channel)
{
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_DSBL);

    HWTest_IGN_Pulsed = 0U;
    assert_ign_pulse(context, channel);
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_IGN_Pulsed, bit));

    HWTest_IGN_Pulsed = 0xFFU;
    assert_ign_pulse(context, channel);
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_IGN_Pulsed, bit));
}

static void test_handler_ign_n_off_clears_bit(uint8_t channel)
{
    uint16_t offCmd = createCmd(TS_CMD_IGN2_OFF, TS_CMD_IGN1_OFF, channel);
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    assert_ign_pulse(context, channel);
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_IGN_Pulsed, bit));

    TEST_ASSERT_TRUE(context.handleTsCommand(offCmd));
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_IGN_Pulsed, bit));
}

static void test_handler_ign_n_off_inactive_nochange(uint8_t channel)
{
    uint16_t offCmd = createCmd(TS_CMD_IGN2_OFF, TS_CMD_IGN1_OFF, channel);
    uint8_t bit = channel - 1U;

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_DSBL);

    HWTest_IGN_Pulsed = 0xFFU;
    TEST_ASSERT_TRUE(context.handleTsCommand(offCmd));
    TEST_ASSERT_TRUE(BIT_CHECK(HWTest_IGN_Pulsed, bit));

    HWTest_IGN_Pulsed = 0U;
    TEST_ASSERT_TRUE(context.handleTsCommand(offCmd));
    TEST_ASSERT_FALSE(BIT_CHECK(HWTest_IGN_Pulsed, bit));
}

static void test_handler_ign_n_on_returns_true(uint8_t channel)
{
    uint16_t onCmd = createCmd(TS_CMD_IGN2_ON, TS_CMD_IGN1_ON, channel);

    test_context_t context;
    context.handleTsCommand(TS_CMD_TEST_ENBL);
    TEST_ASSERT_TRUE(context.handleTsCommand(onCmd));

    context.handleTsCommand(TS_CMD_TEST_DSBL);
    TEST_ASSERT_TRUE(context.handleTsCommand(onCmd));
}

#define DECLARE_IGN_PULSED_TEST(N)                                            \
  static void test_handler_ign##N##_pulsed_sets_bit(void)                     \
  {                                                                           \
    test_handler_ign_n_pulsed_sets_bit(N);                                    \
  }                                                                           \
  static void test_handler_ign##N##_off_clears_bit(void)                      \
  {                                                                           \
    test_handler_ign_n_off_clears_bit(N);                                     \
  }                                                                           \
  static void test_handler_ign##N##_on_returns_true(void)                     \
  {                                                                           \
    test_handler_ign_n_on_returns_true(N);                                    \
  }                                                                           \
  static void test_handler_ign##N##n_inactive_pulsed_nochange(void)           \
  {                                                                           \
      test_handler_ign_n_inactive_pulsed_nochange(N);                         \
  }                                                                           \
  static void test_handler_ign##N##_off_inactive_nochange(void)               \
  {                                                                           \
      test_handler_ign_n_off_inactive_nochange(N);                            \
  }                                                                           \
   static void test_handler_ign##N(void)                                      \
  {                                                                           \
    RUN_TEST_P(test_handler_ign##N##_pulsed_sets_bit);                        \
    RUN_TEST_P(test_handler_ign##N##_off_clears_bit);                         \
    RUN_TEST_P(test_handler_ign##N##_on_returns_true);                        \
    RUN_TEST_P(test_handler_ign##N##n_inactive_pulsed_nochange);              \
    RUN_TEST_P(test_handler_ign##N##_off_inactive_nochange);                  \
  }

DECLARE_INJ_PULSED_TEST(1)
#if INJ_CHANNELS >= 2
DECLARE_INJ_PULSED_TEST(2)
#endif
#if INJ_CHANNELS >= 3
DECLARE_INJ_PULSED_TEST(3)
#endif
#if INJ_CHANNELS >= 4
DECLARE_INJ_PULSED_TEST(4)
#endif
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

DECLARE_IGN_PULSED_TEST(1)
#if IGN_CHANNELS >= 2
DECLARE_IGN_PULSED_TEST(2)
#endif
#if IGN_CHANNELS >= 3
DECLARE_IGN_PULSED_TEST(3)
#endif
#if IGN_CHANNELS >= 4
DECLARE_IGN_PULSED_TEST(4)
#endif
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
    RUN_TEST(test_vss_60km_internal_pin);
    RUN_TEST(test_vss_60km_external);

    test_handler_inj1();
#if INJ_CHANNELS >= 2
    test_handler_inj2();
#endif
#if INJ_CHANNELS >= 3
    test_handler_inj3();
#endif
#if INJ_CHANNELS >= 4
    test_handler_inj4();
#endif
#if INJ_CHANNELS >= 5
    test_handler_inj5();
#endif
#if INJ_CHANNELS >= 6
    test_handler_inj6();
#endif
#if INJ_CHANNELS >= 7
    test_handler_inj7();
#endif
#if INJ_CHANNELS >= 8
    test_handler_inj8();
#endif

    test_handler_ign1();
#if IGN_CHANNELS >= 2
    test_handler_ign2();
#endif
#if IGN_CHANNELS >= 3
    test_handler_ign3();
#endif
#if IGN_CHANNELS >= 4
    test_handler_ign4();
#endif
#if IGN_CHANNELS >= 5
    test_handler_ign5();
#endif
#if IGN_CHANNELS >= 6
    test_handler_ign6();
#endif
#if IGN_CHANNELS >= 7
    test_handler_ign7();
#endif
#if IGN_CHANNELS >= 8
    test_handler_ign8();
#endif
  }
}
