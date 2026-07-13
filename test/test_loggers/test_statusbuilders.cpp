#include <unity.h>
#include "logger.h"
#include "../test_utils.h"

// ============================ buildEngineStatus =============================

static void test_buildEngineStatus_all_clear(void)
{
  statuses s = {};
  TEST_ASSERT_EQUAL_UINT8(0U, buildEngineStatus(s));
}

static void test_buildEngineStatus_running_only(void)
{
  statuses s = {};
  s.rotationStatus  = EngineRotationStatus::Running;
  TEST_ASSERT_EQUAL_UINT8(0x01U, buildEngineStatus(s));
}

static void test_buildEngineStatus_cranking_only(void)
{
  statuses s = {};
  s.rotationStatus  = EngineRotationStatus::Cranking;
  TEST_ASSERT_EQUAL_UINT8(0x02U, buildEngineStatus(s));
}

static void test_buildEngineStatus_ase_only(void)
{
  statuses s = {};
  s.aseIsActive = true;
  TEST_ASSERT_EQUAL_UINT8(0x04U, buildEngineStatus(s));
}

static void test_buildEngineStatus_wue_only(void)
{
  statuses s = {};
  s.wueIsActive = true;
  TEST_ASSERT_EQUAL_UINT8(0x08U, buildEngineStatus(s));
}

static void test_buildEngineStatus_accel_only(void)
{
  statuses s = {};
  s.isAcceleratingTPS = true;
  TEST_ASSERT_EQUAL_UINT8(0x10U, buildEngineStatus(s));
}

static void test_buildEngineStatus_decel_only(void)
{
  statuses s = {};
  s.isDeceleratingTPS = true;
  TEST_ASSERT_EQUAL_UINT8(0x20U, buildEngineStatus(s));
}

static void test_buildEngineStatus_combined(void)
{
  statuses s = {};
  s.rotationStatus  = EngineRotationStatus::Running;
  s.wueIsActive = true;
  s.isAcceleratingTPS = true;
  TEST_ASSERT_EQUAL_UINT8(0x01U | 0x08U | 0x10U, buildEngineStatus(s));
}

// ============================ buildSdCardStatus =============================

static void test_buildSdCardStatus_all_clear(void)
{
  statuses s = {};
  TEST_ASSERT_EQUAL_UINT8(0U, buildSdCardStatus(s));
}

static void test_buildSdCardStatus_present_bit(void)
{
  statuses s = {};
  s.sdCardPresent = 1U;
  TEST_ASSERT_EQUAL_UINT8(0x01U, buildSdCardStatus(s));
}

static void test_buildSdCardStatus_type1_bit(void)
{
  statuses s = {};
  s.sdCardType = 1U;
  TEST_ASSERT_EQUAL_UINT8(0x02U, buildSdCardStatus(s));
}

static void test_buildSdCardStatus_ready_bit(void)
{
  statuses s = {};
  s.sdCardReady = 1U;
  TEST_ASSERT_EQUAL_UINT8(0x04U, buildSdCardStatus(s));
}

static void test_buildSdCardStatus_logging_bit(void)
{
  statuses s = {};
  s.sdCardLogging = 1U;
  TEST_ASSERT_EQUAL_UINT8(0x08U, buildSdCardStatus(s));
}

static void test_buildSdCardStatus_error_bit(void)
{
  statuses s = {};
  s.sdCardError = 1U;
  TEST_ASSERT_EQUAL_UINT8(0x10U, buildSdCardStatus(s));
}

void testStatusBuilders(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST(test_buildEngineStatus_all_clear);
    RUN_TEST(test_buildEngineStatus_running_only);
    RUN_TEST(test_buildEngineStatus_cranking_only);
    RUN_TEST(test_buildEngineStatus_ase_only);
    RUN_TEST(test_buildEngineStatus_wue_only);
    RUN_TEST(test_buildEngineStatus_accel_only);
    RUN_TEST(test_buildEngineStatus_decel_only);
    RUN_TEST(test_buildEngineStatus_combined);

    RUN_TEST(test_buildSdCardStatus_all_clear);
    RUN_TEST(test_buildSdCardStatus_present_bit);
    RUN_TEST(test_buildSdCardStatus_type1_bit);
    RUN_TEST(test_buildSdCardStatus_ready_bit);
    RUN_TEST(test_buildSdCardStatus_logging_bit);
    RUN_TEST(test_buildSdCardStatus_error_bit);
  }
}
