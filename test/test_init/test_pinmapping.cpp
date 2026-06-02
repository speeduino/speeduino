#include <unity.h>
#include "../test_utils.h"
#include "src/pins/pinNumbers_t.h"

extern pinNumbers_t getDefaultPinMapping(void);
extern pinNumbers_t getPinMapping(uint8_t boardID);

static size_t count_instances(const uint8_t *pStart, const uint8_t *pEnd, uint8_t value)
{
  size_t count = 0;
  while (pStart != pEnd)
  {
    if (*pStart == value)
    {
      ++count;
    }
    ++pStart;
  }
  return count;
}

static void assert_unique(const uint8_t *pStart, const uint8_t *pEnd)
{
  for (const uint8_t *pPin = pStart; pPin != pEnd; ++pPin)
  {
    if (*pPin != NOT_A_PIN)
    {
      char szMessage[64];
      snprintf(szMessage, sizeof(szMessage), "Duplicate pin %d, offset %d", *pPin, pPin - pStart);
      TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, count_instances(pStart, pEnd, *pPin), szMessage);
    }
  }
}

static void assert_mandatory_pins(const pinNumbers_t &pins)
{
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinTachOut, "pinTachOut");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinFuelPump, "pinFuelPump");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinFan, "pinFan");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.coilPins[0], "Coil 1");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.injectorPins[0], "Injector 1");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinTrigger, "Primary trigger");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinTPS, "TPS");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinO2, "O2");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinMAP, "MAP");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinIAT, "IAT");
  TEST_ASSERT_NOT_EQUAL_UINT8_MESSAGE(NOT_A_PIN, pins.pinCLT, "CLT");
}

static pinNumbers_t testPins;
static void test_pinMapping(void)
{
  assert_mandatory_pins(testPins);
  // Baro pin is allowed to be the same as MAP, so we need to ignore that when checking for uniqueness
  if (testPins.pinBaro != NOT_A_PIN && testPins.pinBaro == testPins.pinMAP)
  {
    TEST_MESSAGE("Baro pin is the same as MAP pin, which is allowed. Ignoring for uniqueness check.");
    testPins.pinBaro = NOT_A_PIN; // Set baro pin to NOT_A_PIN for uniqueness check
  }
  // VSS pin is allowed to be the same as tertiary trigger, so we need to ignore that when checking for uniqueness
  if (testPins.pinVSS != NOT_A_PIN && testPins.pinVSS == testPins.pinTrigger3)
  {
    TEST_MESSAGE("VSS pin is the same as tertiary trigger pin, which is allowed. Ignoring for uniqueness check.");
    testPins.pinVSS = NOT_A_PIN; // Set VSS pin to NOT_A_PIN for uniqueness check
  }
  // Boost pin is allowed to be the same as idle2, so we need to ignore that when checking for uniqueness
  if (testPins.pinBoost != NOT_A_PIN && testPins.pinBoost == testPins.pinIdle2)
  {
    TEST_MESSAGE("Boost pin is the same as idle2 pin, which is allowed. Ignoring for uniqueness check.");
    testPins.pinBoost = NOT_A_PIN; // Set Boost pin to NOT_A_PIN for uniqueness check
  }
  const uint8_t *pStart = (const uint8_t *)&testPins;
  const uint8_t *pEnd = pStart + sizeof(testPins)/sizeof(uint8_t);
  assert_unique(pStart, pEnd);
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
      // Special case: pin 50 is repeated in the V04 shield mapping!
      if (boardId==3U)
      {
        pins.injectorPins[5] = NOT_A_PIN;
      }
      char szName[128];
      snprintf(szName, sizeof(szName), "test_pinMapping_%d", boardId);
      testPins = pins;
      UnityDefaultTestRun(test_pinMapping, szName, __LINE__);
    }
  }
}

static void test_defaultPinMapping(void)
{
    testPins = getDefaultPinMapping();
    test_pinMapping();
}

void testPinMapping()
{
  SET_UNITY_FILENAME() {
    test_pinMappings();
    RUN_TEST(test_defaultPinMapping);
  }
}