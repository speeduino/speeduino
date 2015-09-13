/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/*
This file contains the various crank and cam wheel decoder functions.

Each decoder must have the following 4 functions (Where xxxx is the decoder name):

* triggerSetup_xxx - Called once from within setup() and configures any required variables
* triggerPri_xxxx - Called each time the primary (No. 1) crank/cam signal is triggered (Called as an interrupt, so variables must be declared volatile)
* triggerSec_xxxx - Called each time the secondary (No. 2) crank/cam signal is triggered (Called as an interrupt, so variables must be declared volatile)
* getRPM_xxxx - Returns the current RPM, as calculated by the decoder
* getCrankAngle_xxxx - Returns the current crank angle, as calculated b the decoder

And each decoder must utlise at least the following variables:
toothLastToothTime - The time (In uS) that the last primary tooth was 'seen'
* 

*/

/* 
Name: Missing tooth wheel
Desc: A multi-tooth wheel with one of more 'missing' teeth. The first tooth after the missing one is considered number 1 and isthe basis for the trigger angle
Note: This does not currently support dual wheel (ie missing tooth + single tooth on cam)
*/
void triggerSetup_missingTooth()
{
  triggerToothAngle = 360 / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth
  triggerActualTeeth = configPage2.triggerTeeth - configPage2.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
}

void triggerPri_missingTooth()
{
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm

   curTime = micros();
   curGap = curTime - toothLastToothTime;
   if ( curGap < triggerFilterTime ) { return; } //Debounce check. Pulses should never be less than triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
   toothCurrentCount++; //Increment the tooth counter
   
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   //Begin the missing tooth detection
   //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
   //if ( (curTime - toothLastToothTime) > (1.5 * (toothLastToothTime - toothLastMinusOneToothTime))) { toothCurrentCount = 1; }
   if(configPage2.triggerMissingTeeth == 1) { targetGap = (3 * (toothLastToothTime - toothLastMinusOneToothTime)) >> 1; } //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
   //else { targetGap = (10 * (toothLastToothTime - toothLastMinusOneToothTime)) >> 2; } //Multiply by 2.5 (Checks for a gap 2.5x greater than the last one)
   else { targetGap = ((toothLastToothTime - toothLastMinusOneToothTime)) * 2; } //Multiply by 2 (Checks for a gap 2x greater than the last one)
   if ( curGap > targetGap || toothCurrentCount > triggerActualTeeth)
   { 
     toothCurrentCount = 1; 
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     startRevolutions++; //Counter 
   } 
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}

void triggerSec_missingTooth(){ return; } //This function currently is not used

int getRPM_missingTooth()
{
   noInterrupts();
   revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
   interrupts(); 
   return (US_IN_MINUTE / revolutionTime); //Calc RPM based on last full revolution time (Faster as /)
}

int getCrankAngle_missingTooth(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables. 
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();
    
    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    crankAngle += ( (micros() - tempToothLastToothTime) / timePerDegree); //Estimate the number of degrees travelled since the last tooth

    if (crankAngle > 360) { crankAngle -= 360; }
    
    return crankAngle;
}

/* 
Name: Missing tooth wheel
Desc: A multi-tooth wheel with one of more 'missing' teeth. The first tooth after the missing one is considered number 1 and isthe basis for the trigger angle
Note: This does not currently support dual wheel (ie missing tooth + single tooth on cam)
*/
void triggerPri_DualWheel()
{ 
  return;
}


/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Basic Distributor
Desc: Tooth equal to the number of cylinders are evenly spaced on the cam. No position sensing (Distributor is retained) so crank angle is a made up figure based purely on the first teeth to be seen
Note: This is a very simple decoder. See http://www.megamanual.com/ms2/GM_7pinHEI.htm
*/
void triggerSetup_BasicDistributor()
{
  triggerToothAngle = 360 / (configPage1.nCylinders / 2); //The number of degrees that passes from tooth to tooth
}

void triggerPri_BasicDistributor()
{
  curTime = micros();
  
  toothCurrentCount++; //Increment the tooth counter
  if(toothCurrentCount > (configPage1.nCylinders >> 1) ) //Check if we're back to the beginning of a revolution
  { 
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     startRevolutions++; //Counter 
  } 
  
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}
void triggerSec_BasicDistributor() { return; } //Not required
int getRPM_BasicDistributor()
{
   noInterrupts();
   revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
   interrupts(); 
   return ldiv(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time (uses ldiv rather than div as US_IN_MINUTE is a long) 
}
int getCrankAngle_BasicDistributor(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables. 
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();
    
    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    crankAngle += ldiv( (micros() - tempToothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
    if (crankAngle > 360) { crankAngle -= 360; }
    
    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: GM7X
Desc: GM 7X trigger wheel. It has six equally spaced teeth and a seventh tooth for cylinder identification.
Note: Within the code below, the sync tooth is referred to as tooth #3 rather than tooth #7. This makes for simpler angle calculations
*/
void triggerSetup_GM7X()
{
  triggerToothAngle = 360 / 6; //The number of degrees that passes from tooth to tooth
}

void triggerPri_GM7X()
{
   lastGap = curGap;
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   toothCurrentCount++; //Increment the tooth counter
   
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   //
   if( toothCurrentCount > 7 )
   {
     toothCurrentCount = 1; 
     toothOneMinusOneTime = toothOneTime; 
     toothOneTime = curTime;
   }
   else
   {
     targetGap = (lastGap) >> 1; //The target gap is set at half the last tooth gap
     if ( curGap < targetGap) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the magical 3rd tooth
     { 
       toothCurrentCount = 3; 
       currentStatus.hasSync = true;
       startRevolutions++; //Counter 
     } 
   }
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}
void triggerSec_GM7X() { return; } //Not required
int getRPM_GM7X()
{
   noInterrupts();
   revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
   interrupts(); 
   return ldiv(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time (uses ldiv rather than div as US_IN_MINUTE is a long) 
}
int getCrankAngle_GM7X(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables. 
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();
    
    //Check if the last tooth seen was the reference tooth (Number 3). All others can be calculated, but tooth 3 has a unique angle
    int crankAngle;
    if( tempToothCurrentCount < 3 )
    {
      crankAngle = (tempToothCurrentCount - 1) * 60 + 42; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    }
    else if( tempToothCurrentCount == 3 )
    {
      crankAngle = 112;
    }
    else
    {
      crankAngle = (tempToothCurrentCount - 2) * 60 + 42; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    }
    
    crankAngle += ldiv( (micros() - tempToothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
    if (crankAngle > 360) { crankAngle -= 360; }
    
    return crankAngle;
}


/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Mitsubishi 4G63 / NA/NB Miata + MX-5 / 4/2
Desc: TBA
Note: https://raw.githubusercontent.com/noisymime/speeduino/master/reference/wiki/decoders/4g63_trace.png
Tooth #1 is defined as the next crank tooth after the crank signal is LOW when the cam signal is rising.
Tooth number one is at 355* ATDC
*/
void triggerSetup_4G63()
{
  triggerToothAngle = 180; //The number of degrees that passes from tooth to tooth (primary)
  toothCurrentCount = 99; //Fake tooth count represents no sync
  
  //Note that these angles are for every rising and falling edge
  
  toothAngles[0] = 355; //Falling edge of tooth #1
  toothAngles[1] = 105; //Rising edge of tooth #2
  toothAngles[2] = 175; //Falling edge of tooth #2
  toothAngles[3] = 285; //Rising edge of tooth #1
  
  triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth. 
}

void triggerPri_4G63()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap < triggerFilterTime ) { return; } //Debounce check. Pulses should never be less than triggerFilterTime
  
  if(toothCurrentCount == 0 || toothCurrentCount == 4)
  { 
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     startRevolutions++; //Counter 
  }
  else  { toothCurrentCount++; }
  
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}
void triggerSec_4G63()
{ 
  if(!currentStatus.hasSync)
  {
    //Check the status of the crank trigger
    bool crank = digitalRead(pinTrigger);
    if(crank == LOW) { toothCurrentCount = 0; } //If the crank trigger is currently HIGH, it means we're on tooth #1
  }
  return; 
}


int getRPM_4G63()
{
   noInterrupts();
   revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
   interrupts(); 
   return ldiv(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time (uses ldiv rather than div as US_IN_MINUTE is a long) 
}
int getCrankAngle_4G63(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables. 
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();
    
    int crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage2.triggerAngle; //Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was. 
    crankAngle += ldiv( (micros() - tempToothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
    if (crankAngle > 360) { crankAngle -= 360; }
    
    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: GM 24X
Desc: TBA
Note: Useful references:
http://www.vems.hu/wiki/index.php?page=MembersPage%2FJorgenKarlsson%2FTwentyFourX
Provided that the cam signal is used, this decoder simply counts the teeth and then looks their angles up against a lookup table. The cam signal is used to determine tooth #1
*/
void triggerSetup_24X()
{
  triggerToothAngle = 180; //The number of degrees that passes from tooth to tooth (primary)
  toothAngles[0] = 12;
  toothAngles[1] = 18;
  toothAngles[2] = 33;
  toothAngles[3] = 48;
  toothAngles[4] = 63;
  toothAngles[5] = 78;
  toothAngles[6] = 102;
  toothAngles[7] = 108;
  toothAngles[8] = 123;
  toothAngles[9] = 138;
  toothAngles[10] = 162;
  toothAngles[11] = 177;
  toothAngles[12] = 183;
  toothAngles[13] = 198;
  toothAngles[14] = 222;
  toothAngles[15] = 237;
  toothAngles[16] = 252;
  toothAngles[17] = 258;
  toothAngles[18] = 282;
  toothAngles[19] = 288;
  toothAngles[20] = 312;
  toothAngles[21] = 327;
  toothAngles[22] = 342;
  toothAngles[23] = 357;
  
  toothCurrentCount = 25; //We set the initial tooth value to be something that should never be reached. This indicates no sync
}

void triggerPri_24X()
{
  if(toothCurrentCount == 25) { currentStatus.hasSync = false; return; } //Indicates sync has not been achieved (Still waiting for 1 revolution of the crank to take place)
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  
  if(toothCurrentCount == 0)
  { 
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     startRevolutions++; //Counter 
  }
  else
  {
    toothCurrentCount++; //Increment the tooth counter
  }
  
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   toothLastToothTime = curTime;
}
void triggerSec_24X()
{ 
  toothCurrentCount = 0; //All we need to do is reset the tooth count back to zero, indicating that we're at the beginning of a new revolution
  return; 
}

int getRPM_24X()
{
   noInterrupts();
   revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
   interrupts(); 
   return ldiv(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time (uses ldiv rather than div as US_IN_MINUTE is a long) 
}
int getCrankAngle_24X(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables. 
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();
    
    int crankAngle;
    if (toothCurrentCount == 0) { crankAngle = 0 + configPage2.triggerAngle; } //This is the special case to handle when the 'last tooth' seen was the cam tooth. 0 is the angle at which the crank tooth goes high (Within 360 degrees). 
    else { crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage2.triggerAngle;} //Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was. 
    crankAngle += ldiv( (micros() - tempToothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
    if (crankAngle > 360) { crankAngle -= 360; }
    
    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Jeep 2000
Desc: For '91 to 2000 6 cylinder Jeep engines
Note: Quite similar to the 24X setup. 24 crank teeth over 720 degrees, in groups of 4. Crank wheel is high for 360 crank degrees. AS we only need timing within 360 degrees, only 12 tooth angles are defined.
Tooth number 1 represents the first tooth seen after the cam signal goes high
http://speeduino.com/forum/download/file.php?id=205
*/
void triggerSetup_Jeep2000()
{
  triggerToothAngle = 180; //The number of degrees that passes from tooth to tooth (primary)
  toothAngles[0] = 174;
  toothAngles[1] = 194;
  toothAngles[2] = 214;
  toothAngles[3] = 234;
  toothAngles[4] = 294;
  toothAngles[5] = 314;
  toothAngles[6] = 334;
  toothAngles[7] = 354;
  toothAngles[8] = 414;
  toothAngles[9] = 434;
  toothAngles[10] = 454;
  toothAngles[11] = 474;
  
  toothCurrentCount = 13; //We set the initial tooth value to be something that should never be reached. This indicates no sync
}

void triggerPri_Jeep2000()
{
  if(toothCurrentCount == 13) { currentStatus.hasSync = false; return; } //Indicates sync has not been achieved (Still waiting for 1 revolution of the crank to take place)
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  
  if(toothCurrentCount == 0)
  { 
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     startRevolutions++; //Counter 
  }
  else
  {
    toothCurrentCount++; //Increment the tooth counter
  }
  
   //High speed tooth logging history
   toothHistory[toothHistoryIndex] = curGap;
   if(toothHistoryIndex == 511)
   { toothHistoryIndex = 0; }
   else
   { toothHistoryIndex++; }
   
   toothLastToothTime = curTime;
}
void triggerSec_Jeep2000()
{ 
  toothCurrentCount = 0; //All we need to do is reset the tooth count back to zero, indicating that we're at the beginning of a new revolution
  return; 
}

int getRPM_Jeep2000()
{
   noInterrupts();
   revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
   interrupts(); 
   return ldiv(US_IN_MINUTE, revolutionTime).quot; //Calc RPM based on last full revolution time (uses ldiv rather than div as US_IN_MINUTE is a long) 
}
int getCrankAngle_Jeep2000(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables. 
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();
    
    int crankAngle;
    if (toothCurrentCount == 0) { crankAngle = 146 + configPage2.triggerAngle; } //This is the special case to handle when the 'last tooth' seen was the cam tooth. 146 is the angle at which the crank tooth goes high. 
    else { crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage2.triggerAngle;} //Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was. 
    crankAngle += ldiv( (micros() - tempToothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth
    if (crankAngle > 360) { crankAngle -= 360; }
    
    return crankAngle;
}

