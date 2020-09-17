#include <decoders.h>
#include <globals.h>
#include <unity.h>
#include "missing_tooth_and_dual_wheel.h"
// This is a set of helper functions that test the functinoality of the dual wheel and missing tooth setups. 

// - - Configuration Of The Main Test Loop - - 

// Define the min / max total teeth
const byte TEETH_MINIMUM = 8;
const byte TEETH_MAXIMUM = 120; 

// Define the min / max total missing teeth
const byte MISSING_TEETH_MINIMUM = 0;
const byte MISSING_TEETH_MAXIMUM = 4; 

// Define the trigger angle min / max as well as the step
const int TRIGGER_ANGLE_MINIMUM = -360;
const int TRIGGER_ANGLE_MAXIMUM = 360;
const int TRIGGER_ANGLE_STEP = 45;

// Define the Ignition Advance 
const int IGNITION_ADVANCE = 10;

// Define pointers to our 8 channels so we can poke them in our for loop without a bunch of branching logic. Performance here is not a major issue. 
int *ignitionXEndAngles[] = {
    &ignition1EndAngle, 
    &ignition2EndAngle,
    &ignition3EndAngle,
    &ignition4EndAngle,
    &ignition5EndAngle,
    &ignition6EndAngle,
    &ignition7EndAngle,
    &ignition8EndAngle
};

// Define pointers to our 8 end teeth variables so we can poke them in our loops without bunches of branching logic.
uint16_t *ignitionXEndTooth[] = {
    &ignition1EndTooth,
    &ignition2EndTooth,
    &ignition3EndTooth,
    &ignition4EndTooth,
    &ignition5EndTooth,
    &ignition6EndTooth,
    &ignition7EndTooth,
    &ignition8EndTooth
};

// Define the list of ignition modes we want to loop through
// TODO: Do we want to test more ign modes here ?
int ignitionModes[] = {
    IGN_MODE_WASTED,
    IGN_MODE_SEQUENTIAL,
};

// This method sweeps through: 
//    -- All combinations of 16 -> 120 teeth 
//    -- All combinations of 1 -> 4 missing teeth
//    -- 45 Degree Increments of -360 -> 360 degrees of trigger angle
//    -- A Defined 10 degrees of ignition advance
//    -- Internally Calculates Ignition1 -> 8 End Angles
//    -- Tests Ignition Modes IGN_MODE_WASTED and IGN_MODE_SEQUENTIAL
//    -- Calculates the given tooth and compares to the ignitionXEndTooth global variable
//    -- If Assertion fails logs the condition of failure 
//
void test_shared_variableMissingToothPatterns(void (*triggerSetEndTeeth_TestMethod)(), bool isDualWheel) 
{
    
    // Define the number of ignition channels we are using
    // TODO: Lots of other tests fail when I try to bump the ignition channels up to 8. We will not be testing past 5 channels unitll IGN_CHANNELS is bumped up. Although the code should work up to 8 without changes. 
    // set global: IGN_CHANNELS to 8 

    // Sweep through the min -> max number of teeth
    for(int teeth = TEETH_MINIMUM; teeth < TEETH_MAXIMUM; teeth++)
    {
        // Sweep through the min -> max number of missing teeth
        for(int missingTeeth = MISSING_TEETH_MINIMUM; missingTeeth < MISSING_TEETH_MAXIMUM; missingTeeth++)
        {
            // Sweep through the ignition modes listed
            for(uint16_t ignitionModeLookup = 0; ignitionModeLookup < sizeof(ignitionModes)/sizeof(int); ignitionModeLookup++)
            {

                // Setup our global variables for triggers and compute crank angle
                test_shared_triggerSetup(teeth, missingTeeth, ignitionModes[ignitionModeLookup], IGN_CHANNELS);

                // Go through a set of offset trigger angles
                for(int triggerAngleOffset = TRIGGER_ANGLE_MINIMUM; triggerAngleOffset < TRIGGER_ANGLE_MAXIMUM; triggerAngleOffset += TRIGGER_ANGLE_STEP)
                {
                    // Set the trigger angle offset on the global configs
                    configPage4.triggerAngle = triggerAngleOffset;

                    // Invoke the method we are testing
                    triggerSetEndTeeth_TestMethod();

                    // assert each of our valid ignition channels match our expected output from the test file calculations
                    for(int ignitionChannel = 1; ignitionChannel <= IGN_CHANNELS; ignitionChannel++)
                    {
                        int arrNum = ignitionChannel - 1;
                        int knownGoodTooth = test_local_calculateEndTooth(*ignitionXEndAngles[arrNum], triggerAngleOffset, teeth, missingTeeth, isDualWheel);
                        TEST_ASSERT_EQUAL_MESSAGE(knownGoodTooth, *ignitionXEndTooth[arrNum], test_local_createMessage(ignitionChannel, teeth, missingTeeth, *ignitionXEndAngles[arrNum], triggerAngleOffset).c_str());
                    }
                }
            }
        }
    }
}

// This method configures global variables for the given configuration of teeth, missing teeth, ignitionMode, and number of ignition channels
// This method is not intended to be invoked from the PIO RUN_TEST method but invoked from indivual tests themselves.
void test_shared_triggerSetup(int teeth, int missingTeeth, int ignitionMode, int ignitionChannels)
{
    // Seed global variables with our current testing setup
    configPage4.triggerTeeth = teeth;
    configPage4.triggerMissingTeeth = missingTeeth;
    configPage4.TrigSpeed = CRANK_SPEED;
    configPage4.trigPatternSec = SEC_TRIGGER_SINGLE;

    // Internally calculate ignition step angles and assign them to the global variables
    int ignitionAngleStep = 0;
    if(ignitionMode == IGN_MODE_WASTED)
    {
        // In a wasted spark configuration we use a 360 degree calculation
        configPage4.sparkMode = IGN_MODE_WASTED;
        ignitionAngleStep =  360 / ignitionChannels;
    }
    if(ignitionMode == IGN_MODE_SEQUENTIAL)
    {
        // In a sequential spark configuration we use a 720 degree calculation as to not trigger the spark on exhaust stroke
        //    during the setup our angle step is effectivly doubled so we need to double it here as well. 
        configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
        ignitionAngleStep =  720 / ignitionChannels;
    }

    // Calculate Ignition Angles
    for(int x = 0; x < ignitionChannels; x++) 
    {
        int ignitionAngleX = 360 - (x * ignitionAngleStep) - IGNITION_ADVANCE;
        if(ignitionAngleX < 0)
        {
            // Loop ignition angles back around to the top if IGNITION_ADVANCE > IgnitionAngleStep
            if(ignitionMode == IGN_MODE_SEQUENTIAL)
            {
                ignitionAngleX += 360;
            }
            else if(ignitionMode == IGN_MODE_WASTED) 
            {
                ignitionAngleX += 720;
            }
        }

        // Set the ignition angle on the global variable through our lookup pointer map
        *ignitionXEndAngles[x] = ignitionAngleX;
    }
    
    // Invoke the external method to ensure the triggers are setup
    triggerSetup_missingTooth();
}



// This is a helper function in case one of the tests fail within the loop enough information about which part failed can be sent back to the testing library. 
// Its a bit odd because the string concat limitations of the arduino lib we also do some enum unwrapping.
static String test_local_createMessage(int channel, int teeth, int missingTeeth, int ignitionAngle, int triggerAngleOffset)
{
    String message = "Failed to assert ignition end tooth on channel ";
    message += channel;
    message += " with configuration: teeth:";
    message += teeth;
    message += " missingTeeth:";
    message += missingTeeth;
    message += " ignitionAngle:";
    message += ignitionAngle;
    message += " triggerAngleOffset";
    message += triggerAngleOffset;
    if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL) 
    {
        message += " in IGN_MODE_SEQUENTIAL";
    }
    if(configPage4.sparkMode == IGN_MODE_WASTED)
    {
        message += " in IGN_MODE_WASTED";
    }

    return message;
}

// This is a helper function to calculate the target tooth based on just the numbers we have here in the test file.         
static int test_local_calculateEndTooth(int ignitionAngle, int triggerOffset, int totalTeeth, int missingTeeth, bool isDualWheel)
{
    // This is an edge case: 
    // During SEQUENTIAL ignition when the trigger is on the CRANK the ignition angle offsets can be greather than 360 
    //    we want to calculate the end tooth for a full 720 degree rotation in the instance. In all other instances we will 
    //    be calculating them based on a 360 degree rotation. This is handled by a 'toothAdder'
    int toothAdder = 0;
    if(configPage4.sparkMode == IGN_MODE_SEQUENTIAL && configPage4.TrigSpeed == CRANK_SPEED) 
    {
        toothAdder = totalTeeth;
    }

    // We want to calculate the number of teeth to get to the specified angle minus 1 
    int tooth = ((ignitionAngle - triggerOffset) / (int16_t)(360 / totalTeeth)); 
    if (!isDualWheel)
    {
        // The only difference between triggerSetEndTeeth_missingTooth() and triggerSetEndTeeth_DualWheel() is that dual wheel does not subtract 1 and we dont check if its the missing tooth.
        tooth -= 1;
    }

    // Clamp to a single rotation with the exception of SEQ on CRANK then we clamp to 2 rotaions via the tooth adder
    if(tooth > totalTeeth + toothAdder)
    {
        tooth -= totalTeeth + toothAdder;
    }
    if(tooth <= 0)
    {
        tooth += totalTeeth + toothAdder;
    }

    // If we are triggering on a 'missing' tooth we need to go backwards to find the last real tooth.
    if(!isDualWheel && (tooth > (totalTeeth - missingTeeth) + toothAdder))
    {
        tooth = (totalTeeth - missingTeeth) + toothAdder;
    }

    // Return the tooth we found to assert.
    return tooth;
}