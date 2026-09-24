#include "../test_utils.h"
#include "src/controllers/auxChannels/auxChannelController.h"
#include "src/pins/pinMapping.h"
#include "globals.h"

extern bool _auxEnabled;

static uint8_t getPinModeForTest(uint8_t pin)
{
  auto bit = digitalPinToBitMask(pin);
  auto port = digitalPinToPort(pin);

  if (0 == port) return 0xFFU;
  if (0 == bit) return 0xFFU;
  if (bit & (bit - 1U)) return 0xFFU;

  auto reg = portModeRegister(port);
  auto out = portOutputRegister(port);

  if (*reg & bit) return OUTPUT;
  if (*out & bit) return INPUT_PULLUP;
  return INPUT;
}

static void test_initAuxChannels_external_can_enables_aux(void)
{
  statuses current{};
  config9 page9{};
  _auxEnabled = false;
  current.ioError = false;

  page9.enable_secondarySerial = 1U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 1U;
  page9.caninput_sel[0] = 4U;

  initAuxChannels(current, page9);

  TEST_ASSERT_TRUE(_auxEnabled);
  TEST_ASSERT_FALSE(current.ioError);
}

static void test_initAuxChannels_analog_local_pin_sets_input_and_flag(void)
{
  statuses current{};
  config9 page9{};
  const uint8_t analogPin = pinTranslateAnalog(7U);
  _auxEnabled = false;
  current.ioError = false;

  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 0U;
  page9.caninput_sel[1] = 2U;
  page9.Auxinpina[1] = 7U;

  initAuxChannels(current, page9);

  TEST_ASSERT_TRUE(_auxEnabled);
  TEST_ASSERT_FALSE(current.ioError);
  TEST_ASSERT_EQUAL_UINT8(INPUT, getPinModeForTest(analogPin));
}

static void test_initAuxChannels_analog_local_pin_conflict_sets_io_error(void)
{
  statuses current{};
  config9 page9{};
  const uint8_t analogPin = pinTranslateAnalog(7U);
  const uint8_t savedCltPin = pinNumbers.pinCLT;
  _auxEnabled = false;
  current.ioError = false;

  pinNumbers.pinCLT = analogPin;
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 0U;
  page9.caninput_sel[2] = 2U;
  page9.Auxinpina[2] = 7U;

  initAuxChannels(current, page9);

  TEST_ASSERT_FALSE(_auxEnabled);
  TEST_ASSERT_TRUE(current.ioError);

  pinNumbers.pinCLT = savedCltPin;
}

static void test_initAuxChannels_digital_local_pin_sets_input_and_flag(void)
{
  statuses current{};
  config9 page9{};
  const uint8_t digitalPin = 26U;
  _auxEnabled = false;
  current.ioError = false;

  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 0U;
  page9.caninput_sel[3] = 3U;
  page9.Auxinpinb[3] = digitalPin - 1U;

  initAuxChannels(current, page9);

  TEST_ASSERT_TRUE(_auxEnabled);
  TEST_ASSERT_FALSE(current.ioError);
  TEST_ASSERT_EQUAL_UINT8(INPUT, getPinModeForTest(digitalPin));
}

static void test_initAuxChannels_digital_local_pin_conflict_sets_io_error(void)
{
  statuses current{};
  config9 page9{};
  const uint8_t digitalPin = 27U;
  const uint8_t savedFanPin = pinNumbers.pinFan;
  const uint8_t savedFanEnable = configPage2.fanEnable;
  _auxEnabled = false;
  current.ioError = false;

  pinNumbers.pinFan = digitalPin;
  configPage2.fanEnable = 1U;
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 0U;
  page9.caninput_sel[4] = 3U;
  page9.Auxinpinb[4] = digitalPin - 1U;

  initAuxChannels(current, page9);

  TEST_ASSERT_FALSE(_auxEnabled);
  TEST_ASSERT_TRUE(current.ioError);

  pinNumbers.pinFan = savedFanPin;
  configPage2.fanEnable = savedFanEnable;
}

static void test_initAuxChannels_disabled_channel_keeps_aux_clear(void)
{
  statuses current{};
  config9 page9{};
  _auxEnabled = false;
  current.ioError = false;

  initAuxChannels(current, page9);

  TEST_ASSERT_FALSE(_auxEnabled);
  TEST_ASSERT_FALSE(current.ioError);
}

void testAuxChannelInit(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_initAuxChannels_external_can_enables_aux);
    RUN_TEST_P(test_initAuxChannels_analog_local_pin_sets_input_and_flag);
    RUN_TEST_P(test_initAuxChannels_analog_local_pin_conflict_sets_io_error);
    RUN_TEST_P(test_initAuxChannels_digital_local_pin_sets_input_and_flag);
    RUN_TEST_P(test_initAuxChannels_digital_local_pin_conflict_sets_io_error);
    RUN_TEST_P(test_initAuxChannels_disabled_channel_keeps_aux_clear);
  }
}