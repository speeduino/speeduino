#include <Arduino.h>
#include <unity.h>

#include "schedule_calcs.h"
#include "crankMaths.h"

#define _countof(x) (sizeof(x) / sizeof (x[0]))

constexpr uint16_t DWELL_TIME_MS = 4;
extern volatile uint16_t degreesPeruSx2048;

static uint16_t dwellAngle;
static void setEngineSpeed(uint16_t rpm, int16_t max_ign) {
    timePerDegreex16 = ldiv( 2666656L, rpm).quot; //The use of a x16 value gives accuracy down to 0.1 of a degree and can provide noticeably better timing results on low resolution triggers
    timePerDegree = timePerDegreex16 / 16; 
    degreesPeruSx2048 = 2048 / timePerDegree;
    degreesPeruSx32768 = 524288 / timePerDegreex16;       
    revolutionTime =  (60*1000000) / rpm;
    CRANK_ANGLE_MAX_IGN = max_ign;
    dwellAngle = timeToAngle(DWELL_TIME_MS*1000, CRANKMATH_METHOD_INTERVAL_REV);
}

void test_calc_ign1_timeout(uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    memset(&ignitionSchedule1, 0, sizeof(ignitionSchedule1));

    ignitionSchedule1.Status = PENDING;
    calculateIgnitionAngle1(dwellAngle);
    TEST_ASSERT_EQUAL(pending, calculateIgnition1Timeout(crankAngle));
    
    ignitionSchedule1.Status = RUNNING;
    calculateIgnitionAngle1(dwellAngle);
    TEST_ASSERT_EQUAL(running, calculateIgnition1Timeout(crankAngle));    
}


void test_calc_ignN_timeout(Schedule &schedule, int channelDegrees, const int &startAngle, void (*pEndAngleCalc)(int), uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    memset(&schedule, 0, sizeof(schedule));

    schedule.Status = PENDING;
    pEndAngleCalc(dwellAngle);
    TEST_ASSERT_EQUAL(pending, calculateIgnitionNTimeout(schedule, startAngle, channelDegrees, crankAngle));
    
    schedule.Status = RUNNING;
    pEndAngleCalc(dwellAngle);
    TEST_ASSERT_EQUAL(running, calculateIgnitionNTimeout(schedule, startAngle, channelDegrees, crankAngle));
}

void test_calc_ign_timeout(int channelDegrees, uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    channel2IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule2, channelDegrees, ignition2StartAngle, &calculateIgnitionAngle2, crankAngle, pending, running);
    channel3IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule3, channelDegrees, ignition3StartAngle, &calculateIgnitionAngle3, crankAngle, pending, running);
    channel4IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule4, channelDegrees, ignition4StartAngle, &calculateIgnitionAngle4, crankAngle, pending, running);
    channel5IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule5, channelDegrees, ignition5StartAngle, &calculateIgnitionAngle5, crankAngle, pending, running);
    channel6IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule6, channelDegrees, ignition6StartAngle, &calculateIgnitionAngle6, crankAngle, pending, running);
    channel7IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule7, channelDegrees, ignition7StartAngle, &calculateIgnitionAngle7, crankAngle, pending, running);
    channel8IgnDegrees = channelDegrees;
    test_calc_ignN_timeout(ignitionSchedule8, channelDegrees, ignition8StartAngle, &calculateIgnitionAngle8, crankAngle, pending, running);
}

void test_calc_ign_timeout(int channelDegrees, const int16_t (*pStart)[4], const int16_t (*pEnd)[4])
{
    int16_t local[4];
    while (pStart!=pEnd)
    {
        memcpy_P(local, pStart, sizeof(local));
        currentStatus.advance = local[0];
        test_calc_ign_timeout(channelDegrees, local[1], local[2], local[3]);
        ++pStart;
    }
}

// Test name format:
// calc_ign_timeout_<channel offset>_<CRANK_ANGLE_MAX_IGN>

void test_calc_ign_timeout_0_360()
{
    setEngineSpeed(4000, 360);

    static const int16_t test_results[][4] PROGMEM = {
        // Advance, Crank, Expected Pending, Expected Running
        { -40, 0, 12666, 12666 },
        { -40, 45, 10791, 10791 },
        { -40, 90, 8916, 8916 },
        { -40, 135, 7041, 7041 },
        { -40, 180, 5166, 5166 },
        { -40, 215, 3708, 3708 },
        { -40, 270, 1416, 1416 },
        { -40, 315, 0, 14541 },
        { -40, 360, 0, 12666 },
        { 0, 0, 11000, 11000 },
        { 0, 45, 9125, 9125 },
        { 0, 90, 7250, 7250 },
        { 0, 135, 5375, 5375 },
        { 0, 180, 3500, 3500 },
        { 0, 215, 2041, 2041 },
        { 0, 270, 0, 14750 },
        { 0, 315, 0, 12875 },
        { 0, 360, 0, 11000 },
        { 40, 0, 9333, 9333 },
        { 40, 45, 7458, 7458 },
        { 40, 90, 5583, 5583 },
        { 40, 135, 3708, 3708 },
        { 40, 180, 1833, 1833 },
        { 40, 215, 375, 375 },
        { 40, 270, 0, 13083 },
        { 40, 315, 0, 11208 },
        { 40, 360, 0, 9333 },
    };
    const int16_t (*pStart)[4] = &test_results[0];
    const int16_t (*pEnd)[4] = &test_results[0]+_countof(test_results);

    test_calc_ign_timeout(0, pStart, pEnd);

    // Separate test for ign 0 - different code path, same results!
    channel1IgnDegrees = 0;
    int16_t local[4];
    while (pStart!=pEnd)
    {
        memcpy_P(local, pStart, sizeof(local));
        currentStatus.advance = local[0];
        test_calc_ign1_timeout(local[1], local[2], local[3]);
        ++pStart;
    }    

}

void test_calc_ign_timeout_72_360()
{
    setEngineSpeed(4000, 360);

    static const int16_t test_results[][4] PROGMEM = {
        // Advance, Crank, Expected Pending, Expected Running
        { -40, 0, 666, 666 },
        { -40, 45, 0, 13791 },
        { -40, 90, 11916, 11916 },
        { -40, 135, 10041, 10041 },
        { -40, 180, 8166, 8166 },
        { -40, 215, 6708, 6708 },
        { -40, 270, 4416, 4416 },
        { -40, 315, 2541, 2541 },
        { -40, 360, 666, 666 },
        { 0, 0, 0, 14000 },
        { 0, 45, 0, 12125 },
        { 0, 90, 10250, 10250 },
        { 0, 135, 8375, 8375 },
        { 0, 180, 6500, 6500 },
        { 0, 215, 5041, 5041 },
        { 0, 270, 2750, 2750 },
        { 0, 315, 875, 875 },
        { 0, 360, 0, 14000 },
        { 40, 0, 0, 12333 },
        { 40, 45, 0, 10458 },
        { 40, 90, 8583, 8583 },
        { 40, 135, 6708, 6708 },
        { 40, 180, 4833, 4833 },
        { 40, 215, 3375, 3375 },
        { 40, 270, 1083, 1083 },
        { 40, 315, 0, 14208 },
        { 40, 360, 0, 12333 },
    };

    test_calc_ign_timeout(72, &test_results[0], &test_results[0]+_countof(test_results));
}


void test_calc_ign_timeout_144_360()
{
    setEngineSpeed(4000, 360);

    static const int16_t test_results[][4] PROGMEM = {
        // Advance, Crank, Expected Pending, Expected Running
        { -40, 0, 3666, 3666 },
        { -40, 45, 1791, 1791 },
        { -40, 90, 0, 14916 },
        { -40, 135, 0, 13041 },
        { -40, 180, 11166, 11166 },
        { -40, 215, 9708, 9708 },
        { -40, 270, 7416, 7416 },
        { -40, 315, 5541, 5541 },
        { -40, 360, 3666, 3666 },
        { 0, 0, 2000, 2000 },
        { 0, 45, 125, 125 },
        { 0, 90, 0, 13250 },
        { 0, 135, 0, 11375 },
        { 0, 180, 9500, 9500 },
        { 0, 215, 8041, 8041 },
        { 0, 270, 5750, 5750 },
        { 0, 315, 3875, 3875 },
        { 0, 360, 2000, 2000 },
        { 40, 0, 333, 333 },
        { 40, 45, 0, 13458 },
        { 40, 90, 0, 11583 },
        { 40, 135, 0, 9708 },
        { 40, 180, 7833, 7833 },
        { 40, 215, 6375, 6375 },
        { 40, 270, 4083, 4083 },
        { 40, 315, 2208, 2208 },
        { 40, 360, 333, 333 },
    };

    test_calc_ign_timeout(144, &test_results[0], &test_results[0]+_countof(test_results));
}


void test_calc_ign_timeout_480_720()
{
    setEngineSpeed(4000, 720);

    static const int16_t test_results[][4] PROGMEM = {
        // Crank, Advance, Expected Pending, Expected Running
        { -40, 0, 17666, 17666 },
        { -40, 45, 15791, 15791 },
        { -40, 90, 13916, 13916 },
        { -40, 135, 12041, 12041 },
        { -40, 180, 10166, 10166 },
        { -40, 215, 8708, 8708 },
        { -40, 270, 6416, 6416 },
        { -40, 315, 4541, 4541 },
        { -40, 360, 2666, 2666 },
        { 0, 0, 16000, 16000 },
        { 0, 45, 14125, 14125 },
        { 0, 90, 12250, 12250 },
        { 0, 135, 10375, 10375 },
        { 0, 180, 8500, 8500 },
        { 0, 215, 7041, 7041 },
        { 0, 270, 4750, 4750 },
        { 0, 315, 2875, 2875 },
        { 0, 360, 1000, 1000 },
        { 40, 0, 14333, 14333 },
        { 40, 45, 12458, 12458 },
        { 40, 90, 10583, 10583 },
        { 40, 135, 8708, 8708 },
        { 40, 180, 6833, 6833 },
        { 40, 215, 5375, 5375 },
        { 40, 270, 3083, 3083 },
        { 40, 315, 1208, 1208 },
        { 40, 360, 0, 29333 },
    };

    test_calc_ign_timeout(480, &test_results[0], &test_results[0]+_countof(test_results));
}

void test_calc_ign_timeout_576_720()
{
    setEngineSpeed(4000, 720);

    static const int16_t test_results[][4] PROGMEM = {
        // Crank, Advance, Expected Pending, Expected Running
        { -40, 0, 21666, 21666 },
        { -40, 45, 19791, 19791 },
        { -40, 90, 17916, 17916 },
        { -40, 135, 16041, 16041 },
        { -40, 180, 14166, 14166 },
        { -40, 215, 12708, 12708 },
        { -40, 270, 10416, 10416 },
        { -40, 315, 8541, 8541 },
        { -40, 360, 6666, 6666 },
        { 0, 0, 20000, 20000 },
        { 0, 45, 18125, 18125 },
        { 0, 90, 16250, 16250 },
        { 0, 135, 14375, 14375 },
        { 0, 180, 12500, 12500 },
        { 0, 215, 11041, 11041 },
        { 0, 270, 8750, 8750 },
        { 0, 315, 6875, 6875 },
        { 0, 360, 5000, 5000 },
        { 40, 0, 18333, 18333 },
        { 40, 45, 16458, 16458 },
        { 40, 90, 14583, 14583 },
        { 40, 135, 12708, 12708 },
        { 40, 180, 10833, 10833 },
        { 40, 215, 9375, 9375 },
        { 40, 270, 7083, 7083 },
        { 40, 315, 5208, 5208 },
        { 40, 360, 3333, 3333 },
    };

    test_calc_ign_timeout(576, &test_results[0], &test_results[0]+_countof(test_results));
}


void test_calc_ign_timeout_99_720()
{
    setEngineSpeed(4000, 720);

    static const int16_t test_results[][4] PROGMEM = {
        // Crank, Advance, Expected Pending, Expected Running
        { -40, 0, 1791, 1791 },
        { -40, 45, 0, 29916 },
        { -40, 90, 0, 28041 },
        { -40, 135, 26166, 26166 },
        { -40, 180, 24291, 24291 },
        { -40, 215, 22833, 22833 },
        { -40, 270, 20541, 20541 },
        { -40, 315, 18666, 18666 },
        { -40, 360, 16791, 16791 },
        { 0, 0, 125, 125 },
        { 0, 45, 0, 28250 },
        { 0, 90, 0, 26375 },
        { 0, 135, 24500, 24500 },
        { 0, 180, 22625, 22625 },
        { 0, 215, 21166, 21166 },
        { 0, 270, 18875, 18875 },
        { 0, 315, 17000, 17000 },
        { 0, 360, 15125, 15125 },
        { 40, 0, 0, 28458 },
        { 40, 45, 0, 26583 },
        { 40, 90, 0, 24708 },
        { 40, 135, 22833, 22833 },
        { 40, 180, 20958, 20958 },
        { 40, 215, 19500, 19500 },
        { 40, 270, 17208, 17208 },
        { 40, 315, 15333, 15333 },
        { 40, 360, 13458, 13458 },
    };

    test_calc_ign_timeout(99, &test_results[0], &test_results[0]+_countof(test_results));
}


void test_calc_ign_timeout(void)
{
    RUN_TEST(test_calc_ign_timeout_0_360);
    RUN_TEST(test_calc_ign_timeout_72_360);
    RUN_TEST(test_calc_ign_timeout_144_360);
    RUN_TEST(test_calc_ign_timeout_480_720);
    RUN_TEST(test_calc_ign_timeout_576_720);
    RUN_TEST(test_calc_ign_timeout_99_720);
}