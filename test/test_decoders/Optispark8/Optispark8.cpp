#include "decoders.h"
#include "decoder_init.h"
#include "globals.h"
#include "crankMaths.h"
#include "../test_utils.h"

extern void processOptispark8Edge(uint32_t now, bool pinHigh);

// Independent absolute crank-angle timestamps from the published Ardu-Stim stream.
static constexpr uint16_t edges[] = {86,100,176,180,266,290,356,360,446,480,536,540,626,670,716,720};
static uint32_t now;

static uint16_t gapBefore(uint8_t index)
{
  return index == 0U ? 86U : edges[index] - edges[index - 1U];
}

static decoder_t prepare(bool inverted = false)
{
  configPage4.TrigEdge = inverted;
  configPage4.triggerAngle = 0;
  configPage4.TrigSpeed = CRANK_SPEED; // Deliberately stale: this pattern must ignore it.
  currentStatus.syncLossCounter = 0;
  CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = 720;
  now = 1000;
  return triggerSetup_Optispark8();
}

static void feed(uint8_t index, uint32_t interval)
{
  now += interval;
  processOptispark8Edge(now, ((index & 1U) == 0U) != (configPage4.TrigEdge != 0U));
}

static void expectPhase(const decoder_t &decoder, uint8_t index)
{
  TEST_ASSERT_EQUAL(SyncStatus::Full, decoder.getStatus().syncStatus);
  TEST_ASSERT_EQUAL_INT16(edges[index] % 720U, decoder.pGetCrankAngle(now));
}

static void runCycles(const decoder_t &decoder, uint8_t cycles)
{
  for (uint16_t n = 0; n < cycles * 16U; ++n)
  {
    const uint8_t index = n % 16U;
    feed(index, gapBefore(index) * 100U);
    if (n >= 32U) { expectPhase(decoder, index); }
  }
}

static void test_startup_phase_rpm_polarity(void)
{
  for (const uint16_t rpm : {41U,80U,200U,1000U,6500U,18000U})
  {
    for (uint8_t polarity = 0; polarity < 2U; ++polarity)
    {
      for (uint8_t start = 0; start < 16U; ++start)
      {
        const auto decoder = prepare(polarity != 0U);
        for (uint8_t n = 0; n < 64U; ++n)
        {
          const uint8_t index = (start + n) % 16U;
          feed(index, (uint32_t)gapBefore(index) * 1000000U / (6UL * rpm));
          if (n >= 20U) { expectPhase(decoder, index); }
          if (decoder.getStatus().syncStatus == SyncStatus::Full)
          {
            expectPhase(decoder, index);
            TEST_ASSERT_UINT16_WITHIN(rpm / 100U + 1U, rpm, decoder.getRPM());
          }
        }
        TEST_ASSERT_EQUAL_UINT16(0, currentStatus.syncLossCounter);
      }
    }
  }
}

static void test_reverse_and_equal_width_rejected(void)
{
  for (uint8_t start = 0; start < 16U; ++start)
  {
    const auto decoder = prepare();
    for (uint8_t n = 0; n < 80U; ++n)
    {
      const uint8_t index = (start + 16U - n % 16U) % 16U;
      now += (uint32_t)gapBefore((index + 1U) % 16U) * 100U;
      processOptispark8Edge(now, (index & 1U) != 0U); // Reverse motion swaps edge polarity.
      TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
      TEST_ASSERT_EQUAL_UINT16(0, decoder.getRPM());
    }
  }
  const auto decoder = prepare();
  for (uint8_t n = 0; n < 80U; ++n)
  {
    feed(n % 16U, 4500U);
    TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
  }
}

static void test_missing_pulse_and_recovery(void)
{
  const auto decoder = prepare();
  runCycles(decoder, 3);
  for (uint8_t index = 0; index < 16U; ++index)
  {
    if (index == 4U || index == 5U) { now += (uint32_t)gapBefore(index) * 100U; }
    else { feed(index, (uint32_t)gapBefore(index) * 100U); }
    if (index == 6U)
    {
      TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
      TEST_ASSERT_EQUAL_UINT16(0, decoder.getRPM());
      TEST_ASSERT_EQUAL_UINT32(0, currentStatus.startRevolutions);
    }
  }
  TEST_ASSERT_EQUAL_UINT16(1, currentStatus.syncLossCounter);
  runCycles(decoder, 3);
}

static void test_extra_pulse_and_recovery(void)
{
  const auto decoder = prepare();
  runCycles(decoder, 3);
  // Last real edge was falling. Insert a short complete pulse into the next low interval.
  processOptispark8Edge(now + 100U, true);
  TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
  processOptispark8Edge(now + 200U, false);
  runCycles(decoder, 3);
  TEST_ASSERT_EQUAL_UINT16(1, currentStatus.syncLossCounter);
}

static void test_stop_reset_and_restart(void)
{
  const auto decoder = prepare();
  runCycles(decoder, 3);
  TEST_ASSERT_TRUE(decoder.isEngineRunning(now));
  TEST_ASSERT_FALSE(decoder.isEngineRunning(now + 500001UL));
  feed(0, 600000UL); // Timeout must clear old history even before main-loop reset.
  TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
  TEST_ASSERT_EQUAL_UINT16(0, decoder.getRPM());
  runCycles(decoder, 3);
  decoder.reset();
  TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
  TEST_ASSERT_EQUAL_INT16(0, decoder.pGetCrankAngle(now));
  TEST_ASSERT_EQUAL_UINT32(0, currentStatus.startRevolutions);
  runCycles(decoder, 3);
}

static void test_micros_wrap_and_zero_timestamp(void)
{
  const auto decoder = prepare();
  // Acquire phase first; the second cycle ends exactly at micros()==0.
  now = UINT32_MAX - 143999U;
  runCycles(decoder, 2);
  TEST_ASSERT_EQUAL_UINT32(0, now);
  expectPhase(decoder, 15);
  runCycles(decoder, 3);
  TEST_ASSERT_EQUAL_UINT16(0, currentStatus.syncLossCounter);
  TEST_ASSERT_UINT16_WITHIN(1, 1667, decoder.getRPM());
}

static void test_acceleration_and_jitter(void)
{
  const auto decoder = prepare();
  uint32_t usPerDegree = 2000U;
  for (uint16_t n = 0; n < 192U; ++n)
  {
    const uint8_t index = n % 16U;
    const uint32_t interval = gapBefore(index) * usPerDegree;
    feed(index, interval * ((n & 1U) != 0U ? 103U : 97U) / 100U);
    if (n >= 32U) { expectPhase(decoder, index); }
    usPerDegree = usPerDegree * 99U / 100U;
  }
  TEST_ASSERT_EQUAL_UINT16(0, currentStatus.syncLossCounter);
}

static void test_angle_offset_interpolation_and_ranges(void)
{
  const auto decoder = prepare();
  runCycles(decoder, 3);
  for (const int16_t offset : {-360,-90,0,90,360})
  {
    configPage4.triggerAngle = offset;
    for (uint8_t index = 0; index < 16U; ++index)
    {
      feed(index, (uint32_t)gapBefore(index) * 100U);
      for (const int16_t range : {360,720})
      {
        CRANK_ANGLE_MAX_IGN = CRANK_ANGLE_MAX_INJ = range;
        const int16_t expected = (edges[index] + offset + 720) % range;
        TEST_ASSERT_EQUAL_INT16(expected, decoder.pGetCrankAngle(now));
        TEST_ASSERT_EQUAL_INT16((expected + 2) % range, decoder.pGetCrankAngle(now + 200U));
        TEST_ASSERT_EQUAL_INT16(expected, decoder.pGetCrankAngle(now - 1U)); // ISR/caller race
      }
    }
  }
}

static void test_configuration_and_repeated_edge(void)
{
  prepare();
  pinNumbers.pinTrigger = 2;
  configPage2.perToothIgn = true;
  const auto decoder = buildDecoder(DECODER_OPTISPARK_8);
  TEST_ASSERT_FALSE(configPage2.perToothIgn);
  TEST_ASSERT_EQUAL_UINT8(CHANGE, decoder.primary.edge);
  TEST_ASSERT_EQUAL_UINT8(TRIGGER_EDGE_NONE, decoder.secondary.edge);
  TEST_ASSERT_TRUE(decoder.getFeatures().supportsSequential);
  TEST_ASSERT_FALSE(decoder.getFeatures().supportsPerToothIgnition);
  runCycles(decoder, 3);
  processOptispark8Edge(now + 100U, false); // Duplicate falling edge
  TEST_ASSERT_EQUAL(SyncStatus::None, decoder.getStatus().syncStatus);
  TEST_ASSERT_EQUAL_UINT16(1, currentStatus.syncLossCounter);
  runCycles(decoder, 3);
}

void testOptispark8(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_startup_phase_rpm_polarity);
    RUN_TEST_P(test_reverse_and_equal_width_rejected);
    RUN_TEST_P(test_missing_pulse_and_recovery);
    RUN_TEST_P(test_extra_pulse_and_recovery);
    RUN_TEST_P(test_stop_reset_and_restart);
    RUN_TEST_P(test_micros_wrap_and_zero_timestamp);
    RUN_TEST_P(test_acceleration_and_jitter);
    RUN_TEST_P(test_angle_offset_interpolation_and_ranges);
    RUN_TEST_P(test_configuration_and_repeated_edge);
  }
}
