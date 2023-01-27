
#include <Arduino.h>
#include <unity.h>

#include "schedule_calcs.h"
#include "crankMaths.h"

#define _countof(x) (sizeof(x) / sizeof (x[0]))

constexpr uint16_t DWELL_TIME_MS = 4;
extern volatile uint16_t degreesPeruSx2048;

Schedule &testSchedule = ignitionSchedule2;
int &testIgnDegrees = channel2IgnDegrees;

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

void test_calc_ign_timeout(uint16_t crankAngle, uint32_t pending, uint32_t running)
{
    testSchedule.Status = PENDING;
    calculateIgnitionAngle2(dwellAngle);
    TEST_ASSERT_EQUAL(pending, calculateIgnitionNTimeout(testSchedule, ignition2StartAngle, testIgnDegrees, crankAngle));
    
    testSchedule.Status = RUNNING;
    TEST_ASSERT_EQUAL(running, calculateIgnitionNTimeout(testSchedule, ignition2StartAngle, testIgnDegrees, crankAngle));
}

void test_calc_ign_timeout(const int16_t (*pStart)[4], const int16_t (*pEnd)[4])
{
    int16_t local[4];
    while (pStart!=pEnd)
    {
        memcpy_P(local, pStart, sizeof(local));
        currentStatus.advance = local[0];
        test_calc_ign_timeout(local[1], local[2], local[3]);
        ++pStart;
    }
}

// Test name format:
// calc_ign_timeout_<channel offset>_<CRANK_ANGLE_MAX_IGN>

void test_calc_ign_timeout_0_360()
{
    setEngineSpeed(4000, 360);
    memset(&testSchedule, 0, sizeof(testSchedule));
    testIgnDegrees = 0;

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

    test_calc_ign_timeout(&test_results[0], &test_results[0]+_countof(test_results));
}

void test_calc_ign_timeout_72_360()
{
    setEngineSpeed(4000, 360);
    memset(&testSchedule, 0, sizeof(testSchedule));
    testIgnDegrees = 72;

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

    test_calc_ign_timeout(&test_results[0], &test_results[0]+_countof(test_results));
}


void test_calc_ign_timeout_144_360()
{
    setEngineSpeed(4000, 360);
    memset(&testSchedule, 0, sizeof(testSchedule));
    testIgnDegrees = 144;

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

    test_calc_ign_timeout(&test_results[0], &test_results[0]+_countof(test_results));
}


void test_calc_ign_timeout(void)
{
    RUN_TEST(test_calc_ign_timeout_0_360);
    RUN_TEST(test_calc_ign_timeout_72_360);
    RUN_TEST(test_calc_ign_timeout_144_360);
}