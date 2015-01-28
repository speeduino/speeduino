/*
The corrections functions in this file affect the fuel pulsewidth (Either increasing or decreasing)
based on factors other than the VE lookup.

These factors include temperature (Warmup Enrichment and After Start Enrichment), Acceleration/Decelleration, 
Flood clear mode etc.
*/
//************************************************************************************************************


/*
correctionsTotal() calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
byte correctionsTotal()
{
  int sumCorrections = 100;
  byte result; //temporary variable to store the result of each corrections function
  
  //As the 'normal' case will be for each function to return 100, we only perform the division operation if the returned result is not equal to that
  result = correctionWUE();
  if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  result = correctionASE();
  if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  result = correctionAccel();
  //if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  result = correctionFloodClear();
  if (result != 100) { sumCorrections = div((sumCorrections * result), 100).quot; }
  
  return (byte)sumCorrections;
}

/*
Warm Up Enrichment (WUE)
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
byte correctionWUE()
{
  //Possibly reduce the frequency this runs at (Costs about 50 loops per second)
  if (currentStatus.coolant > (WUETable.axisX[9] - CALIBRATION_TEMPERATURE_OFFSET)) { return 100; } //This prevents us doing the 2D lookup if we're already up to temp
  return table2D_getValue(WUETable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET);
}

/*
After Start Enrichment
This is a short period (Usually <20 seconds) immediately after the engine first fires (But not when cranking)
where an additional amount of fuel is added (Over and above the WUE amount)
*/
byte correctionASE()
{
  
  if (currentStatus.runSecs < configPage1.aseCount) 
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
    if( micros() >= currentStatus.TAEEndTime )
    {
      //Time to turn enrichment off
      BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC);
      currentStatus.TAEamount = 0;
      return 100;
    }
    //Enrichment still needs to keep running. Simply return the total TAE amount
    return 100 + currentStatus.TAEamount; 
  }
  
  //If TAE isn't currently turned on, need to check whether it needs to be turned on
  int rateOfChange = ldiv(1000000, (currentLoopTime - previousLoopTime)).quot * (currentStatus.TPS - currentStatus.TPSlast); //This is the % per second that the TPS has moved
  currentStatus.tpsDOT = divs10(rateOfChange); //The TAE bins are divided by 10 in order to allow them to be stored in a byte. 
  
  if (currentStatus.tpsDOT > (configPage1.tpsThresh * 10))
  {
    BIT_SET(currentStatus.engine, BIT_ENGINE_ACC); //Mark accleration enrichment as active.
    currentStatus.TAEamount = table2D_getValue(taeTable, currentStatus.tpsDOT); //Lookup and store the amount of enrichment required
    currentStatus.TAEEndTime = micros() + (configPage1.taeTime * 100); //Set the time in the future where the enrichment will be turned off. taeTime is stored as mS * 10, so multiply it by 100 to get it in uS
    return 100 + currentStatus.TAEamount;
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
