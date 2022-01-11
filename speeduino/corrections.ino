/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/** @file
Corrections to injection pulsewidth.
The corrections functions in this file affect the fuel pulsewidth (Either increasing or decreasing)
based on factors other than the VE lookup.

These factors include:
- Temperature (Warmup Enrichment and After Start Enrichment)
- Acceleration/Decelleration
- Flood clear mode
- etc.

Most correction functions return value 100 (like 100% == 1) for no need for correction.

There are 2 top level functions that call more detailed corrections for Fuel and Ignition respectively:
- @ref correctionsFuel() - All fuel related corrections
- @ref correctionsIgn() - All ignition related corrections
*/
//************************************************************************************************************

#include "globals.h"
#include "corrections.h"
#include "speeduino.h"
#include "timers.h"
#include "maths.h"
#include "sensors.h"


int MAP_rateOfChange;
int TPS_rateOfChange;
byte activateMAPDOT; //The mapDOT value seen when the MAE was activated. 
byte activateTPSDOT; //The tpsDOT value seen when the MAE was activated.

uint16_t ego_NextCycleCount;
unsigned long knockStartTime;
byte lastKnockCount;
int16_t knockWindowMin; //The current minimum crank angle for a knock pulse to be valid
int16_t knockWindowMax;//The current maximum crank angle for a knock pulse to be valid
uint16_t aseTaperStart;
uint16_t dfcoStart;
uint16_t idleAdvStart;
bool O2_SensorIsRich;
bool O2_SensorIsRichPrev;
bool O2_2ndSensorIsRich;
bool O2_2ndSensorIsRichPrev;
int16_t ego_FuelLoadPrev;
uint32_t ego_FreezeEndTime;
int8_t ego_Integral;
int8_t ego2_Integral;
uint32_t ego_DelaySensorTime;
uint8_t ego_IntDelayLoops;
uint8_t ego2_IntDelayLoops;
unsigned long pwLimit;

/** Initialize instances and vars related to corrections (at ECU boot-up).
 */
void initialiseCorrections()
{
  currentStatus.corrections = 100;
  currentStatus.flexIgnCorrection = 0;
  currentStatus.egoCorrection = 100; //Default value of no adjustment must be set to avoid randomness on first correction cycle after startup
  currentStatus.ego2Correction = 100; //Default value of no adjustment must be set to avoid randomness on first correction cycle after startup
  currentStatus.afrTarget = configPage2.stoich; // Init AFR Target at stoich.
  ego_NextCycleCount = 0;
  ego_FreezeEndTime = 0;
  ego_DelaySensorTime = 0;
  ego_Integral = 0;
  ego2_Integral = 0;
  O2_SensorIsRich = false;
  O2_SensorIsRichPrev = O2_SensorIsRich;
  O2_2ndSensorIsRich = false;
  O2_2ndSensorIsRichPrev = O2_2ndSensorIsRich;
  ego_FuelLoadPrev = currentStatus.fuelLoad;
  currentStatus.knockActive = false;
  currentStatus.battery10 = 125; //Set battery voltage to sensible value for dwell correction for "flying start" (else ignition gets suprious pulses after boot)  
}

/** Dispatch calculations for all fuel related corrections.
Calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
uint16_t correctionsFuel()
{
  #define MAX_CORRECTIONS 3 //The maximum number of corrections allowed before the sum is reprocessed
  uint32_t sumCorrections = 100;
  byte activeCorrections = 0;
  uint16_t result; //temporary variable to store the result of each corrections function

  //The values returned by each of the correction functions are multipled together and then divided back to give a single 0-255 value.
  currentStatus.wueCorrection = correctionWUE();
  if (currentStatus.wueCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.wueCorrection); activeCorrections++; }

  result = correctionASE();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }

  result = correctionCranking();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; } // Need to check this to ensure that sumCorrections doesn't overflow. Can occur when the number of corrections is greater than 3 (Which is 100^4) as 100^5 can overflow

  currentStatus.AEamount = correctionAccel();
  if (configPage2.aeApplyMode == AE_MODE_MULTIPLIER)
  {
  if (currentStatus.AEamount != 100) { sumCorrections = (sumCorrections * currentStatus.AEamount); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }
  }

  result = correctionFloodClear();
  if (result != 100) { sumCorrections = (sumCorrections * result); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.egoCorrection = correctionAFRClosedLoop();
  if (currentStatus.egoCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.egoCorrection); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.batCorrection = correctionBatVoltage();
  if (configPage2.battVCorMode == BATTV_COR_MODE_OPENTIME)
  {
    inj_opentime_uS = configPage2.injOpen * currentStatus.batCorrection; // Apply voltage correction to injector open time. *100 is no correction so this also converts to us.
  }
  if (configPage2.battVCorMode == BATTV_COR_MODE_WHOLE)
  {
    if (currentStatus.batCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.batCorrection); activeCorrections++; }
    if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }    
  }

  currentStatus.iatCorrection = correctionIATDensity();
  if (currentStatus.iatCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.iatCorrection); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.baroCorrection = correctionBaro();
  if (currentStatus.baroCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.baroCorrection); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.flexCorrection = correctionFlex();
  if (currentStatus.flexCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.flexCorrection); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.fuelTempCorrection = correctionFuelTemp();
  if (currentStatus.fuelTempCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.fuelTempCorrection); activeCorrections++; }
  if (activeCorrections == MAX_CORRECTIONS) { sumCorrections = sumCorrections / powint(100,activeCorrections); activeCorrections = 0; }

  currentStatus.launchCorrection = correctionLaunch();
  if (currentStatus.launchCorrection != 100) { sumCorrections = (sumCorrections * currentStatus.launchCorrection); activeCorrections++; }

  bitWrite(currentStatus.status1, BIT_STATUS1_DFCO, correctionDFCO());
  if ( BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) { sumCorrections = 0; }

  sumCorrections = sumCorrections / powint(100,activeCorrections);

  if(sumCorrections > 1500) { sumCorrections = 1500; } //This is the maximum allowable increase during cranking
  return (uint16_t)sumCorrections;
}

/*
correctionsTotal() calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
static inline byte correctionsFuel_new()
{
  uint32_t sumCorrections = 100;
  byte numCorrections = 0;

  //The values returned by each of the correction functions are multipled together and then divided back to give a single 0-255 value.
  currentStatus.wueCorrection = correctionWUE(); numCorrections++;
  uint16_t correctionASEvalue = correctionASE(); numCorrections++;
  uint16_t correctionCrankingValue = correctionCranking(); numCorrections++;
  currentStatus.AEamount = correctionAccel(); numCorrections++;
  uint8_t correctionFloodClearValue = correctionFloodClear(); numCorrections++;
  currentStatus.egoCorrection = correctionAFRClosedLoop(); numCorrections++;

  currentStatus.batCorrection = correctionBatVoltage(); numCorrections++;
  currentStatus.iatCorrection = correctionIATDensity(); numCorrections++;
  currentStatus.baroCorrection = correctionBaro(); numCorrections++; 
  currentStatus.flexCorrection = correctionFlex(); numCorrections++;
  currentStatus.launchCorrection = correctionLaunch(); numCorrections++;

  bitWrite(currentStatus.status1, BIT_STATUS1_DFCO, correctionDFCO());
  if ( BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) { sumCorrections = 0; }

  sumCorrections = currentStatus.wueCorrection \
                  + correctionASEvalue \
                  + correctionCrankingValue \
                  + currentStatus.AEamount \
                  + correctionFloodClearValue \
                  + currentStatus.batCorrection \
                  + currentStatus.iatCorrection \
                  + currentStatus.baroCorrection \
                  + currentStatus.flexCorrection \
                  + currentStatus.launchCorrection;
  return (sumCorrections);

}

/** Warm Up Enrichment (WUE) corrections.
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
byte correctionWUE()
{
  byte WUEValue;
  //Possibly reduce the frequency this runs at (Costs about 50 loops per second)
  //if (currentStatus.coolant > (WUETable.axisX[9] - CALIBRATION_TEMPERATURE_OFFSET))
  if (currentStatus.coolant > (table2D_getAxisValue(&WUETable, 9) - CALIBRATION_TEMPERATURE_OFFSET))
  {
    //This prevents us doing the 2D lookup if we're already up to temp
    BIT_CLEAR(currentStatus.engine, BIT_ENGINE_WARMUP);
    WUEValue = table2D_getRawValue(&WUETable, 9);
  }
  else
  {
    BIT_SET(currentStatus.engine, BIT_ENGINE_WARMUP);
    WUEValue = table2D_getValue(&WUETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
  }

  return WUEValue;
}

/** Cranking Enrichment corrections.
Additional fuel % to be added when the engine is cranking
*/
uint16_t correctionCranking()
{
  uint16_t crankingValue = 100;
  //Check if we are actually cranking
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
  {
    crankingValue = table2D_getValue(&crankingEnrichTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
    crankingValue = (uint16_t) crankingValue * 5; //multiplied by 5 to get range from 0% to 1275%
  }
  
  //If we're not cranking, check if if cranking enrichment tapering to ASE should be done
  else if ( (uint32_t) runSecsX10 <= configPage10.crankingEnrichTaper)
  {
    crankingValue = table2D_getValue(&crankingEnrichTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
    crankingValue = (uint16_t) crankingValue * 5; //multiplied by 5 to get range from 0% to 1275%
    //Taper start value needs to account for ASE that is now running, so total correction does not increase when taper begins
    unsigned long taperStart = (unsigned long) crankingValue * 100 / currentStatus.ASEValue;
    crankingValue = (uint16_t) map(runSecsX10, 0, configPage10.crankingEnrichTaper, taperStart, 100); //Taper from start value to 100%
    if (crankingValue < 100) { crankingValue = 100; } //Sanity check
  }
  return crankingValue;
}

/** Afer Start Enrichment calculation.
 * This is a short period (Usually <20 seconds) immediately after the engine first fires (But not when cranking)
 * where an additional amount of fuel is added (Over and above the WUE amount).
 * 
 * @return uint8_t The After Start Enrichment modifier as a %. 100% = No modification. 
 */
byte correctionASE()
{
  int16_t ASEValue;
  //Two checks are requiredL:
  //1) Is the engine run time less than the configured ase time
  //2) Make sure we're not still cranking
  if ( BIT_CHECK(TIMER_mask, BIT_TIMER_10HZ) || (currentStatus.ASEValue == 0) )
  {
    if ( (currentStatus.runSecs < (table2D_getValue(&ASECountTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET))) && !(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) )
    {
      BIT_SET(currentStatus.engine, BIT_ENGINE_ASE); //Mark ASE as active.
      ASEValue = 100 + table2D_getValue(&ASETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
      aseTaperStart = runSecsX10;
    }
    else
    {
      if ( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && ((runSecsX10 - aseTaperStart) < configPage2.aseTaperTime) ) //Cranking check needs to be here also, so cranking and afterstart enrichments won't run simultaneously
      {
        BIT_SET(currentStatus.engine, BIT_ENGINE_ASE); //Mark ASE as active.
        ASEValue = 100 + map((runSecsX10 - aseTaperStart), 0, configPage2.aseTaperTime,\
          table2D_getValue(&ASETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET), 0);
      }
      else
      {
        BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ASE); //Mark ASE as inactive.
        ASEValue = 100;
      }
    }
    
    //Safety checks
    if(ASEValue > 255) { ASEValue = 255; }
    if(ASEValue < 0) { ASEValue = 0; }
    currentStatus.ASEValue = (byte)ASEValue;
  }
  return currentStatus.ASEValue;
}

/** Acceleration enrichment correction calculation.
 * 
 * Calculates the % change of the throttle over time (%/second) and performs a lookup based on this
 * Coolant-based modifier is applied on the top of this.
 * When the enrichment is turned on, it runs at that amount for a fixed period of time (taeTime)
 * 
 * @return uint16_t The Acceleration enrichment modifier as a %. 100% = No modification.
 * 
 * As the maximum enrichment amount is +255% and maximum cold adjustment for this is 255%, the overall return value
 * from this function can be 100+(255*255/100)=750. Hence this function returns a uint16_t rather than byte.
 */
uint16_t correctionAccel()
{
  int16_t accelValue = 100;
  int16_t MAP_change = 0;
  int8_t TPS_change = 0;

  if(configPage2.aeMode == AE_MODE_MAP)
  {
    //Get the MAP rate change
    MAP_change = (currentStatus.MAP - MAPlast);
    MAP_rateOfChange = ldiv(1000000, (MAP_time - MAPlast_time)).quot * MAP_change; //This is the % per second that the TPS has moved
    //MAP_rateOfChange = 15 * MAP_change; //This is the kpa per second that the MAP has moved
    if(MAP_rateOfChange >= 0) { currentStatus.mapDOT = MAP_rateOfChange / 10; } //The MAE bins are divided by 10 in order to allow them to be stored in a byte. Faster as this than divu10
    else { currentStatus.mapDOT = 0; } //Prevent overflow as mapDOT is signed
  }
  else if(configPage2.aeMode == AE_MODE_TPS)
  {
    //Get the TPS rate change
    TPS_change = (currentStatus.TPS - TPSlast);
    //TPS_rateOfChange = ldiv(1000000, (TPS_time - TPSlast_time)).quot * TPS_change; //This is the % per second that the TPS has moved
    TPS_rateOfChange = TPS_READ_FREQUENCY * TPS_change; //This is the % per second that the TPS has moved
    if(TPS_rateOfChange >= 0) { currentStatus.tpsDOT = TPS_rateOfChange / 20; } //The TAE bins are divided by 10 in order to allow them to be stored in a byte and then by 2 due to TPS being 0.5% resolution (0-200)
    else { currentStatus.tpsDOT = 0; } //Prevent overflow as tpsDOT is signed
  }
  

  //First, check whether the accel. enrichment is already running
  if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) )
  {
    //If it is currently running, check whether it should still be running or whether it's reached it's end time
    if( micros_safe() >= currentStatus.AEEndTime )
    {
      //Time to turn enrichment off
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC);
      currentStatus.AEamount = 0;
      accelValue = 100;

      //Reset the relevant DOT value to 0
      if(configPage2.aeMode == AE_MODE_MAP) { currentStatus.mapDOT = 0; }
      else if(configPage2.aeMode == AE_MODE_TPS) { currentStatus.tpsDOT = 0; }
    }
    else
    {
      //Enrichment still needs to keep running. 
      //Simply return the total TAE amount
      accelValue = currentStatus.AEamount;

      //Need to check whether the accel amount has increased from when AE was turned on
      //If the accel amount HAS increased, we clear the current enrich phase and a new one will be started below
      if( (configPage2.aeMode == AE_MODE_MAP) && (currentStatus.mapDOT > activateMAPDOT) ) { BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); }
      else if( (configPage2.aeMode == AE_MODE_TPS) && (currentStatus.tpsDOT > activateTPSDOT) ) { BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); }
    }
  }

  //else
  if( !BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC) ) //Need to check this again as it may have been changed in the above section
  {
    if(configPage2.aeMode == AE_MODE_MAP)
    {
      if (MAP_change <= 2)
      {
        accelValue = 100;
        currentStatus.mapDOT = 0;
      }
      else
      {
        //If MAE isn't currently turned on, need to check whether it needs to be turned on
        if (MAP_rateOfChange > configPage2.maeThresh)
        {
          BIT_SET(currentStatus.engine, BIT_ENGINE_ACC); //Mark accleration enrichment as active.
          activateMAPDOT = currentStatus.mapDOT;
          currentStatus.AEEndTime = micros_safe() + ((unsigned long)configPage2.aeTime * 10000); //Set the time in the future where the enrichment will be turned off. taeTime is stored as mS / 10, so multiply it by 100 to get it in uS
          accelValue = table2D_getValue(&maeTable, currentStatus.mapDOT);

          //Apply the RPM taper to the above
          //The RPM settings are stored divided by 100:
          uint16_t trueTaperMin = configPage2.aeTaperMin * 100;
          uint16_t trueTaperMax = configPage2.aeTaperMax * 100;
          if (currentStatus.RPM > trueTaperMin)
          {
            if(currentStatus.RPM > trueTaperMax) { accelValue = 0; } //RPM is beyond taper max limit, so accel enrich is turned off
            else 
            {
              int16_t taperRange = trueTaperMax - trueTaperMin;
              int16_t taperPercent = ((currentStatus.RPM - trueTaperMin) * 100) / taperRange; //The percentage of the way through the RPM taper range
              accelValue = percentage((100-taperPercent), accelValue); //Calculate the above percentage of the calculated accel amount. 
            }
          }

          //Apply AE cold coolant modifier, if CLT is less than taper end temperature
          if ( currentStatus.coolant < (int)(configPage2.aeColdTaperMax - CALIBRATION_TEMPERATURE_OFFSET) )
          {
            //If CLT is less than taper min temp, apply full modifier on top of accelValue
            if ( currentStatus.coolant <= (int)(configPage2.aeColdTaperMin - CALIBRATION_TEMPERATURE_OFFSET) )
            {
              uint16_t accelValue_uint = (uint16_t) accelValue * configPage2.aeColdPct / 100;
              accelValue = (int16_t) accelValue_uint;
            }
            //If CLT is between taper min and max, taper the modifier value and apply it on top of accelValue
            else
            {
              int16_t taperRange = (int16_t) configPage2.aeColdTaperMax - configPage2.aeColdTaperMin;
              int16_t taperPercent = (int)((currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET - configPage2.aeColdTaperMin) * 100) / taperRange;
              int16_t coldPct = (int16_t) 100+ percentage((100-taperPercent), configPage2.aeColdPct-100);
              uint16_t accelValue_uint = (uint16_t) accelValue * coldPct / 100; //Potential overflow (if AE is large) without using uint16_t
              accelValue = (int16_t) accelValue_uint;
            }
          }
          
          accelValue = 100 + accelValue; //Add the 100 normalisation to the calculated amount
        } //MAE Threshold
      }
    }
    else if(configPage2.aeMode == AE_MODE_TPS)
    {
    
      //Check for deceleration (Deceleration adjustment not yet supported)
      //Also check for only very small movement (Movement less than or equal to 2% is ignored). This not only means we can skip the lookup, but helps reduce false triggering around 0-2% throttle openings
      if (TPS_change <= 2)
      {
        accelValue = 100;
        currentStatus.tpsDOT = 0;
      }
      else
      {
        //If TAE isn't currently turned on, need to check whether it needs to be turned on
        if (TPS_rateOfChange > configPage2.taeThresh)
        {
          BIT_SET(currentStatus.engine, BIT_ENGINE_ACC); //Mark accleration enrichment as active.
          activateTPSDOT = currentStatus.tpsDOT;
          currentStatus.AEEndTime = micros_safe() + ((unsigned long)configPage2.aeTime * 10000); //Set the time in the future where the enrichment will be turned off. taeTime is stored as mS / 10, so multiply it by 100 to get it in uS
          accelValue = table2D_getValue(&taeTable, currentStatus.tpsDOT);

          //Apply the RPM taper to the above
          //The RPM settings are stored divided by 100:
          uint16_t trueTaperMin = configPage2.aeTaperMin * 100;
          uint16_t trueTaperMax = configPage2.aeTaperMax * 100;
          if (currentStatus.RPM > trueTaperMin)
          {
            if(currentStatus.RPM > trueTaperMax) { accelValue = 0; } //RPM is beyond taper max limit, so accel enrich is turned off
            else 
            {
              int16_t taperRange = trueTaperMax - trueTaperMin;
              int16_t taperPercent = ((currentStatus.RPM - trueTaperMin) * 100) / taperRange; //The percentage of the way through the RPM taper range
              accelValue = percentage((100-taperPercent), accelValue); //Calculate the above percentage of the calculated accel amount. 
            }
          }

          //Apply AE cold coolant modifier, if CLT is less than taper end temperature
          if ( currentStatus.coolant < (int)(configPage2.aeColdTaperMax - CALIBRATION_TEMPERATURE_OFFSET) )
          {
            //If CLT is less than taper min temp, apply full modifier on top of accelValue
            if ( currentStatus.coolant <= (int)(configPage2.aeColdTaperMin - CALIBRATION_TEMPERATURE_OFFSET) )
            {
              uint16_t accelValue_uint = (uint16_t) accelValue * configPage2.aeColdPct / 100;
              accelValue = (int16_t) accelValue_uint;
            }
            //If CLT is between taper min and max, taper the modifier value and apply it on top of accelValue
            else
            {
              int16_t taperRange = (int16_t) configPage2.aeColdTaperMax - configPage2.aeColdTaperMin;
              int16_t taperPercent = (int)((currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET - configPage2.aeColdTaperMin) * 100) / taperRange;
              int16_t coldPct = (int16_t) 100+ percentage((100-taperPercent), configPage2.aeColdPct-100);
              uint16_t accelValue_uint = (uint16_t) accelValue * coldPct / 100; //Potential overflow (if AE is large) without using uint16_t
              accelValue = (int16_t) accelValue_uint;
            }
          }

          accelValue = 100 + accelValue; //Add the 100 normalisation to the calculated amount
        } //TAE Threshold
      } //TPS change > 2
    } //AE Mode
  } //AE active

  return accelValue;
}

/** Simple check to see whether we are cranking with the TPS above the flood clear threshold.
@return 100 (not cranking and thus no need for flood-clear) or 0 (Engine cranking and TPS above @ref config4.floodClear limit).
*/
byte correctionFloodClear()
{
  byte floodValue = 100;
  if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
  {
    //Engine is currently cranking, check what the TPS is
    if(currentStatus.TPS >= configPage4.floodClear)
    {
      //Engine is cranking and TPS is above threshold. Cut all fuel
      floodValue = 0;
    }
  }
  return floodValue;
}

/** Battery Voltage correction.
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add.
*/
byte correctionBatVoltage()
{
  byte batValue = 100;
  batValue = table2D_getValue(&injectorVCorrectionTable, currentStatus.battery10);
  return batValue;
}

/** Simple temperature based corrections lookup based on the inlet air temperature (IAT).
This corrects for changes in air density from movement of the temperature.
*/
byte correctionIATDensity()
{
  byte IATValue = 100;
  IATValue = table2D_getValue(&IATDensityCorrectionTable, currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET); //currentStatus.IAT is the actual temperature, values in IATDensityCorrectionTable.axisX are temp+offset

  return IATValue;
}

/** Correction for current baromtetric / ambient pressure.
 * @returns A percentage value indicating the amount the fueling should be changed based on the barometric reading. 100 = No change. 110 = 10% increase. 90 = 10% decrease
 */
byte correctionBaro()
{
  byte baroValue = 100;
  baroValue = table2D_getValue(&baroFuelTable, currentStatus.baro);

  return baroValue;
}

/** Launch control has a setting to increase the fuel load to assist in bringing up boost.
This simple check applies the extra fuel if we're currently launching
*/
byte correctionLaunch()
{
  byte launchValue = 100;
  if(currentStatus.launchingHard || currentStatus.launchingSoft) { launchValue = (100 + configPage6.lnchFuelAdd); }

  return launchValue;
}

/*
 * Returns true if decelleration fuel cutoff should be on, false if its off
 */
bool correctionDFCO()
{
  bool DFCOValue = false;
  if ( configPage2.dfcoEnabled == 1 )
  {
    if ( BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO) == 1 ) 
    {
      DFCOValue = ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ); 
      if ( DFCOValue == false) { dfcoStart = 0; }
    }
    else 
    {
      if ( ( currentStatus.coolant >= (int)(configPage2.dfcoMinCLT - CALIBRATION_TEMPERATURE_OFFSET) ) && ( currentStatus.RPM > (unsigned int)( (configPage4.dfcoRPM * 10) + configPage4.dfcoHyster) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ) )
      {
        if ( dfcoStart == 0 ) { dfcoStart = runSecsX10; }
        if( (runSecsX10 - dfcoStart) > configPage2.dfcoDelay ) { DFCOValue = true; }
      }
      else { dfcoStart = 0; } //Prevent future activation right away if previous time wasn't activated
    } // DFCO active check
  } // DFCO enabled check
  return DFCOValue;
}

/** Flex fuel adjustment to vary fuel based on ethanol content.
 * The amount of extra fuel required is a linear relationship based on the % of ethanol.
*/
byte correctionFlex()
{
  byte flexValue = 100;

  if (configPage2.flexEnabled == 1)
  {
    flexValue = table2D_getValue(&flexFuelTable, currentStatus.ethanolPct);
  }
  return flexValue;
}

/*
 * Fuel temperature adjustment to vary fuel based on fuel temperature reading
*/
byte correctionFuelTemp()
{
  byte fuelTempValue = 100;

  if (configPage2.flexEnabled == 1)
  {
    fuelTempValue = table2D_getValue(&fuelTempTable, currentStatus.fuelTemp + CALIBRATION_TEMPERATURE_OFFSET);
  }
  return fuelTempValue;
}


/*
* Closed loop using Oxygen Sensors. Lookup the AFR target table and perform a Proportional + Integral fueling adjustment based on this.
*/
byte correctionAFRClosedLoop()
{
  byte ego_AdjustPct = 100;
  byte ego2_AdjustPct = 100;
  uint8_t O2_Error;
  int8_t ego_Prop;
  int8_t ego2_Prop;
  bool ego_EngineCycleCheck = false;
  
  /*Note that this should only run after the sensor warmup delay when using Include AFR option, but this is protected in the main loop where it's used so really don't need this.
   * When using Incorporate AFR option it needs to be done at all times
  */
  // Start AFR Target Determination
  if((configPage2.incorporateAFR == true) || 
     ((configPage6.egoType > 0) &&
      (currentStatus.runSecs > configPage6.egoStartdelay))) //afrTarget value lookup must be done if O2 sensor is enabled, and always if incorporateAFR is enabled
  {
    currentStatus.afrTarget = get3DTableValue(&afrTable, currentStatus.fuelLoad, currentStatus.RPM); 
  }
  else { currentStatus.afrTarget = configPage2.stoich; }
  // END AFR Target Determination    
    
  if ((currentStatus.startRevolutions >> 1) >= ego_NextCycleCount) // Crank revolutions divided by 2 is engine cycles. This check always needs to happen, to correctly align revolutions and the time delay
    {
      ego_EngineCycleCheck = true;
      // Scale the revolution counts between the two values linearly based on load value used in VE table.
      ego_NextCycleCount = (currentStatus.startRevolutions >> 1) + (uint16_t)map(currentStatus.fuelLoad, 0, (int16_t)configPage6.egoFuelLoadMax*2, (int16_t)configPage6.egoCountL, (int16_t)configPage6.egoCountH); 
    }
  
  //General Enable Condtions for closed loop ego. egoType of 0 means no O2 sensor and no point having O2 closed loop and cannot use include AFR from sensor since this would be 2x proportional controls.
  if( (configPage6.egoType > 0) && (configPage6.egoAlgorithm <= EGO_ALGORITHM_DUALO2) && (configPage2.includeAFR == false) ) 
  {
    //Requirements to NOT run Closed Loop (Freeze), check this rapidly so we don't miss freeze events.
    if ((abs((currentStatus.fuelLoad - ego_FuelLoadPrev)) > (int16_t)configPage9.egoFuelLoadChngMax*2 ) || //Change in fuel load (MAP or TPS) since last time algo ran to see if we need to freeze algo due to load change.
        (currentStatus.afrTarget < configPage6.egoAFRTargetMin) || // Target too rich - good for inhibiting O2 correction using AFR Target Table
        (currentStatus.fuelLoad > (int16_t)configPage6.egoFuelLoadMax*2) || // Too much load
        (currentStatus.launchCorrection != 100) || // Launch Control Active
        (BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO) == 1)) //Fuel Cut
    { 
      ego_FreezeEndTime = runSecsX10 + configPage9.egoFreezeDelay; // Set ego freeze condition timer
    }
       
    if ((ego_EngineCycleCheck == true) && (runSecsX10 >= ego_DelaySensorTime))
    {
      ego_DelaySensorTime = runSecsX10 + configPage6.egoSensorDelay; // Save the minimum sensor delay time for next loop
      // Read the O2 sensors before the algo runs, may be faster than the main loop.
      readO2();
      readO2_2();
      O2_Readflag = true; // Used for informing the time based O2 sensor read function that we read the O2 value here.
      ego_FuelLoadPrev = currentStatus.fuelLoad; // save last value to check for load change

      //Requirements to run Closed Loop else its reset to 100pct. These are effectively errors where closed loop cannot run.
      if( (currentStatus.coolant > (int)(configPage6.egoTemp - CALIBRATION_TEMPERATURE_OFFSET)) && 
          (currentStatus.RPM >= (unsigned int)(configPage6.egoRPM * 100)) &&
          (currentStatus.runSecs > configPage6.egoStartdelay) &&
          (currentStatus.engineProtectStatus == 0) &&      // Engine protection , fuel or ignition cut is active.     
          ((configPage2.egoResetwAFR == false) ||
           (currentStatus.afrTarget >= configPage6.egoAFRTargetMin)) && // Ignore this criteria if cal set to freeze (false).
          ((configPage2.egoResetwfuelLoad == false) ||
           (currentStatus.fuelLoad <= (int16_t)configPage6.egoFuelLoadMax*2))) // Ignore this criteria if cal set to freeze (false).
      {
        BIT_SET(currentStatus.status4, BIT_STATUS4_EGO_READY);
        
        if(runSecsX10 >= ego_FreezeEndTime) // Check the algo freeze conditions are not active.
        {
          BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO_FROZEN);
          
          /* Build the lookup table for the integrator dynamically. This saves eeprom by using less variables. 
           * The calibration values are expressed as a "% of max adjustment" where the max adjustment would theoretically correct the AFR to the target in one step." 
           * The scaling in the .ini file applies the correct adjustment in g_ego % units depending on the fixed axis defined by. egoIntAFR_XBins.
           * For example 3.0 afr error is 20% g_ego. So 100%/20% = 5 % per step. If egoIntAFR_XBins axis points are changed, the scaling in the .ini file also needs to be adjusted. 
          */
          egoIntAFR_Values[0] = configPage9.egoInt_Lean2; // Corresponds with -3.0 AFR Error
          egoIntAFR_Values[1] = configPage9.egoInt_Lean1; // Corresponds with -0.6 AFR Error
          egoIntAFR_Values[2] = OFFSET_AFR_ERR; //offset value is 0 adjustment
          egoIntAFR_Values[3] = configPage9.egoInt_Rich1; // Corresponds with 0.6 AFR Error
          egoIntAFR_Values[4] = configPage9.egoInt_Rich2; // Corresponds with 3.0 AFR Error
          
          
          // Sensor check to check in range.
          if ((currentStatus.O2 >= configPage6.egoMin) && // Not too rich
              ((currentStatus.O2 <= configPage6.egoMax) ||
              (BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO) == 1))) // Not too lean but ignore egoMax (lean) if in DFCO.
          {              
            //Integral Control - Sensor 1
            O2_Error = currentStatus.afrTarget - currentStatus.O2 + OFFSET_AFR_ERR; //+127 is 0 error value. Richer than target is positive.
            
            /* Tracking of rich and lean (compared to target). Used for an integrator delay which is intended to allow proportional switching control
             * time to intentionally over-correct the fuel to generate a switch from rich to lean. If the proportional control alone is not enough to switch 
             * only then will the integral will start to move to adjust the mean value after a certain amount of time.
             * This is designed to generate the correct oscillation of rich and lean pulses required for 3 way catalyst control.
             * This logic only makes sense if the AFR target is at the stoich point for the chosen fuel.           
            */ 
            if ((configPage9.egoIntDelay > 0) && (currentStatus.afrTarget == configPage2.stoich))
            {
              O2_SensorIsRichPrev = O2_SensorIsRich;
              if (O2_Error > OFFSET_AFR_ERR) 
              { 
                 O2_SensorIsRich = true;  //Positive error = rich. 
                 ego_Prop = -(int8_t)(configPage9.egoProp_Swing); // Negative swing on prop.
              } 
              else 
              { 
                O2_SensorIsRich = false; 
                ego_Prop = (int8_t)(configPage9.egoProp_Swing);  //Positive swing on prop.
              } 
           
              if (O2_SensorIsRich == O2_SensorIsRichPrev) // Increment delay loops for the integrator if switch not detected 
              {
                if (ego_IntDelayLoops < configPage9.egoIntDelay) { ego_IntDelayLoops++; } // Limit to max value.
              }              
              else { ego_IntDelayLoops = 0; } // Switch in fuelling has been detected, reset integrator delay counter. If the switch is not detected the integrator will keep updating every loop after this delay.
            }
            else 
            { 
              ego_IntDelayLoops = configPage9.egoIntDelay; 
              ego_Prop = 0;
            }
            
            //If integrator delay is passed then update integrator
            if (ego_IntDelayLoops >= configPage9.egoIntDelay) 
            { 
              BIT_SET(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR);
              ego_Integral = ego_Integral + (int8_t)(table2D_getValue(&ego_IntegralTable, O2_Error) - OFFSET_AFR_ERR); 
            } //Integrate step value from table
            
            //Integrator Limits
            if (ego_Integral < -configPage6.egoLimit) { ego_Integral = -configPage6.egoLimit; BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR); }
            if (ego_Integral > configPage6.egoLimit) { ego_Integral = configPage6.egoLimit; BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR); }
            
            //2nd check to limit total value after update with prop and output the final correction
            if ((ego_Integral + ego_Prop) < -configPage6.egoLimit) { ego_AdjustPct = 100 - configPage6.egoLimit; }
            else if ((ego_Integral + ego_Prop) > configPage6.egoLimit) { ego_AdjustPct = 100 + configPage6.egoLimit; }
            else { ego_AdjustPct = 100 + ego_Integral + ego_Prop; }
          }
          else 
          { // O2 sensor out of range
            BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR);
            ego_AdjustPct = 100;
            ego_Integral = 0;
            ego_IntDelayLoops = 0; 
          } 

          // Sensor2 check to check in range and if Enabled
          if ((configPage6.egoAlgorithm == EGO_ALGORITHM_DUALO2) && // 2nd Sensor Logic
              (currentStatus.O2_2 >= configPage6.egoMin) && // Not too rich
              ((currentStatus.O2_2 <= configPage6.egoMax) ||
               (BIT_CHECK(currentStatus.status1, BIT_STATUS1_DFCO) == 1))) // Not too lean but ignore egoMax (lean) if in DFCO. 
          {
            //Proportional Integral Control - Sensor 2 Re-using some variables to save RAM.
            O2_Error = currentStatus.afrTarget - currentStatus.O2_2 + OFFSET_AFR_ERR; //+127 is 0 error value. Richer than target is positive.
            
            /* Tracking of rich and lean (compared to target). Used for an integrator delay which is intended to allow proportional switching control
             * time to intentionally over-correct the fuel to generate a switch from rich to lean. If the proportional control alone is not enough to switch 
             * only then will the integral will start to move to adjust the mean value after a certain amount of time.
             * This is designed to generate the correct oscillation of rich and lean pulses required for 3 way catalyst control.
             * This logic only makes sense if the AFR target is at the stoich point for the chosen fuel.           
            */ 
            if ((configPage9.egoIntDelay > 0) && (currentStatus.afrTarget == configPage2.stoich))
            {
              O2_2ndSensorIsRichPrev = O2_2ndSensorIsRich;
              if (O2_Error > OFFSET_AFR_ERR) 
              { 
                 O2_2ndSensorIsRich = true;  //Positive error = rich. 
                 ego2_Prop = -(int8_t)(configPage9.egoProp_Swing); // Negative swing on prop.
              } 
              else 
              { 
                O2_2ndSensorIsRich = false; 
                ego2_Prop = (int8_t)(configPage9.egoProp_Swing);  //Positive swing on prop.
              } 
           
              if (O2_2ndSensorIsRich == O2_2ndSensorIsRichPrev) // Increment delay loops for the integrator if switch not detected 
              {
                if (ego2_IntDelayLoops < configPage9.egoIntDelay) { ego2_IntDelayLoops++; } // Limit to max value.
              }              
              else { ego2_IntDelayLoops = 0; } // Switch in fuelling has been detected, reset integrator delay counter. If the switch is not detected the integrator will keep updating every loop after this delay.
            }
            else 
            { 
              ego2_IntDelayLoops = configPage9.egoIntDelay; 
              ego2_Prop = 0;
            }
                        
            //If integrator delay is passed then update integrator
            if (ego2_IntDelayLoops >= configPage9.egoIntDelay) 
            { 
               BIT_SET(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR);
               ego2_Integral = ego2_Integral + (int8_t)(table2D_getValue(&ego_IntegralTable, O2_Error) - OFFSET_AFR_ERR); 
            } //Integrate step value from table
            
            //Integrator Limits
            if (ego2_Integral < -configPage6.egoLimit) { ego2_Integral = -configPage6.egoLimit; BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR); }
            if (ego2_Integral > configPage6.egoLimit) { ego2_Integral = configPage6.egoLimit; BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR); }
            
            //2nd check to limit total value after update with prop and output the final correction
            if ((ego2_Integral + ego2_Prop) < -configPage6.egoLimit) { ego2_AdjustPct = 100 - configPage6.egoLimit; }
            else if ((ego2_Integral + ego2_Prop) > configPage6.egoLimit) { ego2_AdjustPct = 100 + configPage6.egoLimit; }
            else { ego2_AdjustPct = 100 + ego2_Integral + ego2_Prop; }
          }
          else 
          { // No 2nd O2 or O2 sensor out of range
            BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR);
            ego2_AdjustPct = 100;
            ego2_Integral = 0;
            ego2_IntDelayLoops = 0; 
          } 
        } // End Conditions to not freeze ego correction
        else { ego_AdjustPct = currentStatus.egoCorrection; ego2_AdjustPct = currentStatus.ego2Correction; BIT_SET(currentStatus.status4, BIT_STATUS4_EGO_FROZEN); BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR);} // ego frozen at last values
  	  } // End Conditions not to reset ego
  	  else 
      { //Reset closed loop. Also activate freeze delay to for when we re-enable.
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO_READY);
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR);
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR);
        ego_AdjustPct = 100;
        ego2_AdjustPct = 100;      
        ego_Integral = 0;
        ego2_Integral = 0;        
        ego_IntDelayLoops = 0;
        ego2_IntDelayLoops = 0;         
        ego_FreezeEndTime = runSecsX10 + configPage9.egoFreezeDelay;
      }
    } //End O2 Algorithm Run Loop check
    else
    {
      if (currentStatus.RPM >= (unsigned int)(configPage6.egoRPM * 100)) 
      { // hold last value
        ego_AdjustPct = currentStatus.egoCorrection; 
        ego2_AdjustPct = currentStatus.ego2Correction;
      } 
      else 
      {// Engine speed probably stopped or recranking so don't apply EGO during crank.
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO_READY);
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO_FROZEN);
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR);
        BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR);
        ego_AdjustPct = 100;
        ego2_AdjustPct = 100;  
        ego_NextCycleCount = 0;
        ego_DelaySensorTime = 0;
        ego_Integral = 0;
        ego2_Integral = 0;
        ego_IntDelayLoops = 0;
        ego2_IntDelayLoops = 0; 
        ego_FreezeEndTime = 0;
      } 
    }
  } //End egoType
  else
  { // No O2 sensors or incorrect config to run closed loop O2
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO_READY);
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO_FROZEN);
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO1_INTCORR);
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_EGO2_INTCORR);
    ego_AdjustPct = 100;
    ego2_AdjustPct = 100;  
    ego_NextCycleCount = 0;
    ego_DelaySensorTime = 0;
    ego_Integral = 0;
    ego2_Integral = 0;
    ego_IntDelayLoops = 0;
    ego2_IntDelayLoops = 0;
    ego_FreezeEndTime = 0; 
  }
  
  // This algo only returns a single byte for the bank1 correction. A 2nd ego output is needed for Bank2 which is used later in individual bank adjustments.
  currentStatus.ego2Correction = ego2_AdjustPct;  

  return ego_AdjustPct;
}

//******************************** IGNITION ADVANCE CORRECTIONS ********************************
/** Dispatch calculations for all ignition related corrections.
 * @param base_advance - Base ignition advance (deg. ?)
 * @return Advance considering all (~12) individual corrections
 */
int8_t correctionsIgn(int8_t base_advance)
{
  int8_t advance;
  advance = correctionFlexTiming(base_advance);
  advance = correctionWMITiming(advance);
  advance = correctionIATretard(advance);
  advance = correctionCLTadvance(advance);
  advance = correctionIdleAdvance(advance);
  advance = correctionSoftRevLimit(advance);
  advance = correctionNitrous(advance);
  advance = correctionSoftLaunch(advance);
  advance = correctionSoftFlatShift(advance);
  advance = correctionKnock(advance);

  //Fixed timing check must go last
  advance = correctionFixedTiming(advance);
  advance = correctionCrankingFixedTiming(advance); //This overrides the regular fixed timing, must come last

  return advance;
}
/** Correct ignition timing to configured fixed value.
 * Must be called near end to override all other corrections.
 */
int8_t correctionFixedTiming(int8_t advance)
{
  int8_t ignFixValue = advance;
  if (configPage2.fixAngEnable == 1) { ignFixValue = configPage4.FixAng; } //Check whether the user has set a fixed timing angle
  return ignFixValue;
}
/** Correct ignition timing to configured fixed value to use during craning.
 * Must be called near end to override all other corrections.
 */
int8_t correctionCrankingFixedTiming(int8_t advance)
{
  int8_t ignCrankFixValue = advance;
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) ) { ignCrankFixValue = configPage4.CrankAng; } //Use the fixed cranking ignition angle
  return ignCrankFixValue;
}

int8_t correctionFlexTiming(int8_t advance)
{
  int16_t ignFlexValue = advance;
  if( configPage2.flexEnabled == 1 ) //Check for flex being enabled
  {
    ignFlexValue = (int16_t) table2D_getValue(&flexAdvTable, currentStatus.ethanolPct) - OFFSET_IGNITION; //Negative values are achieved with offset
    currentStatus.flexIgnCorrection = (int8_t) ignFlexValue; //This gets cast to a signed 8 bit value to allows for negative advance (ie retard) values here. 
    ignFlexValue = (int8_t) advance + currentStatus.flexIgnCorrection;
  }
  return (int8_t) ignFlexValue;
}

int8_t correctionWMITiming(int8_t advance)
{
  if( (configPage10.wmiEnabled >= 1) && (configPage10.wmiAdvEnabled == 1) && !BIT_CHECK(currentStatus.status4, BIT_STATUS4_WMI_EMPTY) ) //Check for wmi being enabled
  {
    if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPM >= configPage10.wmiRPM) && (currentStatus.MAP/2 >= configPage10.wmiMAP) && ((currentStatus.IAT + CALIBRATION_TEMPERATURE_OFFSET) >= configPage10.wmiIAT) )
    {
      return (int16_t) advance + table2D_getValue(&wmiAdvTable, currentStatus.MAP/2) - OFFSET_IGNITION; //Negative values are achieved with offset
    }
  }
  return advance;
}
/** Ignition correction for inlet air temperature (IAT).
 */
int8_t correctionIATretard(int8_t advance)
{
  int8_t advanceIATadjust = table2D_getValue(&IATRetardTable, currentStatus.IAT);

  return advance - advanceIATadjust;
}
/** Ignition correction for coolant temperature (CLT).
 */
int8_t correctionCLTadvance(int8_t advance)
{
  int8_t ignCLTValue = advance;
  //Adjust the advance based on CLT.
  int8_t advanceCLTadjust = (int16_t)(table2D_getValue(&CLTAdvanceTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) - 15;
  ignCLTValue = (advance + advanceCLTadjust);
  
  return ignCLTValue;
}
/** Ignition Idle advance correction.
 */
int8_t correctionIdleAdvance(int8_t advance)
{

  int8_t ignIdleValue = advance;
  //Adjust the advance based on idle target rpm.
  if( (configPage2.idleAdvEnabled >= 1) && (runSecsX10 >= (configPage2.idleAdvDelay * 5)) )
  {
    currentStatus.CLIdleTarget = (byte)table2D_getValue(&idleTargetTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
    int idleRPMdelta = (currentStatus.CLIdleTarget - (currentStatus.RPM / 10) ) + 50;
    // Limit idle rpm delta between -500rpm - 500rpm
    if(idleRPMdelta > 100) { idleRPMdelta = 100; }
    if(idleRPMdelta < 0) { idleRPMdelta = 0; }
    if( (currentStatus.RPMdiv100 < configPage2.idleAdvRPM) && ((configPage2.vssMode == 0) || (currentStatus.vss < configPage2.idleAdvVss))
    && (((configPage2.idleAdvAlgorithm == 0) && (currentStatus.TPS < configPage2.idleAdvTPS)) || ((configPage2.idleAdvAlgorithm == 1) && (currentStatus.CTPSActive == 1))) ) // closed throttle position sensor (CTPS) based idle state
    {
      if( (runSecsX10 - idleAdvStart) >= configPage9.idleAdvStartDelay )
      {
        int8_t advanceIdleAdjust = (int16_t)(table2D_getValue(&idleAdvanceTable, idleRPMdelta)) - 15;
        if(configPage2.idleAdvEnabled == 1) { ignIdleValue = (advance + advanceIdleAdjust); }
        else if(configPage2.idleAdvEnabled == 2) { ignIdleValue = advanceIdleAdjust; }
      }
    }
    else if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { idleAdvStart = runSecsX10; } //Only copy time at runSecsX10 update rate
  }
  return ignIdleValue;
}
/** Ignition soft revlimit correction.
 */
int8_t correctionSoftRevLimit(int8_t advance)
{
  byte ignSoftRevValue = advance;
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_SFTLIM);

  if (configPage6.engineProtectType == PROTECT_CUT_IGN || configPage6.engineProtectType == PROTECT_CUT_BOTH) 
  {
    if (currentStatus.RPM > ((unsigned int)(configPage4.SoftRevLim) * 100) ) //Softcut RPM limit
    {
      if( (runSecsX10 - softStartTime) < configPage4.SoftLimMax )
      {
        BIT_SET(currentStatus.spark, BIT_SPARK_SFTLIM);
        if (configPage2.SoftLimitMode == SOFT_LIMIT_RELATIVE) { ignSoftRevValue = ignSoftRevValue - configPage4.SoftLimRetard; } //delay timing by configured number of degrees in relative mode
        else if (configPage2.SoftLimitMode == SOFT_LIMIT_FIXED) { ignSoftRevValue = configPage4.SoftLimRetard; } //delay timing to configured number of degrees in fixed mode
      }
    }  
    else if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { softStartTime = runSecsX10; } //Only copy time at runSecsX10 update rate
  }

  return ignSoftRevValue;
}
/** Ignition Nitrous oxide correction.
 */
int8_t correctionNitrous(int8_t advance)
{
  byte ignNitrous = advance;
  //Check if nitrous is currently active
  if(configPage10.n2o_enable > 0)
  {
    //Check which stage is running (if any)
    if( (currentStatus.nitrous_status == NITROUS_STAGE1) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      ignNitrous -= configPage10.n2o_stage1_retard;
    }
    if( (currentStatus.nitrous_status == NITROUS_STAGE2) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      ignNitrous -= configPage10.n2o_stage2_retard;
    }
  }

  return ignNitrous;
}
/** Ignition soft launch correction.
 */
int8_t correctionSoftLaunch(int8_t advance)
{
  byte ignSoftLaunchValue = advance;
  //SoftCut rev limit for 2-step launch control.
  if (configPage6.launchEnabled && clutchTrigger && (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > ((unsigned int)(configPage6.lnchSoftLim) * 100)) && (currentStatus.TPS >= configPage10.lnchCtrlTPS) )
  {
    currentStatus.launchingSoft = true;
    BIT_SET(currentStatus.spark, BIT_SPARK_SLAUNCH);
    ignSoftLaunchValue = configPage6.lnchRetard;
  }
  else
  {
    currentStatus.launchingSoft = false;
    BIT_CLEAR(currentStatus.spark, BIT_SPARK_SLAUNCH);
  }

  return ignSoftLaunchValue;
}
/** Ignition correction for soft flat shift.
 */
int8_t correctionSoftFlatShift(int8_t advance)
{
  int8_t ignSoftFlatValue = advance;

  if(configPage6.flatSEnable && clutchTrigger && (currentStatus.clutchEngagedRPM > ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > (currentStatus.clutchEngagedRPM - (configPage6.flatSSoftWin * 100) ) ) )
  {
    BIT_SET(currentStatus.spark2, BIT_SPARK2_FLATSS);
    ignSoftFlatValue = configPage6.flatSRetard;
  }
  else { BIT_CLEAR(currentStatus.spark2, BIT_SPARK2_FLATSS); }

  return ignSoftFlatValue;
}
/** Ignition knock (retard) correction.
 */
int8_t correctionKnock(int8_t advance)
{
  byte knockRetard = 0;

  //First check is to do the window calculations (ASsuming knock is enabled)
  if( configPage10.knock_mode != KNOCK_MODE_OFF )
  {
    knockWindowMin = table2D_getValue(&knockWindowStartTable, currentStatus.RPMdiv100);
    knockWindowMax = knockWindowMin + table2D_getValue(&knockWindowDurationTable, currentStatus.RPMdiv100);
  }


  if( (configPage10.knock_mode == KNOCK_MODE_DIGITAL)  )
  {
    //
    if(knockCounter > configPage10.knock_count)
    {
      if(currentStatus.knockActive == true)
      {
        //Knock retard is currently 
      }
      else
      {
        //Knock needs to be activated
        lastKnockCount = knockCounter;
        knockStartTime = micros();
        knockRetard = configPage10.knock_firstStep;
      }
    }

  }

  return advance - knockRetard;
}

/** Ignition Dwell Correction.
 */
uint16_t correctionsDwell(uint16_t dwell)
{
  uint16_t tempDwell = dwell;
  //Pull battery voltage based dwell correction and apply if needed
  currentStatus.dwellCorrection = table2D_getValue(&dwellVCorrectionTable, currentStatus.battery10);
  if (currentStatus.dwellCorrection != 100) { tempDwell = divs100(dwell) * currentStatus.dwellCorrection; }

  //Dwell limiter
  uint16_t dwellPerRevolution = tempDwell + (uint16_t)(configPage4.sparkDur * 100); //Spark duration is in mS*10. Multiple it by 100 to get spark duration in uS
  int8_t pulsesPerRevolution = 1;
  //Single channel spark mode is the only time there will be more than 1 pulse per revolution on any given output
  if( ( (configPage4.sparkMode == IGN_MODE_SINGLE) || (configPage4.sparkMode == IGN_MODE_ROTARY) ) && (configPage2.nCylinders > 1) ) //No point in running this for 1 cylinder engines
  {
    pulsesPerRevolution = (configPage2.nCylinders >> 1);
    dwellPerRevolution = dwellPerRevolution * pulsesPerRevolution;
  }

  if(dwellPerRevolution > revolutionTime)
  {
    //Possibly need some method of reducing spark duration here as well, but this is a start
    tempDwell = (revolutionTime / pulsesPerRevolution) - (configPage4.sparkDur * 100);
  }
  return tempDwell;
}

/*********************************************************************************************/
/* Below this line corrections are for individual injectors or injector banks. */

/** Corrections are for individual injectors or injector banks. 
* all these functions modify the injecton pulswidths PW1 to PW8. None of them can modify baseFuel since that is a global correction.
*/
void correctionsFuel_Individual(void)
{  
  //Initially all the pulse widths are set the same. Adjustments are made in the functions below
  currentStatus.PW1 = currentStatus.BaseFuel;
  currentStatus.PW2 = currentStatus.BaseFuel;
  currentStatus.PW3 = currentStatus.BaseFuel;
  currentStatus.PW4 = currentStatus.BaseFuel;
  currentStatus.PW5 = currentStatus.BaseFuel;
  currentStatus.PW6 = currentStatus.BaseFuel;
  currentStatus.PW7 = currentStatus.BaseFuel;
  currentStatus.PW8 = currentStatus.BaseFuel;
  
  // Globally used variable calculatons
  //Check that the duty cycle of the chosen pulsewidth isn't too high.
  pwLimit = percentage(configPage2.dutyLim, revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 crank revolution
  //Handle multiple squirts per rev
  if (configPage2.strokes == FOUR_STROKE) { pwLimit = pwLimit * 2 / currentStatus.nSquirts; } 
  else { pwLimit = pwLimit / currentStatus.nSquirts; }
  
  // Multiplications to fuel
  correctionEGOBank2();
  correctionFuelTrim();
  correctionFuelStaging(); // Fuel staging after fuel trim to incorporate trim into stage amount and limits.
  
  // Additions and Subtractions to fuel
  correctionFuelInjOpen();
  
  // Limits
  correctionFuelPWLimit();
}

/** Staging Correction
* This sets the opposing fuel injector as the "staging" injector. To turn on with the primary injector to inject into the same cylinder (or throttle body).
* example: PW1 is the primary. PW3 is the secondary for cylinder 1.
*          PW2 is the primary. PW4 is the secondary for cylinder 2.
*/
void correctionFuelStaging(void)
{
  //Calculate staging pulsewidths if used
  //To run staged injection, the number of cylinders must be less than or equal to the injector channels (ie Assuming you're running paired injection, you need at least as many injector channels as you have cylinders, half for the primaries and half for the secondaries)
  if( (configPage10.stagingEnabled == true) && 
      ((configPage2.nCylinders <= INJ_CHANNELS) || 
       (configPage2.injType == INJ_TYPE_TBODY)) && 
      (currentStatus.BaseFuel > 0) ) //Final check is to ensure that DFCO isn't active, which would cause an overflow below (See #267)
  {
    unsigned long pwLimit_minusInjOpen = pwLimit - inj_opentime_uS; // Staging needs to calculate the limit minus open time since this is added later.
    //Scale the 'full' pulsewidth by each of the injector capacities
    uint32_t tempPW1 = (((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_pri) / 100);

    if(configPage10.stagingMode == STAGING_MODE_TABLE)
    {
      uint32_t tempPW3 = (((unsigned long)currentStatus.PW1 * staged_req_fuel_mult_sec) / 100); //This is ONLY needed in in table mode. Auto mode only calculates the difference.

      byte stagingSplit = get3DTableValue(&stagingTable, currentStatus.MAP, currentStatus.RPM);
      currentStatus.PW1 = ((100 - stagingSplit) * tempPW1) / 100;
      if (currentStatus.PW1 > pwLimit_minusInjOpen) { currentStatus.PW1 = pwLimit_minusInjOpen; } // Check for injection limit

      if(stagingSplit > 0) 
      { 
        currentStatus.PW3 = (stagingSplit * tempPW3) / 100;
        if (currentStatus.PW3 > pwLimit_minusInjOpen) { currentStatus.PW3 = pwLimit_minusInjOpen; } // Also limit staging injector.        
      }
      else { currentStatus.PW3 = 0; }
    }
    else if(configPage10.stagingMode == STAGING_MODE_AUTO)
    {
      currentStatus.PW1 = tempPW1;
      //If automatic mode, the primary injectors are used all the way up to their limit (Configured by the pulsewidth limit setting)
      //If they exceed their limit, the extra duty is passed to the secondaries
      if(tempPW1 > pwLimit_minusInjOpen)
      {
        uint32_t extraPW = tempPW1 - pwLimit_minusInjOpen;
        currentStatus.PW1 = pwLimit_minusInjOpen;
        currentStatus.PW3 = ((extraPW * staged_req_fuel_mult_sec) / staged_req_fuel_mult_pri); //Convert the 'left over' fuel amount from primary injector scaling to secondary
        if (currentStatus.PW3 > pwLimit_minusInjOpen) { currentStatus.PW3 = pwLimit_minusInjOpen; }// Also limit staging injector.
      }
      else { currentStatus.PW3 = 0; } //If tempPW1 < pwLimit it means that the entire fuel load can be handled by the primaries. Simply set the secondaries to 0
    }

  //Set the 2nd channel of each stage with the same pulseWidth
  currentStatus.PW2 = currentStatus.PW1;
  currentStatus.PW4 = currentStatus.PW3;
  }
}

/** Exhaust Gas Oxygen (EGO) Correction for Bank 2
* The base fuel already includes the global correction for EGO. So EGO2 correction here is the difference between G_ego and G_ego2
* Injectors assigned on bank 1 must only align with EGO, Bank 2 must only align with EGO2. 
* Care must be taken if staging or paired injection is enabled that the ego corrections are as intended.
*/
void correctionEGOBank2(void)
{
  if( (configPage6.egoAlgorithm == EGO_ALGORITHM_DUALO2) && (configPage2.injType == INJ_TYPE_PORT) )
  {
    // need to apply the difference between the already applied global G_ego (bank1) and G_ego2 for bank 2 since, all pw already scaled with egoCorrection for bank 1.
    unsigned long pwBank2percentDiff = (100 + currentStatus.ego2Correction) - currentStatus.egoCorrection;
    
    if (pwBank2percentDiff != 100)
    {
      if( (configPage9.injBank_Inj1 == INJ_BANK2) && (channel1InjEnabled == true) ) { currentStatus.PW1 = (pwBank2percentDiff * currentStatus.PW1) / 100; }
      if( (configPage9.injBank_Inj2 == INJ_BANK2) && (channel2InjEnabled == true) ) { currentStatus.PW2 = (pwBank2percentDiff * currentStatus.PW2) / 100; }
      if( (configPage9.injBank_Inj3 == INJ_BANK2) && (channel3InjEnabled == true) ) { currentStatus.PW3 = (pwBank2percentDiff * currentStatus.PW3) / 100; }
      if( (configPage9.injBank_Inj4 == INJ_BANK2) && (channel4InjEnabled == true) ) { currentStatus.PW4 = (pwBank2percentDiff * currentStatus.PW4) / 100; }
      #if INJ_CHANNELS >= 5
      if( (configPage9.injBank_Inj5 == INJ_BANK2) && (channel5InjEnabled == true) ) { currentStatus.PW5 = (pwBank2percentDiff * currentStatus.PW5) / 100; }
      #endif
      #if INJ_CHANNELS >= 6
      if( (configPage9.injBank_Inj6 == INJ_BANK2) && (channel6InjEnabled == true) ) { currentStatus.PW6 = (pwBank2percentDiff * currentStatus.PW6) / 100; }
      #endif
      #if INJ_CHANNELS >= 7
      if( (configPage9.injBank_Inj7 == INJ_BANK2) && (channel7InjEnabled == true) ) { currentStatus.PW7 = (pwBank2percentDiff * currentStatus.PW7) / 100; }
      #endif
      #if INJ_CHANNELS >= 8
      if( (configPage9.injBank_Inj8 == INJ_BANK2) && (channel8InjEnabled == true) ) { currentStatus.PW8 = (pwBank2percentDiff * currentStatus.PW8) / 100; }
      #endif
    }
  }
}

/** Fuel Trim Correction
* Scales the fuel injection ammount via a table indexed by fuel load and rpm for each injector. 
* Care must be taken if staging or paired injection is enabled that the corrections are as intended.
*/
void correctionFuelTrim(void)
{
  if(configPage6.fuelTrimEnabled == true)
  {
    if (channel1InjEnabled == true)
    {
    unsigned long pw1percent = 100 + (byte)get3DTableValue(&trim1Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
    if (pw1percent != 100) { currentStatus.PW1 = (pw1percent * currentStatus.PW1) / 100; }
    }
    
    if (channel2InjEnabled == true)
    {
      unsigned long pw2percent = 100 + (byte)get3DTableValue(&trim2Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw2percent != 100) { currentStatus.PW2 = (pw2percent * currentStatus.PW2) / 100; }
    }
    if (channel3InjEnabled == true)
    {
      unsigned long pw3percent = 100 + (byte)get3DTableValue(&trim3Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw3percent != 100) { currentStatus.PW3 = (pw3percent * currentStatus.PW3) / 100; }
    }
    if (channel4InjEnabled == true)
    {
      unsigned long pw4percent = 100 + (byte)get3DTableValue(&trim4Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw4percent != 100) { currentStatus.PW4 = (pw4percent * currentStatus.PW4) / 100; }
    }

    #if INJ_CHANNELS >= 5
    if (channel5InjEnabled == true)
    {
      unsigned long pw5percent = 100 + (byte)get3DTableValue(&trim5Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw5percent != 100) { currentStatus.PW5 = (pw5percent * currentStatus.PW5) / 100; }
    }
    #endif
    
    #if INJ_CHANNELS >= 6
    if (channel6InjEnabled == true)
    {
      unsigned long pw6percent = 100 + (byte)get3DTableValue(&trim6Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw6percent != 100) { currentStatus.PW6 = (pw6percent * currentStatus.PW6) / 100; }
    }
    #endif
    
    #if INJ_CHANNELS >= 7
    if (channel7InjEnabled == true)
    {
      unsigned long pw7percent = 100 + (byte)get3DTableValue(&trim7Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw7percent != 100) { currentStatus.PW7 = (pw7percent * currentStatus.PW7) / 100; }
    }
    #endif
    
    #if INJ_CHANNELS >= 8
    if (channel8InjEnabled == true)
    {
      unsigned long pw8percent = 100 + (byte)get3DTableValue(&trim8Table, currentStatus.fuelLoad, currentStatus.RPM) - OFFSET_FUELTRIM;
      if (pw8percent != 100) { currentStatus.PW8 = (pw8percent * currentStatus.PW8) / 100; }
    }
    #endif
  }
}

/** Injector open time correction.
* This needs to be done at the last step because the injector pulsewidth up to this point represents fuel as a linear scale for multiplication. After this addition no further multiplication can occur.
*/
void correctionFuelInjOpen(void)
{ 
  //Check each cylinder for injector cutoff and then if running apply the injector open time compensation.  
  if (currentStatus.PW1 > 0) { currentStatus.PW1 += inj_opentime_uS; }
  if (currentStatus.PW2 > 0) { currentStatus.PW2 += inj_opentime_uS; }
  if (currentStatus.PW3 > 0) { currentStatus.PW3 += inj_opentime_uS; }
  if (currentStatus.PW4 > 0) { currentStatus.PW4 += inj_opentime_uS; }
  #if INJ_CHANNELS >= 5
  if (currentStatus.PW5 > 0) { currentStatus.PW5 += inj_opentime_uS; }
  #endif
  #if INJ_CHANNELS >= 6
  if (currentStatus.PW6 > 0) { currentStatus.PW6 += inj_opentime_uS; }
  #endif
  #if INJ_CHANNELS >= 7
  if (currentStatus.PW7 > 0) { currentStatus.PW7 += inj_opentime_uS; }
  #endif
  #if INJ_CHANNELS >= 8  
  if (currentStatus.PW8 > 0) { currentStatus.PW8 += inj_opentime_uS; } 
  #endif
}

void correctionFuelPWLimit(void)
{
  //Apply the pwLimit if staging is dsiabled and engine is not cranking. Staging takes care of it's own PW limit and Cranking is a special case.
  if( (!BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK)) && (configPage10.stagingEnabled == false) ) 
  {
    if (currentStatus.PW1 > pwLimit) { currentStatus.PW1 = pwLimit; }
    if (currentStatus.PW2 > pwLimit) { currentStatus.PW2 = pwLimit; }
    if (currentStatus.PW3 > pwLimit) { currentStatus.PW3 = pwLimit; }
    if (currentStatus.PW4 > pwLimit) { currentStatus.PW4 = pwLimit; }
    #if INJ_CHANNELS >= 5
    if (currentStatus.PW5 > pwLimit) { currentStatus.PW5 = pwLimit; }
    #endif
    #if INJ_CHANNELS >= 6
    if (currentStatus.PW6 > pwLimit) { currentStatus.PW6 = pwLimit; }
    #endif
    #if INJ_CHANNELS >= 7
    if (currentStatus.PW7 > pwLimit) { currentStatus.PW7 = pwLimit; }  
    #endif
    #if INJ_CHANNELS >= 8
    if (currentStatus.PW8 > pwLimit) { currentStatus.PW8 = pwLimit; }
    #endif    
  }
}
  
