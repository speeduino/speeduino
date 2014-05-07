

byte correctionsTotal()
{
  int sumCorrections = 100;
  sumCorrections = div((sumCorrections * correctionWUE()), 100).quot;
  sumCorrections = div((sumCorrections * correctionASE()), 100).quot;
  sumCorrections = div((sumCorrections * correctionAccel()), 100).quot;
  sumCorrections = div((sumCorrections * correctionFloodClear()), 100).quot;
  return sumCorrections;
}

byte correctionWUE()
{
  //Not yet implemented
  return 100;
}

byte correctionASE()
{
  
  if (currentStatus.runSecs < configPage1.aseCount) 
  {
    BIT_SET(currentStatus.engine,3); //Mark ASE as active.
    return configPage1.asePct;
  }
  
  BIT_CLEAR(currentStatus.engine,3); //Mark ASE as inactive.
  return 100;
   
  
}

/*
TPS based acceleration enrichment
Calculates the % change of the throttle over time (%/second) and performs a lookup based on this
*/

byte correctionAccel()
{
  int rateOfChange = (1000000 / (currentLoopTime - previousLoopTime)) * (currentStatus.TPS - currentStatus.TPSlast); //This is the % per second that the TPS has moved
  currentStatus.tpsDOT = rateOfChange / 10;
  
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
