/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

void instanteneousMAPReading()
{
  //Instantaneous MAP readings
  int tempReading;
  tempReading = analogRead(pinMAP);
  tempReading = analogRead(pinMAP);

  //Error checking
  if(tempReading >= VALID_MAP_MAX || tempReading <= VALID_MAP_MIN) { mapErrorCount += 1; }
  else { currentStatus.mapADC = tempReading; mapErrorCount = 0; }
        
  currentStatus.MAP = map(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
}

void readMAP()
{
    //MAP Sampling system
    int tempReading;
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
          currentStatus.MAP = map(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
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
          currentStatus.MAP = map(currentStatus.mapADC, 0, 1023, configPage1.mapMin, configPage1.mapMax); //Get the current MAP value
          MAPcurRev = startRevolutions; //Reset the current rev count
          MAPrunningValue = 1023; //Reset the latest value so the next reading will always be lower
        }
        break;
    }
}

