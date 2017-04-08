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

long PID_O2, PID_output, PID_AFRTarget;
PID egoPID(&PID_O2, &PID_output, &PID_AFRTarget, configPage3.egoKP, configPage3.egoKI, configPage3.egoKD, REVERSE); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

void initialiseCorrections()
{
  egoPID.SetMode(AUTOMATIC); //Turn O2 PID on
  currentStatus.flexIgnCorrection = 0;
}

/*
correctionsTotal() calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
byte correctionsFuel()
{
  unsigned long sumCorrections = 100;
  byte activeCorrections = 0;
  byte result; //temporary variable to store the result of each corrections function

  //The values returned by each of the correction functions are multipled together and then divided back to give a single 0-255 value.
  currentStatus.wueCorrection = correctionWUE();
  if (currentStatus.wueCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.wueCorrection); activeCorrections++; }

  result = correctionASE();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }

  result = correctionCranking();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; } // Need to check this to ensure that sumCorrections doesn't overflow. Can occur when the number of corrections is greater than 3 (Which is 100^4) as 100^5 can overflow

  currentStatus.TAEamount = correctionAccel();
  if (currentStatus.TAEamount != 100) { sumCorrections = (sumCorrections * currentStatus.TAEamount); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  result = correctionFloodClear();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.egoCorrection = correctionAFRClosedLoop();
  if (currentStatus.egoCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.egoCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.batCorrection = correctionBatVoltage();
  if (currentStatus.batCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.batCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.iatCorrection = correctionIATDensity();
  if (currentStatus.iatCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.iatCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.flexCorrection = correctionFlex();
  if (currentStatus.flexCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.flexCorrection); activeCorrections++; }
  if (activeCorrections == 3) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.launchCorrection = correctionLaunch();
  if (currentStatus.launchCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.launchCorrection); activeCorrections++; }

  bitWrite(currentStatus.squirt, BIT_SQUIRT_DFCO, correctionDFCO());
  if ( bitRead(currentStatus.squirt, BIT_SQUIRT_DFCO) ) { sumCorrections = 0; }

  sumCorrections = sumCorrections / powint(100,activeCorrections);

  if(sumCorrections > 255) { sumCorrections = 255; } //This is the maximum allowable increase
  return (byte)sumCorrections;
}

/*
Warm Up Enrichment (WUE)
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
static inline byte correctionWUE()
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
static inline byte correctionCranking()
{
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { return 100 + configPage1.crankingPct; }
  else { return 100; }
}

/*
After Start Enrichment
This is a short period (Usually <20 seconds) immediately after the engine first fires (But not when cranking)
where an additional amount of fuel is added (Over and above the WUE amount)
*/
static inline byte correctionASE()
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
static inline byte correctionAccel()
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

static inline byte correctionFloodClear()
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
static inline byte correctionBatVoltage()
{
  if (currentStatus.battery10 > (injectorVCorrectionTable.axisX[5])) { return injectorVCorrectionTable.values[injectorVCorrectionTable.xSize-1]; } //This prevents us doing the 2D lookup if the voltage is above maximum
  return table2D_getValue(&injectorVCorrectionTable, currentStatus.battery10);
}

/*
Simple temperature based corrections lookup based on the inlet air temperature.
This corrects for changes in air density from movement of the temperature
*/
static inline byte correctionIATDensity()
{
  if ( (currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET) > (IATDensityCorrectionTable.axisX[8])) { return IATDensityCorrectionTable.values[IATDensityCorrectionTable.xSize-1]; } //This prevents us doing the 2D lookup if the intake temp is above maximum
  return table2D_getValue(&IATDensityCorrectionTable, currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); //currentStatus.IAT is the actual temperature, values in IATDensityCorrectionTable.axisX are temp+offset
}

/*
Launch control has a setting to increase the fuel load to assist in bringing up boost
This simple check applies the extra fuel if we're currently launching
*/
static inline byte correctionLaunch()
{
  if(currentStatus.launchingHard || currentStatus.launchingSoft) { return (100 + configPage3.lnchFuelAdd); }
  else { return 100; }
}

/*
 * Returns true if decelleration fuel cutoff should be on, false if its off
 */
static inline bool correctionDFCO()
{
  if ( !configPage2.dfcoEnabled ) { return false; } //If the DFCO option isn't turned on, always return false (off)
  if ( bitRead(currentStatus.squirt, BIT_SQUIRT_DFCO) ) { return ( currentStatus.RPM > ( configPage2.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage2.dfcoTPSThresh ); }
  else { return ( currentStatus.RPM > (unsigned int)( (configPage2.dfcoRPM * 10) + configPage2.dfcoHyster) ) && ( currentStatus.TPS < configPage2.dfcoTPSThresh ); }
}

/*
 * Flex fuel adjustment to vary fuel based on ethanol content
 * The amount of extra fuel required is a linear relationship based on the % of ethanol.
*/
static inline byte correctionFlex()
{
  if(!configPage1.flexEnabled) { return 100; } //Check for flex being enabled
  byte flexRange = configPage1.flexFuelHigh - configPage1.flexFuelLow;
  return percentage(currentStatus.ethanolPct, flexRange) + 100;
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

static inline byte correctionAFRClosedLoop()
{
  if( (configPage3.egoType == 0)) { return 100; } //egoType of 0 means no O2 sensor

  currentStatus.afrTarget = currentStatus.O2; //Catch all incase the below doesn't run. This prevents the Include AFR option from doing crazy things if the AFR target conditions aren't met. This value is changed again below if all conditions are met.

  //Check the ignition count to see whether the next step is required
  //if( (ignitionCount & (configPage3.egoCount - 1)) == 1 ) //This is the equivalent of ( (ignitionCount % configPage3.egoCount) == 0 ) but without the expensive modulus operation. ie It results in True every <egoCount> ignition loops. Note that it only works for power of two vlaues for egoCount
  {
    //Determine whether the Y axis of the AFR target table tshould be MAP (Speed-Density) or TPS (Alpha-N)
    byte yValue;
    if (configPage1.algorithm == 0) { yValue = currentStatus.MAP; }
    else  { yValue = currentStatus.TPS; }
    currentStatus.afrTarget = get3DTableValue(&afrTable, yValue, currentStatus.RPM); //Perform the target lookup

    //Check all other requirements for closed loop adjustments
    if( (currentStatus.coolant > (int)(configPage3.egoTemp - CALIBRATION_TEMPERATURE_OFFSET)) && (currentStatus.RPM > (unsigned int)(configPage3.egoRPM * 100)) && (currentStatus.TPS < configPage3.egoTPSMax) && (currentStatus.O2 < configPage3.ego_max) && (currentStatus.O2 > configPage3.ego_min) && (currentStatus.runSecs > configPage3.ego_sdelay) )
    {
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
        egoPID.SetOutputLimits((long)(-configPage3.egoLimit), (long)(configPage3.egoLimit)); //Set the limits again, just incase the user has changed them since the last loop. Note that these are sent to the PID library as (Eg:) -15 and +15
        egoPID.SetTunings(configPage3.egoKP, configPage3.egoKI, configPage3.egoKD); //Set the PID values again, just incase the user has changed them since the last loop
        PID_O2 = (long)(currentStatus.O2);
        PID_AFRTarget = (long)(currentStatus.afrTarget);

        egoPID.Compute();
        //currentStatus.egoCorrection = 100 + PID_output;
        return (100 + PID_output);
      }
      else { return 100; } // Occurs if the egoAlgorithm is set to 0 (No Correction)

    }
  }

  return 100; //Catch all (Includes when AFR target = current AFR
}

//******************************** IGNITION ADVANCE CORRECTIONS ********************************

int8_t correctionsIgn(int8_t advance)
{

  advance = correctionFlexTiming(advance);
  advance = correctionIATretard(advance);
  advance = correctionSoftRevLimit(advance);
  advance = correctionSoftLaunch(advance);
  advance = correctionSoftFlatShift(advance);
  //Fixed timing check must go last
  advance = correctionFixedTiming(advance);
  advance = correctionCrankingFixedTiming(advance); //This overrrides the regular fixed timing, must come last

  return advance;
}

static inline int8_t correctionFixedTiming(int8_t advance)
{
  if (configPage2.FixAng != 0) { return configPage2.FixAng; } //Check whether the user has set a fixed timing angle
  return advance;
}

static inline int8_t correctionCrankingFixedTiming(int8_t advance)
{
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { return configPage2.CrankAng; } //Use the fixed cranking ignition angle
  return advance;
}

static inline int8_t correctionFlexTiming(int8_t advance)
{
  if(!configPage1.flexEnabled) { return advance; } //Check for flex being enabled
  byte flexRange = configPage1.flexAdvHigh - configPage1.flexAdvLow;

  if (currentStatus.ethanolPct != 0) { currentStatus.flexIgnCorrection = percentage(currentStatus.ethanolPct, flexRange); }
  else { currentStatus.flexIgnCorrection = 0; }

  return advance + currentStatus.flexIgnCorrection;
}

static inline int8_t correctionIATretard(int8_t advance)
{
  //Adjust the advance based on IAT. If the adjustment amount is greater than the current advance, just set advance to 0
  byte advanceIATadjust = table2D_getValue(&IATRetardTable, currentStatus.IAT);
  if (advanceIATadjust <= advance) { return (advance - advanceIATadjust); }
  else { return 0; }
}

static inline int8_t correctionSoftRevLimit(int8_t advance)
{
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_SFTLIM);
  if (currentStatus.RPM > ((unsigned int)(configPage2.SoftRevLim) * 100) ) { BIT_SET(currentStatus.spark, BIT_SPARK_SFTLIM); return configPage2.SoftLimRetard;  } //Softcut RPM limit (If we're above softcut limit, delay timing by configured number of degrees)
  return advance;
}

static inline int8_t correctionSoftLaunch(int8_t advance)
{
  //SoftCut rev limit for 2-step launch control.
  if (configPage3.launchEnabled && clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage3.flatSArm) * 100)) && (currentStatus.RPM > ((unsigned int)(configPage3.lnchSoftLim) * 100)) )
  {
    currentStatus.launchingSoft = true;
    BIT_SET(currentStatus.spark, BIT_SPARK_SLAUNCH);
    return configPage3.lnchRetard;
  }

  currentStatus.launchingSoft = false;
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_SLAUNCH);
  return advance;
}

static inline int8_t correctionSoftFlatShift(int8_t advance)
{
  if(configPage3.flatSEnable && clutchTrigger && (currentStatus.clutchEngagedRPM > ((unsigned int)(configPage3.flatSArm) * 100)) && (currentStatus.RPM > (currentStatus.clutchEngagedRPM-configPage3.flatSSoftWin) ) )
  {
    BIT_SET(currentStatus.spark2, BIT_SPARK2_FLATSS);
    return configPage3.flatSRetard;
  }

  BIT_CLEAR(currentStatus.spark2, BIT_SPARK2_FLATSS);
  return advance;
}
