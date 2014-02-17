

byte correctionsTotal()
{
  int sumCorrections = (correctionWUE() * correctionASE()) / 100;
  sumCorrections = (sumCorrections * correctionAccel()) / 100;
  return sumCorrections;
}

byte correctionWUE()
{
  
}

byte correctionASE()
{
  
  if (currentStatus.runSecs < configPage1.aseCount) 
  {
    BIT_SET(currentStatus.engine,3); //Mark ASE as active.
    return configPage1.asePct;
  } else
  {
    BIT_CLEAR(currentStatus.engine,3); //Mark ASE as inactive.
  }
   
  
}

/*
TPS based acceleration enrichment
Calculates the % change of the throttle over time (%/second) and performs a lookup based on this
*/

byte correctionAccel()
{
  int rateOfChange = (1000000 / (currentLoopTime - previousLoopTime)) * (currentStatus.TPS - currentStatus.TPSlast); //This is the % per second that the TPS has moved
  
}
