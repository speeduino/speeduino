

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
    return configPage1.asePct;
  }
  
}

byte correctionAccel()
{
  
}
