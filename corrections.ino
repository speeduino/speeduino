/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
The corrections functions in this file affect the fuel pulsewidth (Either increasing or decreasing)
based on factors other than the VE lookup.

These factors include temperature (Warmup Enrichment and After Start Enrichment), Acceleration/Decelleration, 
Flood clear mode etc.
*/
//************************************************************************************************************

#include "corrections.h"
#include "globals.h"

/*
correctionsTotal() calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
byte correctionsTotal()
{
  int sumCorrections = 100;
  byte result; //temporary variable to store the result of each corrections function
  
  //As the 'normal' case will be for each function to return 100, we only perform the division operation if the returned result is not equal to that
  currentStatus.wueCorrection = correctionWUE();
  //if (currentStatus.wueCorrection != 100) { sumCorrections = div((sumCorrections * currentStatus.wueCorrection), 100).quot; }
  if (currentStatus.wueCorrection != 100) { sumCorrections = divs100(sumCorrections * currentStatus.wueCorrection); }
  result = correctionASE();
  //if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  if (result != 100) { sumCorrections = divs100(sumCorrections * result); }
  result = correctionCranking();
  if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  currentStatus.TAEamount = correctionAccel();
  if (currentStatus.TAEamount != 100) { sumCorrections = div((sumCorrections * currentStatus.TAEamount), 100).quot; }
  result = correctionFloodClear();
  if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  currentStatus.egoCorrection = correctionsAFRClosedLoop();
  //if (currentStatus.egoCorrection != 100) { sumCorrections = div((sumCorrections * currentStatus.egoCorrection), 100).quot; }
  if (currentStatus.egoCorrection != 100) { sumCorrections = divs100(sumCorrections * currentStatus.egoCorrection); }
  currentStatus.batCorrection = correctionsBatVoltage();
  //if (currentStatus.batCorrection != 100) { sumCorrections = div((sumCorrections * currentStatus.batCorrection), 100).quot; }
  if (currentStatus.batCorrection != 100) { sumCorrections = divs100(sumCorrections * currentStatus.batCorrection); }
  currentStatus.iatCorrection = correctionsIATDensity();
  //if (currentStatus.iatCorrection != 100) { sumCorrections = div((sumCorrections * currentStatus.iatCorrection), 100).quot; }
  if (currentStatus.iatCorrection != 100) { sumCorrections = divs100(sumCorrections * currentStatus.iatCorrection); }
  currentStatus.launchCorrection = correctionsLaunch();
  if (currentStatus.launchCorrection != 100) { sumCorrections = div((sumCorrections * currentStatus.launchCorrection), 100).quot; }
  
  if(sumCorrections > 255) { sumCorrections = 255; } //This is the maximum allowable increase
  return (byte)sumCorrections;
}

/*
Warm Up Enrichment (WUE)
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
byte correctionWUE()
{
  //Possibly reduce the frequency this runs at (Costs about 50 loops per second)
  if (currentStatus.coolant > (WUETable.axisX[9] - CALIBRATION_TEMPERATURE_OFFSET)) { BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP); return 100; } //This prevents us doing the 2D lookup if we're already up to temp
  BIT_SET(currentStatus.engine, BIT_ENGINE_WARMUP);
  return table2D_getValue(&WUETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
}

/*
Cranking Enrichment
Additional fuel % to be added when the engine is cranking
*/
byte correctionCranking()
{
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { return 100 + configPage1.crankingPct; }
  else { return 100; }
}

/*
After Start Enrichment
This is a short period (Usually <20 seconds) immediately after the engine first fires (But not when cranking)
where an additional amount of fuel is added (Over and above the WUE amount)
*/
byte correctionASE()
{
  //Two checks are requiredL:
  //1) Is the negine run time less than the configured ase time
  //2) Make sure we're not still cranking
  if ( (currentStatus.runSecs < configPage1.aseCount) && !(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) )
  {
    BIT_SET(currentStatus.engine, BIT_ENGINE_ASE); //Mark ASE as active.
    return 100 + configPage1.asePct;
  }
  
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE); //Mark ASE as inactive.
  return 100;
}

/*
TPS based acceleration enrichment
Calculates the % change of the throttle over time (%/second) and performs a lookup based on this
When the enrichment is turned on, it runs at that amount for a fixed period of time (taeTime)
*/
byte correctionAccel()
{
  //First, check whether the accel. enrichment is already running
  if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) )
  {
    //If it is currently running, check whether it should still be running or whether it's reached it's end time
    if( currentLoopTime >= currentStatus.TAEEndTime )
    {
      //Time to turn enrichment off
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC);
      currentStatus.TAEamount = 0;
      return 100;
    }
    //Enrichment still needs to keep running. Simply return the total TAE amount
    return currentStatus.TAEamount; 
  }
  
  //Check for deceleration (Deceleration adjustment not yet supported)
  if (currentStatus.TPS < currentStatus.TPSlast) { return 100; }
  
  //If TAE isn't currently turned on, need to check whether it needs to be turned on
  int rateOfChange = ldiv(1000000, (currentStatus.TPS_time - currentStatus.TPSlast_time)).quot * (currentStatus.TPS - currentStatus.TPSlast); //This is the % per second that the TPS has moved
  //currentStatus.tpsDOT = divs10(rateOfChange); //The TAE bins are divided by 10 in order to allow them to be stored in a byte. 
  currentStatus.tpsDOT = rateOfChange / 10;
  
  if (rateOfChange > configPage1.tpsThresh)
  {
    BIT_SET(currentStatus.engine, BIT_ENGINE_ACC); //Mark accleration enrichment as active.
    currentStatus.TAEEndTime = micros() + ((unsigned long)configPage1.taeTime * 10000); //Set the time in the future where the enrichment will be turned off. taeTime is stored as mS / 10, so multiply it by 100 to get it in uS
    return 100 + table2D_getValue(&taeTable, currentStatus.tpsDOT);
  }
  
  //If we reach here then TAE is neither on, nor does it need to be turned on.
  return 100;
}

/*
Simple check to see whether we are cranking with the TPS above the flood clear threshold
This function always returns either 100 or 0
*/

byte correctionFloodClear()
{
  if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK))
  {
    //Engine is currently cranking, check what the TPS is
    if(currentStatus.TPS >= configPage2.floodClear)
    {
      //Engine is cranking and TPS is above threshold. Cut all fuel
      return 0;
    }
  }
  return 100;
}

/*
Battery Voltage correction
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
byte correctionsBatVoltage()
{
  if (currentStatus.battery10 > (injectorVCorrectionTable.axisX[5])) { return injectorVCorrectionTable.values[injectorVCorrectionTable.xSize-1]; } //This prevents us doing the 2D lookup if the voltage is above maximum 
  return table2D_getValue(&injectorVCorrectionTable, currentStatus.battery10);
}

/*
Simple temperature based corrections lookup based on the inlet air temperature. 
This corrects for changes in air density from movement of the temperature 
*/
byte correctionsIATDensity()
{
  if ( (currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET) > (IATDensityCorrectionTable.axisX[8])) { return IATDensityCorrectionTable.values[IATDensityCorrectionTable.xSize-1]; } //This prevents us doing the 2D lookup if the intake temp is above maximum 
  return table2D_getValue(&IATDensityCorrectionTable, currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); //currentStatus.IAT is the actual temperature, values in IATDensityCorrectionTable.axisX are temp+offset
}

/*
Launch control has a setting to increase the fuel load to assist in bringing up boost
This simple check applies the extra fuel if we're currently launching
*/
byte correctionsLaunch()
{
  if(configPage3.launchEnabled && currentStatus.launching) { return (100 + configPage3.lnchFuelAdd); }
  else { return 100; }
}

/*
Lookup the AFR target table and perform either a simple or PID adjustment based on this

Simple (Best suited to narrowband sensors):
If the O2 sensor reports that the mixture is lean/rich compared to the desired AFR target, it will make a 1% adjustment
It then waits <egoDelta> number of ignition events and compares O2 against the target table again. If it is still lean/rich then the adjustment is increased to 2%
This continues until either:
  a) the O2 reading flips from lean to rich, at which point the adjustment cycle starts again at 1% or
  b) the adjustment amount increases to <egoLimit> at which point it stays at this level until the O2 state (rich/lean) changes
 
PID (Best suited to wideband sensors):
 
*/
double PID_O2, PID_output, PID_AFRTarget;
PID egoPID(&PID_O2, &PID_output, &PID_AFRTarget, configPage3.egoKP, configPage3.egoKI, configPage3.egoKD, REVERSE); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

byte correctionsAFRClosedLoop()
{
  if( (configPage3.egoAlgorithm == 3) || (configPage3.egoType == 0)) { return 100; } //An egoAlgorithm value of 3 means NO CORRECTION, egoType of 0 means no O2 sensor
  
  //Check the ignition count to see whether the next step is required
  if( (ignitionCount & (configPage3.egoCount - 1)) == 1 ) //This is the equivalent of ( (ignitionCount % configPage3.egoCount) == 0 ) but without the expensive modulus operation. ie It results in True every <egoCount> ignition loops. Note that it only works for power of two vlaues for egoCount
  {
    //Check all other requirements for closed loop adjustments
    if( (currentStatus.coolant > configPage3.egoTemp) && (currentStatus.RPM > (unsigned int)(configPage3.egoRPM * 100)) && (currentStatus.TPS < configPage3.egoTPSMax) && (currentStatus.O2 < configPage3.ego_max) && (currentStatus.O2 > configPage3.ego_min) && (currentStatus.runSecs > configPage3.ego_sdelay) )
    {
      //Determine whether the Y axis of the AFR target table tshould be MAP (Speed-Density) or TPS (Alpha-N)
      byte yValue;
      if (configPage1.algorithm == 0) { yValue = currentStatus.MAP; }
      else  { yValue = currentStatus.TPS; }
      
      currentStatus.afrTarget = get3DTableValue(&afrTable, yValue, currentStatus.RPM); //Perform the target lookup
      
      //Check which algorithm is used, simple or PID
      if (configPage3.egoAlgorithm == 0)
      {
        //*************************************************************************************************************************************
        //Simple algorithm
        if(currentStatus.O2 > currentStatus.afrTarget)
        {
          //Running lean
          if(currentStatus.egoCorrection < (100 + configPage3.egoLimit) ) //Fueling adjustment must be at most the egoLimit amount (up or down)
          {
            if(currentStatus.egoCorrection >= 100) { return (currentStatus.egoCorrection + 1); } //Increase the fueling by 1%
            else { return 100; } //This means that the last reading had been rich, so simply return back to no adjustment (100%)
          }
          else { return currentStatus.egoCorrection; } //Means we're at the maximum adjustment amount, so simply return then again
        }
        else
          //Running Rich
          if(currentStatus.egoCorrection > (100 - configPage3.egoLimit) ) //Fueling adjustment must be at most the egoLimit amount (up or down)
          {
            if(currentStatus.egoCorrection <= 100) { return (currentStatus.egoCorrection - 1); } //Increase the fueling by 1%
            else { return 100; } //This means that the last reading had been lean, so simply return back to no adjustment (100%)
          }
          else { return currentStatus.egoCorrection; } //Means we're at the maximum adjustment amount, so simply return then again
      }
      else if(configPage3.egoAlgorithm == 2)
      {
        //*************************************************************************************************************************************
        //PID algorithm
        egoPID.SetOutputLimits((double)(-configPage3.egoLimit), (double)(configPage3.egoLimit)); //Set the limits again, just incase the user has changed them since the last loop. Note that these are sent to the PID library as (Eg:) -15 and +15
        egoPID.SetTunings(configPage3.egoKP, configPage3.egoKI, configPage3.egoKD); //Set the PID values again, just incase the user has changed them since the last loop
        PID_O2 = (double)(currentStatus.O2);
        PID_AFRTarget = (double)(currentStatus.afrTarget);
        
        egoPID.Compute();
        //currentStatus.egoCorrection = 100 + PID_output;
        return (100 + PID_output);
      }
      
    }
  }
  
  return 100; //Catch all (Includes when AFR target = current AFR
}
