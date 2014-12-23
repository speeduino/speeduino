

byte correctionsTotal()
{
  int sumCorrections = 100;
  //sumCorrections = div((sumCorrections * correctionWUE()), 100).quot;
  sumCorrections = div((sumCorrections * correctionASE()), 100).quot;
  //sumCorrections = div((sumCorrections * correctionAccel()), 100).quot;
  sumCorrections = div((sumCorrections * correctionFloodClear()), 100).quot;
  return (byte)sumCorrections;
}

byte correctionWUE()
{
  //Possibly reduce the frequency this runs at (Costs about 50 loops per second)
  return 100 + table2D_getValue(WUETable, currentStatus.coolant);
}

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
*/

byte correctionAccel()
{
  int rateOfChange = ldiv(1000000, (currentLoopTime - previousLoopTime)).quot * (currentStatus.TPS - currentStatus.TPSlast); //This is the % per second that the TPS has moved
  //int rateOfChange = div( (1000000 * (currentStatus.TPS - currentStatus.TPSlast)), (currentLoopTime - previousLoopTime)).quot; //This is the % per second that the TPS has moved
  currentStatus.tpsDOT = div(rateOfChange, 10).quot;
  
  if (rateOfChange > configPage1.tpsThresh)
  {
    return 100 + table2D_getValue(taeTable, currentStatus.tpsDOT);
  }
  
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
