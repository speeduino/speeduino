#include <unity.h>
#include "../test_utils.h"
#include "src/pins/pinNumbers_t.h"

extern pinNumbers_t getDefaultPinMapping(void);
extern pinNumbers_t getPinMapping(uint8_t boardID);

static void assert_mandatory_pins(const pinNumbers_t &pins)
{
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinTachOut, "pinTachOut");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinFuelPump, "pinFuelPump");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinFan, "pinFan");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.coilPins[0], "Coil 1");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.injectorPins[0], "Injector 1");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.triggerPins.primary, "Primary trigger");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.sensors.TPS, "TPS");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.sensors.O2, "O2");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.sensors.MAP, "MAP");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.sensors.IAT, "IAT");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.sensors.CLT, "CLT");
}

static pinNumbers_t testPins;
static void test_pinMapping(void)
{
    assert_mandatory_pins(testPins);
}

static void test_pinMappings(void)
{
  auto defaultPins = getDefaultPinMapping();
  for (uint8_t boardId = 0; boardId < UINT8_MAX; ++boardId)
  {
    // Don't run the test if the pin mapping is the same as the default (as we have a separate test for that).
    auto pins = getPinMapping(boardId);
    if (memcmp(&pins, &defaultPins, sizeof(pinNumbers_t)) != 0)
    {
      char szName[128];
      snprintf(szName, sizeof(szName), "test_pinMapping_%d", boardId);
      testPins = pins;
      UnityDefaultTestRun(test_pinMapping, szName, __LINE__);
    }
  }
}

static void test_defaultPinMapping(void)
{
    assert_mandatory_pins(getDefaultPinMapping());
}

void testPinMapping()
{
  SET_UNITY_FILENAME() {
    test_pinMappings();
    RUN_TEST(test_defaultPinMapping);
  }
}