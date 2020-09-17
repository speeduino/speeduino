
#include <Arduino.h>
#include <unity.h>
#include <decoders.h>
#include <globals.h>

#include "test_decoders.h"
#include "missing_tooth/missing_tooth.h"
#include "dual_wheel/dual_wheel.h"
#include "basic_distributor/basic_distributor.h"

// This invokes the test methods for each folder of decoders as well as local tests to this file.
void testDecoders()
{
    // Local Tests
    
    // addToothLogEntry
    RUN_TEST(test_addToothLogEntry_toothLog);
    RUN_TEST(test_addToothLogEntry_compositeLog);

    // loggerPrimaryISR + loggerSecondaryISR
    RUN_TEST(test_loggerPrimaryISR);
    RUN_TEST(test_loggerSecondaryISR);

    // decoderstdGetRPM
    RUN_TEST(test_decoderStdGetRPM);

    // deocderSetFilter
    RUN_TEST(test_decoderSetFilter);

    // decoderCrankingGetRPM
    RUN_TEST(test_decoderCrankingGetRPM);

    // Downstream Folders 
    test_missingTooth();
    test_dualWheel();
    test_basicDistributor();
}

// - - TEST REGION addToothLogEntry - - 

// Internal Testing Use Only, Used to reset after each test to get a clean slate. 
static void test_internal_addToothLogEntryClean()
{
    // Clear our arrays
    for(int x = 0; x < TOOTH_LOG_BUFFER; x++)
    {
        toothHistory[x] = 0;
        compositeLogHistory[x] = 0;
    }
    // Zero the indexes
    toothHistoryIndex = 0;
    compositeLastToothTime = 0;
    toothHistorySerialIndex = 0;

    // Rest flags
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    currentStatus.toothLogEnabled = false;
    currentStatus.compositeLogEnabled = false;
}

// This tests the addToothLogEntry logic with configurations for logging the tooth log only
void test_addToothLogEntry_toothLog()
{
    test_internal_addToothLogEntryClean();
    
    // Set the flag, this will cause the method to bail before doing anything
    BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    loop_until_bit_is_set(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);

    // Setup to try to add to the toothLog
    currentStatus.toothLogEnabled = true;
    decoderAddToothLogEntry(900, TOOTH_CRANK);

    TEST_ASSERT_EQUAL(0, toothHistory[0]);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Clear the flag so we should actually do stuff now
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    loop_until_bit_is_clear(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);

    // For Testing We Need To Ensure That The Tooth Log Size > 3 otherwise our tets will not work.
    //    if the tooth log is smaller than this these tests will need to be refactored
    TEST_ASSERT_GREATER_OR_EQUAL(4, TOOTH_LOG_SIZE);
    TEST_ASSERT_GREATER_OR_EQUAL(4, TOOTH_LOG_BUFFER);

    // Ensure that with the BIT set correct and the status set to false we still dont increment
    currentStatus.toothLogEnabled = false;
    currentStatus.compositeLogEnabled = false;

    // Test that if both loggers are disabled we dont log anything
    decoderAddToothLogEntry(1000, TOOTH_CRANK);
    TEST_ASSERT_EQUAL(0, toothHistory[0]);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Now we can do some real writes
    currentStatus.toothLogEnabled = true;

    // Test that we dont log CAM teeth 
    decoderAddToothLogEntry(1005, TOOTH_CAM);
    TEST_ASSERT_EQUAL(0, toothHistory[0]);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Test Inital Value Is Logged 
    decoderAddToothLogEntry(1010, TOOTH_CRANK);
    TEST_ASSERT_EQUAL(1010, toothHistory[0]);
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);

    // Test Values are Incremented 
    decoderAddToothLogEntry(1020, TOOTH_CRANK);
    TEST_ASSERT_EQUAL(1020, toothHistory[1]);
    TEST_ASSERT_EQUAL(2, toothHistoryIndex);

    // Test We Dont Overflow The Array 
    for(int x = 0; x <= TOOTH_LOG_BUFFER + 1; x++)
    {
        decoderAddToothLogEntry(x * 10, TOOTH_CRANK);
    }
    TEST_ASSERT_EQUAL((TOOTH_LOG_BUFFER - 3) * 10, toothHistory[TOOTH_LOG_SIZE-1]);
}

// This tests the addToothLogEntry logic with configurations for logging the composite log only
void test_addToothLogEntry_compositeLog()
{
    test_internal_addToothLogEntryClean();

    // Set the flag, this will cause the method to bail before doing anything
    BIT_SET(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    loop_until_bit_is_set(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);

    // Setup to try to add to the toothLog
    currentStatus.compositeLogEnabled = true;
    decoderAddToothLogEntry(900, TOOTH_CRANK);

    TEST_ASSERT_EQUAL(0, toothHistory[0]);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Clear the flag so we should actually do stuff now
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    loop_until_bit_is_clear(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);

    // For Testing We Need To Ensure That The Tooth Log Size > 3 otherwise our tets will not work.
    //    if the tooth log is smaller than this these tests will need to be refactored
    TEST_ASSERT_GREATER_OR_EQUAL(4, TOOTH_LOG_SIZE);
    TEST_ASSERT_GREATER_OR_EQUAL(4, TOOTH_LOG_BUFFER);

    // Ensure that with the BIT set correct and the status set to false we still dont increment
    currentStatus.toothLogEnabled = false;
    currentStatus.compositeLogEnabled = false;

    // Test that if both loggers are disabled we dont log anything
    decoderAddToothLogEntry(1000, TOOTH_CRANK);
    TEST_ASSERT_EQUAL(0, toothHistory[0]);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Now we can do some real writes
    currentStatus.compositeLogEnabled = true;
    // TODO: Unsure how we can test the READ_PRI_TRIGGER() and READ_SEC_TRIGGER definitions. These probably should be 
    //    extended with some form of method depednacy injection as they are hardware based and we can by default use the hardware provider
    //    and during testing we can gather the info from a stub / mock. As board definitions change the providers can also change to normalize data
    //    into our processing and logic layers. 
    //
    //    but for now we will be ensuring that we have sync and checking if the cam tooth flag is set. 
    currentStatus.hasSync = true;

    // Test Inital Value Is Logged 
    decoderAddToothLogEntry(1010, TOOTH_CRANK);
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);
    TEST_ASSERT_NOT_EQUAL(0,compositeLastToothTime);

    TEST_ASSERT_TRUE(BIT_CHECK(compositeLogHistory[0], COMPOSITE_LOG_SYNC));
    TEST_ASSERT_FALSE(BIT_CHECK(compositeLogHistory[0], COMPOSITE_LOG_TRIG));

    // Test Secondary Value Is Logged And Bit Flags Are Set Properly
    currentStatus.hasSync = false;
    decoderAddToothLogEntry(1020, TOOTH_CAM);
    TEST_ASSERT_EQUAL(2, toothHistoryIndex);
    TEST_ASSERT_FALSE(BIT_CHECK(compositeLogHistory[1], COMPOSITE_LOG_SYNC));
    TEST_ASSERT_TRUE(BIT_CHECK(compositeLogHistory[1], COMPOSITE_LOG_TRIG));

    // Ensure we dont overflow the buffer
    for(int x = 0; x < TOOTH_LOG_BUFFER + 5; x++)
    {
        decoderAddToothLogEntry(x * 10, TOOTH_CRANK);
    }
}


// - - END TEST REGION addToothLogEntry - - 
// - - TEST REGION loggerPrimaryISR - - 

static int triggerHandlerCounter = 0;
static void test_internal_triggerHandlerMock() 
{
    // This is set in the trigger handler if it passes filtering its set to true.
    validTrigger = true;
    triggerHandlerCounter++;
}

// Tests the primary trigger 
// NOTE: As written these tests may fail if the arduino is hooked up to hardware that pulls the PRI / SEC triggers high.
//    We need to add some form of DI to the hardware reads for testing / mocking purposes TODO: this comment.
void test_loggerPrimaryISR()
{
    // Configure Global Variables For A Valid Trigger
    primaryTriggerEdge = FALLING;
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    toothHistoryIndex = 0;
    currentStatus.toothLogEnabled = false;
    currentStatus.compositeLogEnabled = false;
    
    triggerHandler = test_internal_triggerHandlerMock;
    triggerHandlerCounter = 0;

    // Test Valid Edge Invokes TriggerHandler
    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, triggerHandlerCounter);

    // Test Invalid Edge Does Not Invoke Trigger Handler
    primaryTriggerEdge = RISING;
    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, triggerHandlerCounter);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Test Invalid Edge + toothLogEnabled Does Not Invoke decoderAddToothLogEntry method.
    currentStatus.toothLogEnabled = true;
    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // Test Valid Edge Invokes The decoderAddToothLogEntry method
    primaryTriggerEdge = FALLING;
    currentStatus.toothLogEnabled = true;
    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);

    // Test Valid Edge + composite Logger invokes decoderAddToothLogEntry
    currentStatus.toothLogEnabled = false;
    currentStatus.compositeLogEnabled = true;
    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(2, toothHistoryIndex);

    // Test Invalid Edge + composite logger invokes decoderAddToothLogEntry
    primaryTriggerEdge = RISING;
    loggerPrimaryISR();
    TEST_ASSERT_EQUAL(3, toothHistoryIndex);
}

// Tests the secondary trigger 
// NOTE: As written these tests may fail if the arduino is hooked up to hardware that pulls the PRI / SEC triggers high.
//    We need to add some form of DI to the hardware reads for testing / mocking purposes TODO: this comment.
void test_loggerSecondaryISR()
{
    // Configure Global Variables For A Valid Trigger
    secondaryTriggerEdge = FALLING;
    BIT_CLEAR(currentStatus.status1, BIT_STATUS1_TOOTHLOG1READY);
    toothHistoryIndex = 0;
    currentStatus.toothLogEnabled = false;
    currentStatus.compositeLogEnabled = false;
    
    triggerSecondaryHandler = test_internal_triggerHandlerMock;
    triggerHandlerCounter = 0;

    // Test Valid Edge Invokes TriggerHandler
    loggerSecondaryISR();
    TEST_ASSERT_EQUAL(1, triggerHandlerCounter);

    // Test Invalid Edge Does Not Invoke Trigger Handler
    secondaryTriggerEdge = RISING;
    loggerSecondaryISR();
    TEST_ASSERT_EQUAL(1, triggerHandlerCounter);
    TEST_ASSERT_EQUAL(0, toothHistoryIndex);

    // With logging enabled
    currentStatus.compositeLogEnabled = true;
    loggerSecondaryISR();
    TEST_ASSERT_EQUAL(1, toothHistoryIndex);
    
    secondaryTriggerEdge = FALLING;
    loggerSecondaryISR();
    TEST_ASSERT_EQUAL(2, toothHistoryIndex);
}


// - - END TEST REGION loggerPrimaryISR - - 
// - - TEST REGION stdGetRPM - - 

void test_decoderStdGetRPM()
{
    // Known good testing values 
    unsigned long tot = 100000UL;
    unsigned long tomot = 90000UL;
    unsigned long resultingRPM = 6000;

    // If we dont have sync RPM is 0
    currentStatus.hasSync = false;
    TEST_ASSERT_EQUAL(0, decoderStdGetRPM(360));

    // If we are cranking and haev less than 1 revolution RPM is 0 
    currentStatus.hasSync = true;
    currentStatus.RPM = 50;
    currentStatus.crankRPM = 100;
    currentStatus.startRevolutions = 0;
    TEST_ASSERT_EQUAL(0, decoderStdGetRPM(360));

    // If we dont have tooth 1 times RPM is 0 
    toothOneTime = 0;
    toothOneMinusOneTime = 0;
    TEST_ASSERT_EQUAL(0, decoderStdGetRPM(360));

    // Get a real RPM value
    toothOneTime = tot;
    toothOneMinusOneTime = tomot;
    currentStatus.RPM = 500;
    TEST_ASSERT_EQUAL(resultingRPM, decoderStdGetRPM(360));

    // If RPM is above MAX_RPM return previous RPM
    toothOneTime = tot / 10;
    toothOneMinusOneTime = tomot / 10;
    currentStatus.RPM = 501;
    TEST_ASSERT_EQUAL(501, decoderStdGetRPM(360));

    // Get a real RPM value for 720 degree triggers (double our previous time)
    toothOneTime = tot;
    toothOneMinusOneTime = tomot;
    currentStatus.RPM = 500;
    TEST_ASSERT_EQUAL(resultingRPM * 2, decoderStdGetRPM(720));
}

// - - END TEST REGION stdGetRPM - - 
// - - TEST REGION decoderSetFilter - - 

void test_decoderSetFilter()
{
    unsigned long currentGap = 10000UL;
    unsigned long rough25percent = 2500UL;
    unsigned long rough50percent = 5000UL;
    unsigned long rough75percent = 7500UL;

    // Test with filtering off
    configPage4.triggerFilter = 0;
    decoderSetFilter(currentGap);
    TEST_ASSERT_EQUAL(0, triggerFilterTime);

    // Test with filtering at 25% or level 1
    configPage4.triggerFilter = 1;
    decoderSetFilter(currentGap);
    TEST_ASSERT_INT_WITHIN(100, rough25percent, triggerFilterTime);

    // Test with filtering at 50% or level 2 
    configPage4.triggerFilter = 2;
    decoderSetFilter(currentGap);
    TEST_ASSERT_INT_WITHIN(100, rough50percent, triggerFilterTime);

    // Test with filtering at 75% or level 3 
    configPage4.triggerFilter = 3;
    decoderSetFilter(currentGap);
    TEST_ASSERT_INT_WITHIN(100, rough75percent, triggerFilterTime);
}

// - - END TEST REGION decoderSetFilter - - 
// - - TEST REGION decoderCrankingGetRPM - - 

void test_decoderCrankingGetRPM() 
{ 
    int totalTeeth = 36;

    // If we have sync return 0 
    currentStatus.hasSync = true;
    TEST_ASSERT_EQUAL(0, decoderCrankingGetRPM(totalTeeth));

    // If we havent cranked passed our 'needed number of cycles to  start ignition stg cycles'
    currentStatus.hasSync = true;
    configPage4.StgCycles = 4;
    currentStatus.startRevolutions = 2;
    TEST_ASSERT_EQUAL(0, decoderCrankingGetRPM(totalTeeth));

    // If we dont have a valid tooth time return 0 
    toothLastToothTime = 0;
    toothLastMinusOneToothTime = 0;
    currentStatus.startRevolutions = 8;
    TEST_ASSERT_EQUAL(0, decoderCrankingGetRPM(totalTeeth));

    // Get a RPM
    toothLastToothTime = 1100;
    toothLastMinusOneToothTime = 100;
    TEST_ASSERT_EQUAL(1666, decoderCrankingGetRPM(totalTeeth));
}

// - - END TEST REGION decoderCrankingGetRPM - - 