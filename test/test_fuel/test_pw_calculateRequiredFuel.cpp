#include <unity.h>
#include "../test_utils.h"
#include "config_pages.h"
#include "statuses.h"
#include "decoders.h"

extern uint16_t calculateRequiredFuel(const config2 &page2, const decoder_status_t &decoderStatus);

static void test_calculateRequiredFuel_2stroke(void) {
  config2 page2 = {};
  page2.reqFuel = 11; // ms*10?
  page2.strokes = TWO_STROKE;

  decoder_status_t decoderStatus;

  page2.injLayout = INJ_PAIRED;
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, decoderStatus));
  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, decoderStatus));

  page2.injLayout = INJ_SEQUENTIAL;
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, decoderStatus));
  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, decoderStatus));
}

static void test_calculateRequiredFuel_4stroke(void) {
  config2 page2 = {};
  page2.reqFuel = 11; // ms*10?
  page2.strokes = FOUR_STROKE;
  page2.nCylinders = 4;
  
  decoder_status_t decoderStatus;

  page2.injLayout = INJ_PAIRED;
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, decoderStatus));
  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, decoderStatus));

  page2.injLayout = INJ_SEQUENTIAL;  
  decoderStatus.syncStatus = SyncStatus::Full;
  TEST_ASSERT_EQUAL(page2.reqFuel*100U, calculateRequiredFuel(page2, decoderStatus));
  decoderStatus.syncStatus = SyncStatus::Partial;
  TEST_ASSERT_EQUAL((page2.reqFuel*100U)/2U, calculateRequiredFuel(page2, decoderStatus));
}


void testCalculateRequiredFuel(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateRequiredFuel_2stroke);
    RUN_TEST_P(test_calculateRequiredFuel_4stroke);
  }
}