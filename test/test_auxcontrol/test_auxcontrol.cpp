#include "../test_utils.h"
#include "src/controllers/auxChannels/auxChannelController.h"
#include "src/pins/pinMapping.h"

using fnSendCanCommand_t = void (*)(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress);
using fnReadAuxanalog_t = uint16_t (*)(uint8_t analogPin);
using fnReadAuxdigital_t = uint16_t (*)(uint8_t digitalPin);

extern void auxChannelControl(statuses &current, const config9 &page9, fnSendCanCommand_t fnSendCanCommand, fnReadAuxanalog_t fnReadAuxanalog, fnReadAuxdigital_t fnReadAuxdigital);

static uint16_t analogReadValue;
static uint16_t digitalReadValue;
static uint8_t analogReadCount;
static uint8_t digitalReadCount;
static uint8_t lastAnalogPin;
static uint8_t lastDigitalPin;
static uint8_t canCommandCount;
static uint8_t lastCanCommandType;
static uint16_t lastCanAddress;
static uint8_t lastCanData1;
static uint8_t lastCanData2;
static uint16_t lastCanSourceAddress;

static uint16_t stubReadAuxanalog(uint8_t analogPin)
{
  ++analogReadCount;
  lastAnalogPin = analogPin;
  return analogReadValue;
}

static uint16_t stubReadAuxdigital(uint8_t digitalPin)
{
  ++digitalReadCount;
  lastDigitalPin = digitalPin;
  return digitalReadValue;
}

static void stubSendCanCommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress)
{
  ++canCommandCount;
  lastCanCommandType = cmdtype;
  lastCanAddress = canaddress;
  lastCanData1 = candata1;
  lastCanData2 = candata2;
  lastCanSourceAddress = sourcecanAddress;
}

static void resetAuxControlStubs(void)
{
  analogReadValue = 0U;
  digitalReadValue = 0U;
  analogReadCount = 0U;
  digitalReadCount = 0U;
  lastAnalogPin = 0U;
  lastDigitalPin = 0U;
  canCommandCount = 0U;
  lastCanCommandType = 0U;
  lastCanAddress = 0U;
  lastCanData1 = 0U;
  lastCanData2 = 0U;
  lastCanSourceAddress = 0U;
}

static void test_auxChannelControl_can_input(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();

  page9.enable_secondarySerial = 1U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 1U;
  page9.caninput_sel[3] = 4U;
  page9.caninput_source_can_address[3] = 0x345U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(1U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(2U, lastCanCommandType);
  TEST_ASSERT_EQUAL_UINT16(0U, lastCanAddress);
  TEST_ASSERT_EQUAL_UINT8(3U, lastCanData1);
  TEST_ASSERT_EQUAL_UINT8(0U, lastCanData2);
  TEST_ASSERT_EQUAL_UINT16(0x445U, lastCanSourceAddress);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT8(15U, current.current_caninchannel);
}

static void test_auxChannelControl_can_input_secondary_serial_variants(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();
  page9.enable_secondarySerial = 1U;
  page9.enable_intcan = 1U;
  page9.caninput_source_can_address[1] = 0x456U;
  page9.caninput_source_can_address[2] = 0x567U;
  page9.caninput_sel[1] = 4U;
  page9.caninput_sel[2] = 4U;
  page9.intcan_available = 1U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(2U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(2U, lastCanCommandType);
  TEST_ASSERT_EQUAL_UINT8(2U, lastCanData1);
  TEST_ASSERT_EQUAL_UINT16(0x667U, lastCanSourceAddress);

  resetAuxControlStubs();
  page9.intcan_available = 0U;
  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(2U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(2U, lastCanCommandType);
  TEST_ASSERT_EQUAL_UINT8(2U, lastCanData1);
  TEST_ASSERT_EQUAL_UINT16(0x667U, lastCanSourceAddress);
}

static void test_auxChannelControl_internal_can_selection(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();

  page9.enable_secondarySerial = 1U;
  page9.enable_intcan = 1U;
  page9.intcan_available = 1U;
  page9.caninput_sel[4] = 4U | 64U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);

  resetAuxControlStubs();
  page9.enable_secondarySerial = 0U;
  page9.caninput_sel[5] = 4U | 128U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);

  page9 = config9{};
  resetAuxControlStubs();
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 1U;
  page9.intcan_available = 1U;
  page9.caninput_sel[5] = 4U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);

  page9 = config9{};
  resetAuxControlStubs();
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 0U;
  page9.intcan_available = 1U;
  page9.caninput_sel[6] = 4U | 128U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);

  page9 = config9{};
  resetAuxControlStubs();
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 1U;
  page9.intcan_available = 0U;
  page9.caninput_sel[7] = 4U | 128U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);
}

static void test_auxChannelControl_analog_input(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();
  analogReadValue = 321U;

  page9.caninput_sel[5] = 2U;
  page9.Auxinpina[5] = 7U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(1U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(pinTranslateAnalog(7U), lastAnalogPin);
  TEST_ASSERT_EQUAL_UINT16(321U, current.canin[5]);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
}

static void test_auxChannelControl_analog_input_variants(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();
  analogReadValue = 111U;
  page9.enable_secondarySerial = 1U;
  page9.caninput_sel[6] = 8U;
  page9.Auxinpina[6] = 3U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);
  TEST_ASSERT_EQUAL_UINT8(1U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT16(111U, current.canin[6]);

  resetAuxControlStubs();
  analogReadValue = 222U;
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 1U;
  page9.intcan_available = 1U;
  page9.caninput_sel[6] = 8U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);
  TEST_ASSERT_EQUAL_UINT8(1U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT16(222U, current.canin[6]);

  resetAuxControlStubs();
  analogReadValue = 333U;
  page9.intcan_available = 0U;
  page9.caninput_sel[6] = 2U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);
  TEST_ASSERT_EQUAL_UINT8(1U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT16(333U, current.canin[6]);
}

static void test_auxChannelControl_digital_input(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();
  digitalReadValue = 654U;

  page9.caninput_sel[9] = 3U;
  page9.Auxinpinb[9] = 11U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(1U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT8(12U, lastDigitalPin);
  TEST_ASSERT_EQUAL_UINT16(654U, current.canin[9]);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
}

static void test_auxChannelControl_digital_input_variants(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();
  digitalReadValue = 444U;
  page9.enable_secondarySerial = 1U;
  page9.caninput_sel[7] = 12U;
  page9.Auxinpinb[7] = 4U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);
  TEST_ASSERT_EQUAL_UINT8(1U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT16(444U, current.canin[7]);

  resetAuxControlStubs();
  digitalReadValue = 555U;
  page9.enable_secondarySerial = 0U;
  page9.enable_intcan = 1U;
  page9.intcan_available = 1U;
  page9.caninput_sel[7] = 12U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);
  TEST_ASSERT_EQUAL_UINT8(1U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT16(555U, current.canin[7]);

  resetAuxControlStubs();
  digitalReadValue = 666U;
  page9.intcan_available = 0U;
  page9.caninput_sel[7] = 3U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);
  TEST_ASSERT_EQUAL_UINT8(1U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT16(666U, current.canin[7]);
}

static void test_auxChannelControl_disabled_input(void)
{
  statuses current{};
  config9 page9{};
  resetAuxControlStubs();
  current.canin[2] = 77U;

  auxChannelControl(current, page9, stubSendCanCommand, stubReadAuxanalog, stubReadAuxdigital);

  TEST_ASSERT_EQUAL_UINT8(0U, canCommandCount);
  TEST_ASSERT_EQUAL_UINT8(0U, analogReadCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digitalReadCount);
  TEST_ASSERT_EQUAL_UINT16(77U, current.canin[2]);
  TEST_ASSERT_EQUAL_UINT8(15U, current.current_caninchannel);
}

void testAuxControl(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_auxChannelControl_can_input);
    RUN_TEST_P(test_auxChannelControl_can_input_secondary_serial_variants);
    RUN_TEST_P(test_auxChannelControl_internal_can_selection);
    RUN_TEST_P(test_auxChannelControl_analog_input);
    RUN_TEST_P(test_auxChannelControl_analog_input_variants);
    RUN_TEST_P(test_auxChannelControl_digital_input);
    RUN_TEST_P(test_auxChannelControl_digital_input_variants);
    RUN_TEST_P(test_auxChannelControl_disabled_input);
  }
}