#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"
#include "scheduler_fuel_controller.h"
#include "channel_test_helpers.h"
#include "units.h"
#include "type_traits.h"

extern bool isAnyFuelScheduleRunning(void);
extern uint16_t lookupInjectorAngle(const statuses &current);
extern table2D_u8_u16_4 injectorAngleTable;
extern void setFuelChannelSchedule(FuelSchedule &schedule, uint8_t channel, uint16_t crankAngle, byte injChannelMask, uint16_t injAngle, injectorAngleCalcCache *pCache);
extern table2D_u8_u8_4 PrimingPulseTable;
extern uint16_t setFuelChannelSchedules(uint16_t crankAngle, byte injChannelMask, uint16_t injAngle);
extern bool changeToFullSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus);
extern bool changeToSemiSequentialInjection(const config2 &page2, const decoder_status_t &decoderStatus);

static void set_all_schedules_off(void)
{
    RUNIF_INJCHANNEL1( { fuelSchedule1._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL2( { fuelSchedule2._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL3( { fuelSchedule3._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL4( { fuelSchedule4._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL5( { fuelSchedule5._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL6( { fuelSchedule6._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL7( { fuelSchedule7._status = ScheduleStatus::OFF; }, {});
    RUNIF_INJCHANNEL8( { fuelSchedule8._status = ScheduleStatus::OFF; }, {});
}

static void test_isAnyFuelScheduleRunning(void)
{
  set_all_schedules_off();
  TEST_ASSERT_FALSE(isAnyFuelScheduleRunning());

  set_all_schedules_off();
  RUNIF_INJCHANNEL1( { fuelSchedule1._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL1( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, {});
  
  set_all_schedules_off();
  RUNIF_INJCHANNEL2( { fuelSchedule2._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL2( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL3( { fuelSchedule3._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL3( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL4( { fuelSchedule4._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL4( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL5( { fuelSchedule5._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL5( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL6( { fuelSchedule6._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL6( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL7( { fuelSchedule7._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL7( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });

  set_all_schedules_off();
  RUNIF_INJCHANNEL8( { fuelSchedule8._status = ScheduleStatus::RUNNING; }, {});
  RUNIF_INJCHANNEL8( { TEST_ASSERT_TRUE(isAnyFuelScheduleRunning()); }, { TEST_ASSERT_FALSE(isAnyFuelScheduleRunning()); });
}

using raw_counter_t = type_traits::remove_reference<FuelSchedule::counter_t>::type;
using raw_compare_t = type_traits::remove_reference<FuelSchedule::compare_t>::type;

static void setup_setFuelChannelSchedule(FuelSchedule &schedule)
{
  CRANK_ANGLE_MAX_INJ = 720U;
  setAngleConverterRevolutionTime(1000000UL);

  schedule._counter = 5U;
  schedule.pw = 1000U;
  schedule.channelDegrees = 0U;
}

static void test_setFuelChannelSchedule_ignores_zero_pw(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);

  injectorAngleCalcCache cache = {};
  schedule.pw = 0;
  setFuelChannelSchedule(schedule, UINT8_C(1), 100U, (byte)(1U << INJ1_CMD_BIT), 180U, &cache);

  TEST_ASSERT_EQUAL(OFF, schedule._status);
  TEST_ASSERT_EQUAL(0U, schedule._duration);
  TEST_ASSERT_EQUAL(0U, schedule._compare);
}

static void test_setFuelChannelSchedule_ignores_zero_timeout(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);

  injectorAngleCalcCache cache = {};
  setFuelChannelSchedule(schedule, UINT8_C(1), 0U, (byte)(1U << INJ1_CMD_BIT), 0U, &cache);

  TEST_ASSERT_EQUAL(OFF, schedule._status);
  TEST_ASSERT_EQUAL(0U, schedule._duration);
  TEST_ASSERT_EQUAL(0U, schedule._compare);
}

static void test_setFuelChannelSchedule_ignores_disabled_channel(void)
{
  raw_counter_t counter = {0};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);

  injectorAngleCalcCache cache = {};
  setFuelChannelSchedule(schedule, UINT8_C(1), 0U, 0U, 180U, &cache);

  TEST_ASSERT_EQUAL(OFF, schedule._status);
  TEST_ASSERT_EQUAL(0U, schedule._duration);
  TEST_ASSERT_EQUAL(0U, schedule._compare);
}

static void test_setFuelChannelSchedule_starts_pending_when_enabled(void)
{
  raw_counter_t counter = {5U};
  raw_compare_t compare = {0};
  FuelSchedule schedule(counter, compare);
  setup_setFuelChannelSchedule(schedule);
  
  injectorAngleCalcCache cache = {};
  setFuelChannelSchedule(schedule, UINT8_C(1), 300U, (byte)(1U << INJ1_CMD_BIT), 355U, &cache);

  TEST_ASSERT_EQUAL(PENDING, schedule._status);
  TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(1000U), schedule._duration);
  TEST_ASSERT_GREATER_THAN(0U, schedule._compare);
}

static void test_lookupInjectorAngle_clamp_max_inj(void)
{
  statuses current = {};
  populate_2dtable(&injectorAngleTable, (uint16_t)720, RPM_COARSE.toRaw(1000));

  setRpm(current, 1000);
  CRANK_ANGLE_MAX_INJ = injectorAngleTable.values[0]/2U;

  TEST_ASSERT_LESS_THAN(injectorAngleTable.values[0], CRANK_ANGLE_MAX_INJ);
  TEST_ASSERT_EQUAL(CRANK_ANGLE_MAX_INJ, lookupInjectorAngle(current));
}

constexpr uint8_t PRIMING_PULSE_WIDTH = 5;

static void assert_isPriming(FuelSchedule &schedule, bool priming)
{
  if (priming)
  {
    TEST_ASSERT_EQUAL(PENDING, schedule._status);
    TEST_ASSERT_EQUAL(uS_TO_TIMER_COMPARE(PRIMING_PULSE_WIDTH*500U), schedule._duration);
  }
  else
  {
    TEST_ASSERT_EQUAL(OFF, schedule._status);
  }
}

static void test_beginInjectorPriming_floodclear(void)
{
  stopFuelSchedulers();

  for (uint8_t channel = 0; channel<INJ_CHANNELS; ++channel)
  {
    statuses current = {};
    config4 page4 = {};
    page4.floodClear = 90;
    current.TPS = page4.floodClear+1;
    set_all_schedules_off();

    current.numPrimaryInjOutputs = channel;
    beginInjectorPriming(current, page4);

    RUNIF_INJCHANNEL1( { assert_isPriming(fuelSchedule1, false); }, {});
    RUNIF_INJCHANNEL2( { assert_isPriming(fuelSchedule2, false); }, {});
    RUNIF_INJCHANNEL3( { assert_isPriming(fuelSchedule3, false); }, {});
    RUNIF_INJCHANNEL4( { assert_isPriming(fuelSchedule4, false); }, {});
    RUNIF_INJCHANNEL5( { assert_isPriming(fuelSchedule5, false); }, {});
    RUNIF_INJCHANNEL6( { assert_isPriming(fuelSchedule6, false); }, {});
    RUNIF_INJCHANNEL7( { assert_isPriming(fuelSchedule7, false); }, {});
    RUNIF_INJCHANNEL8( { assert_isPriming(fuelSchedule8, false); }, {});
  }
}

static void test_beginInjectorPriming(void)
{
  constexpr int16_t COOLANT_TEMP = 200;
  stopFuelSchedulers();
  populate_2dtable(&PrimingPulseTable, PRIMING_PULSE_WIDTH, temperatureAddOffset(COOLANT_TEMP));

  for (uint8_t channel = 0; channel<INJ_CHANNELS; ++channel)
  {
    statuses current = {};
    config4 page4 = {};
    page4.floodClear = 100;
    current.TPS = page4.floodClear/2;
    current.coolant = COOLANT_TEMP;
    set_all_schedules_off();

    current.numPrimaryInjOutputs = channel;
    beginInjectorPriming(current, page4);

    RUNIF_INJCHANNEL1( { assert_isPriming(fuelSchedule1, channel>=1); }, {});
    RUNIF_INJCHANNEL2( { assert_isPriming(fuelSchedule2, channel>=2); }, {});
    RUNIF_INJCHANNEL3( { assert_isPriming(fuelSchedule3, channel>=3); }, {});
    RUNIF_INJCHANNEL4( { assert_isPriming(fuelSchedule4, channel>=4); }, {});
    RUNIF_INJCHANNEL5( { assert_isPriming(fuelSchedule5, channel>=5); }, {});
    RUNIF_INJCHANNEL6( { assert_isPriming(fuelSchedule6, channel>=6); }, {});
    RUNIF_INJCHANNEL7( { assert_isPriming(fuelSchedule7, channel>=7); }, {});
    RUNIF_INJCHANNEL8( { assert_isPriming(fuelSchedule8, channel>=8); }, {});
  }
}

static void test_setFuelChannelSchedules_returnsInjectionAngle(void)
{
  TEST_ASSERT_EQUAL(355U, setFuelChannelSchedules(0, 0, 355));
}

static void setup_changeToFullSequentialInjection(config2 &page2, decoder_status_t &decoderStatus)
{
  page2.injLayout = INJ_SEQUENTIAL;
  decoderStatus.syncStatus=SyncStatus::Full;
  CRANK_ANGLE_MAX_INJ = 360U;
}

static void test_changeToFullSequentialInjection(void)
{
  config2 page2 = {};
  decoder_status_t decoderStatus = {};

  setup_changeToFullSequentialInjection(page2, decoderStatus);
  TEST_ASSERT_TRUE(changeToFullSequentialInjection(page2, decoderStatus));

  setup_changeToFullSequentialInjection(page2, decoderStatus);
  page2.injLayout = INJ_SEMISEQUENTIAL;
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(page2, decoderStatus));

  setup_changeToFullSequentialInjection(page2, decoderStatus);
  decoderStatus.syncStatus=SyncStatus::Partial;
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(page2, decoderStatus));

  setup_changeToFullSequentialInjection(page2, decoderStatus);
  CRANK_ANGLE_MAX_INJ = 720U;
  TEST_ASSERT_FALSE(changeToFullSequentialInjection(page2, decoderStatus));
}

static void setup_changeToSemiSequentialInjection(config2 &page2, decoder_status_t &decoderStatus)
{
  page2.injLayout = INJ_SEQUENTIAL;
  page2.nCylinders = 4;
  decoderStatus.syncStatus=SyncStatus::Partial;
  CRANK_ANGLE_MAX_INJ = 720U;
}

static void test_changeToSemiSequentialInjection(void)
{
  config2 page2 = {};
  decoder_status_t decoderStatus = {};

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  TEST_ASSERT_TRUE(changeToSemiSequentialInjection(page2, decoderStatus));

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  page2.nCylinders = 6;
  TEST_ASSERT_TRUE(changeToSemiSequentialInjection(page2, decoderStatus));

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  page2.nCylinders = 8;
  TEST_ASSERT_TRUE(changeToSemiSequentialInjection(page2, decoderStatus));

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  page2.nCylinders = 1;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(page2, decoderStatus));

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  page2.injLayout = INJ_SEMISEQUENTIAL;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(page2, decoderStatus));

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  decoderStatus.syncStatus=SyncStatus::Full;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(page2, decoderStatus));

  setup_changeToSemiSequentialInjection(page2, decoderStatus);
  CRANK_ANGLE_MAX_INJ = 360U;
  TEST_ASSERT_FALSE(changeToSemiSequentialInjection(page2, decoderStatus));
}

void test_fuel_controller(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_isAnyFuelScheduleRunning);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_zero_pw);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_disabled_channel);
    RUN_TEST_P(test_setFuelChannelSchedule_starts_pending_when_enabled);
    RUN_TEST_P(test_setFuelChannelSchedule_ignores_zero_timeout);
    RUN_TEST_P(test_lookupInjectorAngle_clamp_max_inj);
    RUN_TEST_P(test_beginInjectorPriming_floodclear);
    RUN_TEST_P(test_beginInjectorPriming);
    RUN_TEST_P(test_setFuelChannelSchedules_returnsInjectionAngle);
    RUN_TEST_P(test_changeToFullSequentialInjection);
    RUN_TEST_P(test_changeToSemiSequentialInjection);
  }
}
