
#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "crankMaths.h"
#include "scheduler.h"
#include "scheduledIO.h"

#define TESTCRANKANGLE 26
#define DURATION 1000
#define DELTA 20

struct crankmaths_rev_testdata {
  uint16_t rpm;
  unsigned long revolutionTime;
  int16_t angle;
  unsigned long expected;  
}*duration_testdata_current;

static Schedule *targetSchedule;
static uint32_t start_time, end_time;
static void startCallback(void) {start_time = micros();}
static void endCallback(void) {end_time = micros();}

//test for ignition pulse end timing
void test_accuracy_duration_ign(void)
{
    crankmaths_rev_testdata *testdata = duration_testdata_current;
    revolutionTime = testdata->revolutionTime;
    initialiseSchedulers();
    targetSchedule->StartFunction=startCallback;
    targetSchedule->EndFunction =endCallback;
    start_time = micros();//reset start_time so that we do not get false positives from previous tests
    setIgnitionSchedule(targetSchedule, TESTCRANKANGLE, TESTCRANKANGLE+testdata->angle, DURATION);
    while(targetSchedule->Status == PENDING) /*Wait*/;
    //start_time is saved in the startCallback of the schedule under test
    while(targetSchedule->Status == RUNNING) /*Wait*/;
    //end_time is saved in the endCallback of the schedule under test
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}
//test for fuel injection pulse end timing
void test_accuracy_duration_inj(void)

{
    crankmaths_rev_testdata *testdata = duration_testdata_current;
    revolutionTime = testdata->revolutionTime;
    timePerDegree=revolutionTime/360; //fuelschedules use timePerDegree, lets see if it passes the accuracy test!
    initialiseSchedulers();
    targetSchedule->StartFunction=startCallback;
    targetSchedule->EndFunction =endCallback;
    start_time = micros();//reset start_time so that we do not get false positives from previous tests
    setFuelSchedule(targetSchedule, TESTCRANKANGLE, TESTCRANKANGLE+testdata->angle, DURATION);
    while(targetSchedule->Status == PENDING) /*Wait*/;
    while(targetSchedule->Status == RUNNING) /*Wait*/;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}


void test_accuracy_duration(void)
{
  const byte testNameLength = 200;
  char testName[testNameLength];
    uint8_t i;  

  const crankmaths_rev_testdata crankmaths_rev_testdatas[] = {
    { .rpm = 50,    .revolutionTime = 1200000, .angle = 1,   .expected = 3333 },//3333,3333
    { .rpm = 50,    .revolutionTime = 1200000, .angle = 25,  .expected = 83333 }, // 83333,3333
    { .rpm = 50,    .revolutionTime = 1200000, .angle = 75, .expected = 250000 },//max timing is 262140uS for schedules, longer times do not activate the scheduler!
    { .rpm = 2500,  .revolutionTime = 24000,   .angle = 0,   .expected = 48000 },
    { .rpm = 2500,  .revolutionTime = 24000,   .angle = 25,  .expected = 1666 }, // 1666,6666
    { .rpm = 2500,  .revolutionTime = 24000,   .angle = 720, .expected = 48000 },
    { .rpm = 20000, .revolutionTime = 3000,    .angle = 0,   .expected = 6000 },
    //{ .rpm = 20000, .revolutionTime = 3000,    .angle = 25,  .expected = 208 }, // 208,3333 //everything that is under DURATION+IGNITION_REFRESH_THRESHOLD will fail!
    { .rpm = 20000, .revolutionTime = 3000,    .angle = 180,  .expected = 1500 },
    { .rpm = 20000, .revolutionTime = 3000,    .angle = 720, .expected = 6000 }
  };
    CRANK_ANGLE_MAX_IGN=720;
    for(i=1;i<=IGN_CHANNELS;i++){
        switch(i){
            case 1: targetSchedule=&ignitionSchedule1;break;
            case 2: targetSchedule=&ignitionSchedule2;break;
            case 3: targetSchedule=&ignitionSchedule3;break;
            case 4: targetSchedule=&ignitionSchedule4;break;
            case 5: targetSchedule=&ignitionSchedule5;break;
            #if IGN_CHANNELS >= 6
            case 6: targetSchedule=&ignitionSchedule6;break;
            #if IGN_CHANNELS >= 7
            case 7: targetSchedule=&ignitionSchedule7;break;
            #if IGN_CHANNELS >= 8
            case 8: targetSchedule=&ignitionSchedule8;break;
            #endif
            #endif
            #endif
        }
        for (auto testdata : crankmaths_rev_testdatas) {
            duration_testdata_current = &testdata;
            snprintf(testName, testNameLength, "test_accuracy_duration_ign%u/%urpm/%uangle",i, testdata.rpm, testdata.angle);
            UnityDefaultTestRun(test_accuracy_duration_ign, testName, __LINE__);
        }
    }
    //fuel schedules testing loop
    for(i=1;i<=INJ_CHANNELS;i++){
        switch(i){
            case 1: targetSchedule=&fuelSchedule1;break;
            case 2: targetSchedule=&fuelSchedule2;break;
            case 3: targetSchedule=&fuelSchedule3;break;
            case 4: targetSchedule=&fuelSchedule4;break;
            #if (INJ_CHANNELS >= 5)
            case 5: targetSchedule=&fuelSchedule5;break;
            #if IGN_CHANNELS >= 6
            case 6: targetSchedule=&fuelSchedule6;break;
            #if IGN_CHANNELS >= 7
            case 7: targetSchedule=&fuelSchedule7;break;
            #if IGN_CHANNELS >= 8
            case 8: targetSchedule=&fuelSchedule8;break;
            #endif
            #endif
            #endif
            #endif
        }
        for (auto testdata : crankmaths_rev_testdatas) {
            duration_testdata_current = &testdata;
            snprintf(testName, testNameLength, "test_accuracy_duration_inj%u/%urpm/%uangle",i, testdata.rpm, testdata.angle);
            UnityDefaultTestRun(test_accuracy_duration_inj, testName, __LINE__);
        }
    }

}
