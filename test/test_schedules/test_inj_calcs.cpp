#include <Arduino.h>
#include <unity.h>
#include "test_calcs_common.h"
#include "schedule_calcs.h"
#include "crankMaths.h"

#define _countof(x) (sizeof(x) / sizeof (x[0]))

void test_calc_inj1_timeout(uint16_t pw, uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    uint16_t PWdivTimerPerDegree = div(pw, timePerDegree).quot;

    memset(&fuelSchedule1, 0, sizeof(fuelSchedule1));

    fuelSchedule1.Status = PENDING;
    uint16_t startAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, 0);
    TEST_ASSERT_EQUAL(pending, calculateInjector1Timeout(startAngle, crankAngle));
    
    fuelSchedule1.Status = RUNNING;
    startAngle = calculateInjectorStartAngle( PWdivTimerPerDegree, 0);
    TEST_ASSERT_EQUAL(running, calculateInjector1Timeout(startAngle, crankAngle));
}


void test_calc_injN_timeout(FuelSchedule &schedule, uint16_t pw, int channelDegrees, uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    uint16_t PWdivTimerPerDegree = div(pw, timePerDegree).quot;

    memset(&schedule, 0, sizeof(schedule));

    schedule.Status = PENDING;
    uint16_t startAngle = calculateInjectorStartAngle(PWdivTimerPerDegree, channelDegrees);
    TEST_ASSERT_EQUAL(pending, calculateInjectorNTimeout(schedule, channelDegrees, startAngle, crankAngle));
    
    schedule.Status = RUNNING;
    startAngle = calculateInjectorStartAngle( PWdivTimerPerDegree, channelDegrees);
    TEST_ASSERT_EQUAL(running, calculateInjectorNTimeout(schedule, channelDegrees, startAngle, crankAngle));
}


void test_calc_injN_timeout(int channelDegrees, uint16_t pw, uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    test_calc_injN_timeout(fuelSchedule2, pw, channelDegrees, crankAngle, pending, running);
    test_calc_injN_timeout(fuelSchedule3, pw, channelDegrees, crankAngle, pending, running);
    test_calc_injN_timeout(fuelSchedule4, pw, channelDegrees, crankAngle, pending, running);
    test_calc_injN_timeout(fuelSchedule5, pw, channelDegrees, crankAngle, pending, running);
    test_calc_injN_timeout(fuelSchedule6, pw, channelDegrees, crankAngle, pending, running);
    test_calc_injN_timeout(fuelSchedule7, pw, channelDegrees, crankAngle, pending, running);
    test_calc_injN_timeout(fuelSchedule8, pw, channelDegrees, crankAngle, pending, running);
}

void test_calc_inj_timeout(int channelDegrees, const int16_t (*pStart)[4], const int16_t (*pEnd)[4])
{
    int16_t local[4];
    while (pStart!=pEnd)
    {
        memcpy_P(local, pStart, sizeof(local));
        test_calc_injN_timeout(channelDegrees, local[0], local[1], local[2], local[3]);
        ++pStart;
    }
}

// Test name format:
// test_calc_inj_timeout_<channel offset>_<CRANK_ANGLE_MAX_IGN>

void test_calc_inj_timeout_0_360()
{
    setEngineSpeed(4000, 360);

    currentStatus.injAngle = 355;

    static const int16_t test_data[][4] PROGMEM = {
        // PW (uS), Crank (deg), Expected Pending, Expected Running
        { 3000, 0, 11562, 11562 },
        { 3000, 45, 9717, 9717 },
        { 3000, 90, 7872, 7872 },
        { 3000, 135, 6027, 6027 },
        { 3000, 180, 4182, 4182 },
        { 3000, 215, 2747, 2747 },
        { 3000, 270, 492, 492 },
        { 3000, 315, 0, 13407 },
        { 3000, 360, 0, 11562 },
        { 12000, 0, 2583, 2583 },
        { 13000, 45, 0, 14473 },
        { 14000, 90, 0, 11644 },
        { 15000, 135, 8815, 8815 },
        { 16000, 180, 5945, 5945 },
        { 17000, 215, 3526, 3526 },
        { 18000, 270, 246, 246 },
        { 19000, 315, 0, 12177 },
        { 20000, 360, 0, 9348 },
    };
    const int16_t (*pStart)[4] = &test_data[0];
    const int16_t (*pEnd)[4] = &test_data[0]+_countof(test_data);

    test_calc_inj_timeout(0, pStart, pEnd);

    // Separate test for fuel 1 - different code path, same results!
    int16_t local[4];
    while (pStart!=pEnd)
    {
        memcpy_P(local, pStart, sizeof(local));
        test_calc_inj1_timeout(local[0], local[1], local[2], local[3]);
        ++pStart;
    }
}

// 
void test_calc_inj_timeout(void)
{
    RUN_TEST(test_calc_inj_timeout_0_360);
}