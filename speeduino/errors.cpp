/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
 * Sets the next available error
 * Returns the error number or 0 if there is no more room for errors
 */
#include "globals.h"
#include "errors.h"
#include "auxiliaries.h"

byte errorCount = 0;
byte errorCodes[4];

byte setError(byte errorID)
{
  if(errorCount < MAX_ERRORS)
  {
    errorCodes[errorCount] = errorID;
    errorCount++;
    if(errorCount == 1) { BIT_SET(currentStatus.spark, BIT_SPARK_ERROR); } //Enable the error indicator
  }
  return errorCount;
}

void clearError(byte errorID)
{
  byte clearedError = 255;

  if (errorID == errorCodes[0]) { clearedError = 0; }
  else if(errorID == errorCodes[1]) { clearedError = 1; }
  else if(errorID == errorCodes[2]) { clearedError = 2; }
  else if(errorID == errorCodes[3]) { clearedError = 3; }

  if(clearedError < MAX_ERRORS)
  {
    errorCodes[clearedError] = ERR_NONE;
    //Clear the required error and move any from above it 'down' in the error array
    for (byte x=clearedError; x < (errorCount-1); x++)
    {
      errorCodes[x] = errorCodes[x+1];
      errorCodes[x+1] = ERR_NONE;
    }

    errorCount--;
    if(errorCount == 0) { BIT_CLEAR(currentStatus.spark, BIT_SPARK_ERROR); } //Enable the error indicator
  }
}

byte getNextError(void)
{
  packedError currentError;
  byte currentErrorNum = 0;

  if(errorCount > 0)
  {
    //We alternate through the errors once per second
    currentErrorNum = currentStatus.secl % errorCount; //Which error number will be returned. This changes once per second. 

    currentError.errorNum = currentErrorNum;
    currentError.errorID = errorCodes[currentErrorNum];
  }
  else
  {
    currentError.errorNum = 0;
    currentError.errorID = 0;
  }


  return *(byte*)&currentError; //Ugly, but this forces the cast of the currentError struct to a byte.
}



// Check if we need to display the "check engine light" & reset the light if we don't
void CheckEngineLight (void)
{
  if(configPage9.celEnabled == true)
  {
    // Check Engine Light is enabled, need to turn off the light a few seconds after starting ECU, 
    // then check for a condition that would trigger the light being on and enable it if required
    // this does mean ever 255 seconds the light resets currently - need extra logic to enable after the first 4 seconds the light to latch on till engine reset
    if (configPage9.celLightOnStartUp == true && currentStatus.secl == 5 && BIT_CHECK (currentStatus.checkEngineLight,BIT_CEL_STARTUP) == true)
    {
      // turn off all lights
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_GENERAL);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TEMP);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_LOAD);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TBC1);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TBC2);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_BATTERY);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_O2);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_STARTUP); // only used to restrict access to this function on very first startup of the engine otherwise function called every loop
      CEL_OFF (); // turn the CEL pin off
    }

    if (configPage9.celCheckTemp == true)
    {
      if (currentStatus.cltADC == 0 || currentStatus.coolant < -20 || currentStatus.coolant > 120 ||
          currentStatus.iatADC == 0 || currentStatus.IAT < -20 || currentStatus.IAT > 80)       
      { BIT_SET(currentStatus.checkEngineLight, BIT_CEL_TEMP); }
    }
    else
    { BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TEMP); }

    if (configPage9.celCheckLoad == true)
    {
      // TPS set within readTPS function in sensors.ino
      if(mapErrorCount > 0)
      {
        mapErrorCount = 0;
        BIT_SET(currentStatus.checkEngineLight, BIT_CEL_LOAD);
      }
    }
    else
    { BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_LOAD); }

    if (configPage9.celCheckTBC1 == true)
    {
      BIT_SET(currentStatus.checkEngineLight, BIT_CEL_TBC1);
    }
    else
    { BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TBC1); }

    if (configPage9.celCheckTBC2 == true)
    {
      BIT_SET(currentStatus.checkEngineLight, BIT_CEL_TBC2);
    }
    else
    { BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TBC2); }


    // Battery - Not sure how to checks, perhaps see if we're outside of the range catered for by injector voltage correction?
    // Tuner studio has hard coded  batlow < 11.8 &  bathigh >  15 which is the initial version
    if (configPage9.celCheckBattery == true)
    {
      if(currentStatus.battery10 < 118 || currentStatus.battery10 > 150) // range defined in tuner studio as the levels to warn on 11.8v and 15v
      { BIT_SET(currentStatus.checkEngineLight, BIT_CEL_BATTERY); }          
    }
    else
    { BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_BATTERY); }

    // O2 - checks to be done in readO2 in sensors.ino - perhaps check if value outside of calibration range?
    if (configPage9.celCheckO2 == true)
    {
      if (currentStatus.O2ADC == 0 || currentStatus.O2ADC == 255)
      { BIT_SET(currentStatus.checkEngineLight, BIT_CEL_O2); }
    }
    else
    { BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_O2); }


    if (currentStatus.checkEngineLight != 0)
    {
      // at least one CEL light is set to set the main light
      BIT_SET(currentStatus.checkEngineLight, BIT_CEL_GENERAL);
      CEL_ON(); // turn check engine light pin on
    }

  }
  else
  { 
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_GENERAL);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TEMP);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_LOAD);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TBC1);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_TBC2);      
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_BATTERY);
      BIT_CLEAR(currentStatus.checkEngineLight, BIT_CEL_O2);
      CEL_OFF(); // turn check engine light pin off
  }
}