/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#if defined (__MK20DX256__)
  //tempReading is initated in speeduino.ino. Compiler dont like it here for some reason.
#else
  int tempReading;
#endif

void instanteneousMAPReading()
{
  //Instantaneous MAP readings
  tempReading = analogRead(pinMAP);
  tempReading = analogRead(pinMAP);

  //Error checking
  if(tempReading >= VALID_MAP_MAX || tempReading <= VALID_MAP_MIN) { mapErrorCount += 1; }
  else { currentStatus.mapADC = tempReading; mapErrorCount = 0; }
        
  currentStatus.MAP = fastMap1023toX(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
}

void readMAP()
{
  //MAP Sampling system
  switch(configPage1.mapSample)
  {
    case 0:
      //Instantaneous MAP readings
      instanteneousMAPReading();
      break;
      
    case 1:
      //Average of a cycle
      
      if (currentStatus.RPM < 1) {  instanteneousMAPReading(); return; } //If the engine isn't running, fall back to instantaneous reads
       
      if( (MAPcurRev == startRevolutions) || (MAPcurRev == startRevolutions+1) ) //2 revolutions are looked at for 4 stroke. 2 stroke not currently catered for. 
      {
        tempReading = analogRead(pinMAP);
        tempReading = analogRead(pinMAP);
        
        //Error check
        if(tempReading < VALID_MAP_MAX && tempReading > VALID_MAP_MIN)
        {
          MAPrunningValue = MAPrunningValue + tempReading; //Add the current reading onto the total
          MAPcount++;
        }
        else { mapErrorCount += 1; }
      }
      else
      {
        //Reaching here means that the last cylce has completed and the MAP value should be calculated
        currentStatus.mapADC = ldiv(MAPrunningValue, MAPcount).quot;
        currentStatus.MAP = fastMap1023toX(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
        MAPcurRev = startRevolutions; //Reset the current rev count
        MAPrunningValue = 0;
        MAPcount = 0;
      }
      break;
    
    case 2:
      //Minimum reading in a cycle
      if (currentStatus.RPM < 1) {  instanteneousMAPReading(); return; } //If the engine isn't running, fall back to instantaneous reads
        
      if( (MAPcurRev == startRevolutions) || (MAPcurRev == startRevolutions+1) ) //2 revolutions are looked at for 4 stroke. 2 stroke not currently catered for. 
      {
        tempReading = analogRead(pinMAP);
        tempReading = analogRead(pinMAP);
        //Error check
        if(tempReading < VALID_MAP_MAX && tempReading > VALID_MAP_MIN)
        {
          if( tempReading < MAPrunningValue) { MAPrunningValue = tempReading; } //Check whether the current reading is lower than the running minimum
        }
        else { mapErrorCount += 1; }
      }
      else
      {
        //Reaching here means that the last cylce has completed and the MAP value should be calculated
        currentStatus.mapADC = MAPrunningValue;
        currentStatus.MAP = fastMap1023toX(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
        MAPcurRev = startRevolutions; //Reset the current rev count
        MAPrunningValue = 1023; //Reset the latest value so the next reading will always be lower
      }
      break;
  }
}

void readTPS()
{
  currentStatus.TPSlast = currentStatus.TPS;
  currentStatus.TPSlast_time = currentStatus.TPS_time;
  analogRead(pinTPS);
  byte tempTPS = fastMap1023toX(analogRead(pinTPS), 0, 1023, 0, 255); //Get the current raw TPS ADC value and map it into a byte
  currentStatus.tpsADC = ADC_FILTER(tempTPS, ADCFILTER_TPS, currentStatus.tpsADC);
  //Check that the ADC values fall within the min and max ranges (Should always be the case, but noise can cause these to fluctuate outside the defined range). 
  byte tempADC = currentStatus.tpsADC; //The tempADC value is used in order to allow TunerStudio to recover and redo the TPS calibration if this somehow gets corrupted
  if (currentStatus.tpsADC < configPage1.tpsMin) { tempADC = configPage1.tpsMin; }
  else if(currentStatus.tpsADC > configPage1.tpsMax) { tempADC = configPage1.tpsMax; }
  currentStatus.TPS = map(tempADC, configPage1.tpsMin, configPage1.tpsMax, 0, 100); //Take the raw TPS ADC value and convert it into a TPS% based on the calibrated values
  currentStatus.TPS_time = currentLoopTime;  
}

void readCLT()
{
  tempReading = analogRead(pinCLT);
  tempReading = fastMap1023toX(analogRead(pinCLT), 0, 1023, 0, 511); //Get the current raw CLT value
  currentStatus.cltADC = ADC_FILTER(tempReading, ADCFILTER_CLT, currentStatus.cltADC);
  currentStatus.coolant = cltCalibrationTable[currentStatus.cltADC] - CALIBRATION_TEMPERATURE_OFFSET; //Temperature calibration values are stored as positive bytes. We subtract 40 from them to allow for negative temperatures
}

void readIAT()
{
  tempReading = analogRead(pinIAT);
  tempReading = fastMap1023toX(analogRead(pinIAT), 0, 1023, 0, 511); //Get the current raw IAT value
  currentStatus.iatADC = ADC_FILTER(tempReading, ADCFILTER_IAT, currentStatus.iatADC);
  currentStatus.IAT = iatCalibrationTable[currentStatus.iatADC] - CALIBRATION_TEMPERATURE_OFFSET;
}

void readO2()
{
  tempReading = analogRead(pinO2);
  tempReading = fastMap1023toX(analogRead(pinO2), 0, 1023, 0, 511); //Get the current O2 value. 
  currentStatus.O2ADC = ADC_FILTER(tempReading, ADCFILTER_O2, currentStatus.O2ADC);
  currentStatus.O2 = o2CalibrationTable[currentStatus.O2ADC];
}
       
/* Second O2 currently disabled as its not being used
  currentStatus.O2_2ADC = map(analogRead(pinO2_2), 0, 1023, 0, 511); //Get the current O2 value.
  currentStatus.O2_2ADC = ADC_FILTER(tempReading, ADCFILTER_O2, currentStatus.O2_2ADC);
  currentStatus.O2_2 = o2CalibrationTable[currentStatus.O2_2ADC];
*/

//Flex fuel sensor for E85 reading
void readFLEX(){
  tempReading = analogRead(pinFlexFuel);
  tempReading = fastMap1023toX(analogRead(pinFlexFuel), 0, 1023, 0, 511);
  currentStatus.flexADC = ADC_FILTER(tempReading, ADCFILTER_FLEX, currentStatus.flexADC);  
}

void readBat()
{
  tempReading = analogRead(pinBat);
  tempReading = fastMap1023toX(analogRead(pinBat), 0, 1023, 0, 245); //Get the current raw Battery value. Permissible values are from 0v to 24.5v (245)
  currentStatus.battery10 = ADC_FILTER(tempReading, ADCFILTER_BAT, currentStatus.battery10);
}

