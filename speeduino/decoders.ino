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
#include "decoders.h"

static inline void addToothLogEntry(unsigned long toothTime)
{
  //High speed tooth logging history
  toothHistory[toothHistoryIndex] = toothTime;
  if(toothHistoryIndex == (TOOTH_LOG_BUFFER-1))
  {
    if (toothLogRead)
    {
      toothHistoryIndex = 0;
      BIT_CLEAR(currentStatus.squirt, BIT_SQUIRT_TOOTHLOG1READY);
      toothLogRead = false; //The tooth log ready bit is cleared to ensure that we only get a set of concurrent values.
    }
  }
  else
  { toothHistoryIndex++; }
}

/*
As nearly all the decoders use a common method of determining RPM (The time the last full revolution took)
A common function is simpler
*/
static inline int stdGetRPM()
{
  if( !currentStatus.hasSync ) { return 0; } //Safety check
  noInterrupts();
  revolutionTime = (toothOneTime - toothOneMinusOneTime); //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
  interrupts();
  int tempRPM = (US_IN_MINUTE / revolutionTime); //Calc RPM based on last full revolution time (Faster as /)
  if(tempRPM >= MAX_RPM) { return currentStatus.RPM; } //Sanity check
  return tempRPM;
}

/*
 * Sets the new filter time based on the current settings.
 * This ONLY works for even spaced decoders
 */
static inline void setFilter(unsigned long curGap)
{
   if(configPage2.triggerFilter == 1) { triggerFilterTime = curGap >> 2; } //Lite filter level is 25% of previous gap
   else if(configPage2.triggerFilter == 2) { triggerFilterTime = curGap >> 1; } //Medium filter level is 50% of previous gap
   else if (configPage2.triggerFilter == 3) { triggerFilterTime = (curGap * 3) >> 2; } //Aggressive filter level is 75% of previous gap
   else { triggerFilterTime = 0; } //trigger filter is turned off.
}

/*
This is a special case of RPM measure that is based on the time between the last 2 teeth rather than the time of the last full revolution
This gives much more volatile reading, but is quite useful during cranking, particularly on low resolution patterns.
It can only be used on patterns where the teeth are evently spaced
It takes an argument of the full (COMPLETE) number of teeth per revolution. For a missing tooth wheel, this is the number if the tooth had NOT been missing (Eg 36-1 = 36)
*/
static inline int crankingGetRPM(byte totalTeeth)
{
  noInterrupts();
  revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime) * totalTeeth;
  interrupts();
  return (US_IN_MINUTE / revolutionTime);
}

/*
Name: Missing tooth wheel
Desc: A multi-tooth wheel with one of more 'missing' teeth. The first tooth after the missing one is considered number 1 and isthe basis for the trigger angle
Note: This does not currently support dual wheel (ie missing tooth + single tooth on cam)
*/
void triggerSetup_missingTooth()
{
  triggerToothAngle = 360 / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth
  if(configPage2.TrigSpeed) { triggerToothAngle = 720 / configPage2.triggerTeeth; } //Account for cam speed missing tooth
  triggerActualTeeth = configPage2.triggerTeeth - configPage2.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage2.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise
  secondDerivEnabled = false;
  decoderIsSequential = false;
  checkSyncToothCount = (configPage2.triggerTeeth) >> 1; //50% of the total teeth.
  MAX_STALL_TIME = (3333UL * triggerToothAngle * (configPage2.triggerMissingTeeth + 1)); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
}

void triggerPri_missingTooth()
{
   // http://www.msextra.com/forums/viewtopic.php?f=94&t=22976
   // http://www.megamanual.com/ms2/wheel.htm

   curTime = micros();
   curGap = curTime - toothLastToothTime;
   if ( curGap < triggerFilterTime ) { return; } //Debounce check. Pulses should never be less than triggerFilterTime, so if they are it means a false trigger. (A 36-1 wheel at 8000pm will have triggers approx. every 200uS)
   toothCurrentCount++; //Increment the tooth counter

   addToothLogEntry(curGap);

   //if(toothCurrentCount > checkSyncToothCount || !currentStatus.hasSync)
   {
     //Begin the missing tooth detection
     //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
     if(configPage2.triggerMissingTeeth == 1) { targetGap = (3 * (toothLastToothTime - toothLastMinusOneToothTime)) >> 1; } //Multiply by 1.5 (Checks for a gap 1.5x greater than the last one) (Uses bitshift to multiply by 3 then divide by 2. Much faster than multiplying by 1.5)
     else { targetGap = ((toothLastToothTime - toothLastMinusOneToothTime)) * 2; } //Multiply by 2 (Checks for a gap 2x greater than the last one)

     if ( curGap > targetGap || toothCurrentCount > triggerActualTeeth)
     {
       if(toothCurrentCount < (triggerActualTeeth) && currentStatus.hasSync) { currentStatus.hasSync = false; return; } //This occurs when we're at tooth #1, but haven't seen all the other teeth. This indicates a signal issue so we flag lost sync so this will attempt to resync on the next revolution.
       toothCurrentCount = 1;
       revolutionOne = !revolutionOne; //Flip sequential revolution tracker
       toothOneMinusOneTime = toothOneTime;
       toothOneTime = curTime;
       currentStatus.hasSync = true;
       currentStatus.startRevolutions++; //Counter
       triggerFilterTime = 0; //This is used to prevent a condition where serious intermitent signals (Eg someone furiously plugging the sensor wire in and out) can leave the filter in an unrecoverable state
     }
     else
     {
       //Filter can only be recalc'd for the regular teeth, not the missing one.
       setFilter(curGap);
     }
   }

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}

void triggerSec_missingTooth()
{
  //TODO: This should really have filtering enabled on the secondary input.
  revolutionOne = 1;
}

int getRPM_missingTooth()
{
  if(configPage2.TrigSpeed) { return (stdGetRPM() * 2); } //Account for cam speed
  return stdGetRPM();
}

int getCrankAngle_missingTooth(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    bool tempRevolutionOne;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    tempRevolutionOne = revolutionOne;
    interrupts();

    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = (micros() - tempToothLastToothTime);
    //crankAngle += DIV_ROUND_CLOSEST(elapsedTime, timePerDegree);
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    //Sequential check (simply sets whether we're on the first or 2nd revoltuion of the cycle)
    if (tempRevolutionOne) { crankAngle += 360; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    else if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Dual wheel
Desc: 2 wheels located either both on the crank or with the primary on the crank and the secondary on the cam.
Note: There can be no missing teeth on the primary wheel
*/
void triggerSetup_DualWheel()
{
  triggerToothAngle = 360 / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth
  toothCurrentCount = 255; //Default value
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage2.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  secondDerivEnabled = false;
  decoderIsSequential = true;
  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
}


void triggerPri_DualWheel()
{
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   if ( curGap < triggerFilterTime ) { return; } //Pulses should never be less than triggerFilterTime, so if they are it means a false trigger.
   toothCurrentCount++; //Increment the tooth counter
   addToothLogEntry(curGap);

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;

   if ( !currentStatus.hasSync ) { return; }

   if ( toothCurrentCount == 1 || toothCurrentCount > configPage2.triggerTeeth )
   {
     toothCurrentCount = 1;
     revolutionOne = !revolutionOne; //Flip sequential revolution tracker
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.startRevolutions++; //Counter
   }

   setFilter(curGap); //Recalc the new filter value


}

void triggerSec_DualWheel()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  if ( curGap2 < triggerSecFilterTime ) { return; }
  toothLastSecToothTime = curTime2;
  triggerSecFilterTime = curGap2 >> 2; //Set filter at 25% of the current speed

  if(!currentStatus.hasSync)
  {
    toothLastToothTime = micros();
    toothLastMinusOneToothTime = (toothOneTime - 6000000) / configPage2.triggerTeeth; //Fixes RPM at 10rpm until a full revolution has taken place
    toothCurrentCount = configPage2.triggerTeeth;

    currentStatus.hasSync = true;
  }
  else if (configPage2.useResync) { toothCurrentCount = configPage2.triggerTeeth; }

  revolutionOne = 1; //Sequential revolution reset
}

int getRPM_DualWheel()
{
  if( !currentStatus.hasSync) { return 0; }
  if(currentStatus.RPM < configPage2.crankRPM) { return crankingGetRPM(configPage2.triggerTeeth); }
  return stdGetRPM();
}

int getCrankAngle_DualWheel(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    bool tempRevolutionOne;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    tempRevolutionOne = revolutionOne;
    interrupts();

    //Handle case where the secondary tooth was the last one seen
    if(tempToothCurrentCount == 0) { tempToothCurrentCount = configPage2.triggerTeeth; }

    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    //Sequential check (simply sets whether we're on the first or 2nd revoltuion of the cycle)
    if (tempRevolutionOne) { crankAngle += 360; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}


/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Basic Distributor
Desc: Tooth equal to the number of cylinders are evenly spaced on the cam. No position sensing (Distributor is retained) so crank angle is a made up figure based purely on the first teeth to be seen
Note: This is a very simple decoder. See http://www.megamanual.com/ms2/GM_7pinHEI.htm
*/
void triggerSetup_BasicDistributor()
{
  triggerActualTeeth = configPage1.nCylinders;
  if(triggerActualTeeth == 0) { triggerActualTeeth = 1; }
  //triggerToothAngle = 360 / triggerActualTeeth; //The number of degrees that passes from tooth to tooth
  triggerToothAngle = 720 / triggerActualTeeth; //The number of degrees that passes from tooth to tooth
  triggerFilterTime = 60000000L / MAX_RPM / configPage1.nCylinders; // Minimum time required between teeth
  triggerFilterTime = triggerFilterTime / 2; //Safety margin
  secondDerivEnabled = false;
  decoderIsSequential = false;
  MAX_STALL_TIME = (1851UL * triggerToothAngle); //Minimum 90rpm. (1851uS is the time per degree at 90rpm). This decoder uses 90rpm rather than 50rpm due to the potentially very high stall time on a 4 cylinder if we wait that long.
}

void triggerPri_BasicDistributor()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap < triggerFilterTime ) { return; } //Noise rejection check. Pulses should never be less than triggerFilterTime

  if(toothCurrentCount == triggerActualTeeth ) //Check if we're back to the beginning of a revolution
  {
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     currentStatus.startRevolutions++; //Counter
  }
  else
  {
    toothCurrentCount++; //Increment the tooth counter
  }

  setFilter(curGap); //Recalc the new filter value
  addToothLogEntry(curGap);

  if ( configPage2.ignCranklock && BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
  {
    endCoil1Charge();
    endCoil2Charge();
    endCoil3Charge();
    endCoil4Charge();
  }

  toothLastMinusOneToothTime = toothLastToothTime;
  toothLastToothTime = curTime;
}
void triggerSec_BasicDistributor() { return; } //Not required
int getRPM_BasicDistributor()
{
  uint16_t tempRPM;
  if(currentStatus.RPM < configPage2.crankRPM)
  { tempRPM = crankingGetRPM(triggerActualTeeth); }
  else
  { tempRPM = stdGetRPM(); }

  return tempRPM << 1; //Multiply RPM by 2 due to tracking over 720 degrees now rather than 360
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

    //int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    //crankAngle += ldiv( (micros() - tempToothLastToothTime), timePerDegree).quot; //Estimate the number of degrees travelled since the last tooth


    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

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
  secondDerivEnabled = false;
  decoderIsSequential = false;
  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
}

void triggerPri_GM7X()
{
   lastGap = curGap;
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   toothCurrentCount++; //Increment the tooth counter

   addToothLogEntry(curGap);

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
       currentStatus.startRevolutions++; //Counter
       return; //We return here so that the tooth times below don't get set (The magical 3rd tooth should not be considered for any calculations that use those times)
     }
   }

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}
void triggerSec_GM7X() { return; } //Not required
int getRPM_GM7X()
{
   return stdGetRPM();
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
      crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + 42; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    }
    else if( tempToothCurrentCount == 3 )
    {
      crankAngle = 112;
    }
    else
    {
      crankAngle = (tempToothCurrentCount - 2) * triggerToothAngle + 42; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    }

    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}


/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Mitsubishi 4G63 / NA/NB Miata + MX-5 / 4/2
Desc: TBA
Note: https://raw.githubusercontent.com/noisymime/speeduino/master/reference/wiki/decoders/4g63_trace.png
Tooth #1 is defined as the next crank tooth after the crank signal is HIGH when the cam signal is falling.
Tooth number one is at 355* ATDC
*/
void triggerSetup_4G63()
{
  triggerToothAngle = 180; //The number of degrees that passes from tooth to tooth (primary)
  toothCurrentCount = 99; //Fake tooth count represents no sync
  secondDerivEnabled = false;
  decoderIsSequential = true;
  MAX_STALL_TIME = 366667UL; //Minimum 50rpm based on the 110 degree tooth spacing
  toothLastToothTime = micros(); //Set a startup value here to avoid filter errors when starting

  //Note that these angles are for every rising and falling edge

  toothAngles[0] = 355; //Falling edge of tooth #1
  toothAngles[1] = 105; //Rising edge of tooth #2
  toothAngles[2] = 175; //Falling edge of tooth #2
  toothAngles[3] = 285; //Rising edge of tooth #1

  /*
   * https://forums.libreems.org/attachment.php?aid=34
  toothAngles[0] = 715; //Falling edge of tooth #1
  toothAngles[1] = 49;  //Falling edge of wide cam
  toothAngles[2] = 105; //Rising edge of tooth #2
  toothAngles[3] = 175; //Falling edge of tooth #2
  toothAngles[4] = 229; //Rising edge of narrow cam tooth (??)
  toothAngles[5] = 285; //Rising edge of tooth #3
  toothAngles[6] = 319; //Falling edge of narrow cam tooth
  toothAngles[7] = 355; //falling edge of tooth #3
  toothAngles[8] = 465; //Rising edge of tooth #4
  toothAngles[9] = 535; //Falling edge of tooth #4
  toothAngles[10] = 535; //Rising edge of wide cam tooth
  toothAngles[11] = 645; //Rising edge of tooth #1
   */

  triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  triggerSecFilterTime_duration = 4000;
  secondaryLastToothTime = 0;
}

void triggerPri_4G63()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap < triggerFilterTime ) { return; } //Filter check. Pulses should never be less than triggerFilterTime

  addToothLogEntry(curGap);
  triggerFilterTime = curGap >> 2; //This only applies during non-sync conditions. If there is sync then triggerFilterTime gets changed again below with a better value.

  toothLastMinusOneToothTime = toothLastToothTime;
  toothLastToothTime = curTime;

  toothCurrentCount++;
  if(toothCurrentCount == 1 || toothCurrentCount > 4) //Trigger is on CHANGE, hence 4 pulses = 1 crank rev
  {
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     currentStatus.startRevolutions++; //Counter
     //if ((startRevolutions & 15) == 1) { currentStatus.hasSync = false; } //Every 64 revolutions, force a resync with the cam
  }
  else if (!currentStatus.hasSync) { return; }

  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && configPage2.ignCranklock)
  {
    if( toothCurrentCount == 1 ) { endCoil1Charge(); }
    else if( toothCurrentCount == 3 ) { endCoil2Charge(); }
  }

  //Whilst this is an uneven tooth pattern, if the specific angle between the last 2 teeth is specified, 1st deriv prediction can be used
  if(configPage2.triggerFilter == 1 || currentStatus.RPM < 1800)
  {
    //Lite filter
    if(toothCurrentCount == 1 || toothCurrentCount == 3) { triggerToothAngle = 70; triggerFilterTime = curGap; } //Trigger filter is set to whatever time it took to do 70 degrees (Next trigger is 110 degrees away)
    else { triggerToothAngle = 110; triggerFilterTime = (curGap * 3) >> 3; } //Trigger filter is set to (110*3)/8=41.25=41 degrees (Next trigger is 70 degrees away).
  }
  else if(configPage2.triggerFilter == 2)
  {
    //Medium filter level
    if(toothCurrentCount == 1 || toothCurrentCount == 3) { triggerToothAngle = 70; triggerFilterTime = (curGap * 5) >> 2 ; } //87.5 degrees with a target of 110
    else { triggerToothAngle = 110; triggerFilterTime = (curGap >> 1); } //55 degrees with a target of 70
  }
  else if (configPage2.triggerFilter == 3)
  {
    //Aggressive filter level
    if(toothCurrentCount == 1 || toothCurrentCount == 3) { triggerToothAngle = 70; triggerFilterTime = (curGap * 11) >> 3 ; } //96.26 degrees with a target of 110
    else { triggerToothAngle = 110; triggerFilterTime = (curGap * 9) >> 5; } //61.87 degrees with a target of 70
  }
  else { triggerFilterTime = 0; } //trigger filter is turned off.

}
void triggerSec_4G63()
{
  //byte crankState = READ_PRI_TRIGGER();
  //First filter is a duration based one to ensure the pulse was of sufficient length (time)
  //if(READ_SEC_TRIGGER()) { secondaryLastToothTime1 = micros(); return; }
  if(currentStatus.hasSync)
  {
  //if ( (micros() - secondaryLastToothTime1) < triggerSecFilterTime_duration ) { return; } //1166 is the time taken to cross 70 degrees at 10k rpm
  //triggerSecFilterTime_duration = (micros() - secondaryLastToothTime1) >> 1;
  }


  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  if ( curGap2 < triggerSecFilterTime ) { return; }
  toothLastSecToothTime = curTime2;

  triggerSecFilterTime = curGap2 >> 1; //Basic 50% filter for the secondary reading
  //triggerSecFilterTime = (curGap2 * 9) >> 5; //62.5%
  //triggerSecFilterTime = (curGap2 * 6) >> 3; //75%

  if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) || !currentStatus.hasSync)
  {
    triggerFilterTime = 1500; //If this is removed, can have trouble getting sync again after the engine is turned off (but ECU not reset).
    if(READ_PRI_TRIGGER())// && (crankState == digitalRead(pinTrigger)))
    {
      toothCurrentCount = 4; //If the crank trigger is currently HIGH, it means we're on tooth #1
    }
}

  if ( (micros() - secondaryLastToothTime1) < triggerSecFilterTime_duration )
  {
    triggerSecFilterTime_duration = (micros() - secondaryLastToothTime1) >> 1;
    if(READ_PRI_TRIGGER())// && (crankState == digitalRead(pinTrigger)))
    {
      //toothCurrentCount = 4; //If the crank trigger is currently HIGH, it means we're on tooth #1

      /* High-res mode
      toothCurrentCount = 7; //If the crank trigger is currently HIGH, it means we're on the falling edge of the narrow crank tooth
      toothLastMinusOneToothTime = toothLastToothTime;
      toothLastToothTime = curTime;
      */
    }
  }

  return;
}


int getRPM_4G63()
{
  //During cranking, RPM is calculated 4 times per revolution, once for each rising/falling of the crank signal.
  //Because these signals aren't even (Alternating 110 and 70 degrees), this needs a special function
  if(!currentStatus.hasSync) { return 0; }
  if(currentStatus.RPM < configPage2.crankRPM)
  {
    if(currentStatus.startRevolutions < 2) { return 0; } //Need at least 2 full revolutions to prevent crazy initial rpm value
    int tempToothAngle;
    noInterrupts();
    tempToothAngle = triggerToothAngle;
    /* High-res mode
    if(toothCurrentCount == 1) { tempToothAngle = 70; }
    else { tempToothAngle = toothAngles[toothCurrentCount-1] - toothAngles[toothCurrentCount-2]; }
    */
    revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime); //Note that trigger tooth angle changes between 70 and 110 depending on the last tooth that was seen
    interrupts();
    revolutionTime = revolutionTime * 36;
    int tempRPM = ((unsigned long)tempToothAngle * 6000000UL) / revolutionTime;
    return tempRPM;
  }
  else { return stdGetRPM(); }
}

int getCrankAngle_4G63(int timePerDegree)
{
    if(!currentStatus.hasSync) { return 0;}
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();

    int crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage2.triggerAngle; //Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was.
    //Estimate the number of degrees travelled since the last tooth}

    unsigned long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: GM
Desc: TBA
Note: Useful references:
http://www.vems.hu/wiki/index.php?page=MembersPage%2FJorgenKarlsson%2FTwentyFourX
Provided that the cam signal is used, this decoder simply counts the teeth and then looks their angles up against a lookup table. The cam signal is used to determine tooth #1
*/
void triggerSetup_24X()
{
  triggerToothAngle = 15; //The number of degrees that passes from tooth to tooth (primary)
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

  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  toothCurrentCount = 25; //We set the initial tooth value to be something that should never be reached. This indicates no sync
  secondDerivEnabled = false;
  decoderIsSequential = true;
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
     currentStatus.startRevolutions++; //Counter
  }
  else
  {
    toothCurrentCount++; //Increment the tooth counter
  }

   addToothLogEntry(curGap);

   toothLastToothTime = curTime;
}
void triggerSec_24X()
{
  toothCurrentCount = 0; //All we need to do is reset the tooth count back to zero, indicating that we're at the beginning of a new revolution
  return;
}

int getRPM_24X()
{
   return stdGetRPM();
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

    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

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
  triggerToothAngle = 0; //The number of degrees that passes from tooth to tooth (primary)
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

  MAX_STALL_TIME = (3333UL * 60); //Minimum 50rpm. (3333uS is the time per degree at 50rpm). Largest gap between teeth is 60 degrees.
  toothCurrentCount = 13; //We set the initial tooth value to be something that should never be reached. This indicates no sync
  secondDerivEnabled = false;
  decoderIsSequential = false;
}

void triggerPri_Jeep2000()
{
  if(toothCurrentCount == 13) { currentStatus.hasSync = false; return; } //Indicates sync has not been achieved (Still waiting for 1 revolution of the crank to take place)
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap < triggerFilterTime ) { return; } //Noise rejection check. Pulses should never be less than triggerFilterTime

  if(toothCurrentCount == 0)
  {
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     currentStatus.startRevolutions++; //Counter
  }
  else
  {
    toothCurrentCount++; //Increment the tooth counter
  }

  setFilter(curGap); //Recalc the new filter value

  addToothLogEntry(curGap);

  toothLastMinusOneToothTime = toothLastToothTime;
  toothLastToothTime = curTime;
}
void triggerSec_Jeep2000()
{
  toothCurrentCount = 0; //All we need to do is reset the tooth count back to zero, indicating that we're at the beginning of a new revolution
  return;
}

int getRPM_Jeep2000()
{
   return stdGetRPM();
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
    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

/*
Name: Audi 135
Desc: 135 teeth on the crank and 1 tooth on the cam.
Note: This is very similar to the dual wheel decoder, however due to the 135 teeth not dividing evenly into 360, only every 3rd crank tooth is used in calculating the crank angle. This effectively makes it a 45 tooth dual wheel setup
*/
void triggerSetup_Audi135()
{
  triggerToothAngle = 8; //135/3 = 45, 360/45 = 8 degrees every 3 teeth
  toothCurrentCount = 255; //Default value
  toothSystemCount = 0;
  triggerFilterTime = (unsigned long)(1000000 / (MAX_RPM / 60 * 135UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  secondDerivEnabled = false;
  decoderIsSequential = true;
}

void triggerPri_Audi135()
{
   curTime = micros();
   curGap = curTime - toothSystemLastToothTime;
   if ( curGap < triggerFilterTime ) { return; }
   toothSystemCount++;

   if ( !currentStatus.hasSync ) { toothLastToothTime = curTime; return; }
   if ( toothSystemCount < 3 ) { return; } //We only proceed for every third tooth

   addToothLogEntry(curGap);
   toothSystemLastToothTime = curTime;
   toothSystemCount = 0;
   toothCurrentCount++; //Increment the tooth counter

   if ( toothCurrentCount == 1 || toothCurrentCount > 45 )
   {
     toothCurrentCount = 1;
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     revolutionOne = !revolutionOne;
     currentStatus.startRevolutions++; //Counter
   }

   setFilter(curGap); //Recalc the new filter value

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}

void triggerSec_Audi135()
{
  /*
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  if ( curGap2 < triggerSecFilterTime ) { return; }
  toothLastSecToothTime = curTime2;
  */

  if( !currentStatus.hasSync )
  {
    toothCurrentCount = 0;
    currentStatus.hasSync = true;
    toothSystemCount = 3; //Need to set this to 3 so that the next primary tooth is counted
  }
  else if (configPage2.useResync) { toothCurrentCount = 0; }
  revolutionOne = 1; //Sequential revolution reset
}

int getRPM_Audi135()
{
   return stdGetRPM();
}

int getCrankAngle_Audi135(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    bool tempRevolutionOne;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    tempRevolutionOne = revolutionOne;
    interrupts();

    //Handle case where the secondary tooth was the last one seen
    if(tempToothCurrentCount == 0) { tempToothCurrentCount = 45; }

    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    //Sequential check (simply sets whether we're on the first or 2nd revoltuion of the cycle)
    if (tempRevolutionOne) { crankAngle += 360; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    else if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += CRANK_ANGLE_MAX; }

    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Honda D17
Desc:
Note:
*/
void triggerSetup_HondaD17()
{
  triggerToothAngle = 360 / 12; //The number of degrees that passes from tooth to tooth
  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  secondDerivEnabled = false;
  decoderIsSequential = false;
}

void triggerPri_HondaD17()
{
   lastGap = curGap;
   curTime = micros();
   curGap = curTime - toothLastToothTime;
   toothCurrentCount++; //Increment the tooth counter

   addToothLogEntry(curGap);

   //
   if( toothCurrentCount == 13 && currentStatus.hasSync)
   {
     toothCurrentCount = 0;
     return;
   }
   else if( toothCurrentCount == 1 && currentStatus.hasSync)
   {
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.startRevolutions++; //Counter
   }
   else
   {
     //13th tooth
     targetGap = (lastGap) >> 1; //The target gap is set at half the last tooth gap
     if ( curGap < targetGap) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the magical 13th tooth
     {
       toothCurrentCount = 0;
       currentStatus.hasSync = true;
       return; //We return here so that the tooth times below don't get set (The magical 13th tooth should not be considered for any calculations that use those times)
     }
   }

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;
}
void triggerSec_HondaD17() { return; } //The 4+1 signal on the cam is yet to be supported
int getRPM_HondaD17()
{
   return stdGetRPM();
}
int getCrankAngle_HondaD17(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();

    //Check if the last tooth seen was the reference tooth 13 (Number 0 here). All others can be calculated, but tooth 3 has a unique angle
    int crankAngle;
    if( tempToothCurrentCount == 0 )
    {
      crankAngle = 11 * triggerToothAngle + configPage2.triggerAngle; //if temptoothCurrentCount is 0, the last tooth seen was the 13th one. Based on this, ignore the 13th tooth and use the 12th one as the last reference.
    }
    else
    {
      crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle + configPage2.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    }

    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Miata '99 to '05
Desc: TBA (See: http://forum.diyefi.org/viewtopic.php?f=56&t=1077)
Note: 4x 70 degree duration teeth running at cam speed. Believed to be at the same angles as the 4g63 decoder
Tooth #1 is defined as the next crank tooth after the crank signal is HIGH when the cam signal is falling.
Tooth number one is at 355* ATDC
*/
void triggerSetup_Miata9905()
{
  triggerToothAngle = 90; //The number of degrees that passes from tooth to tooth (primary)
  toothCurrentCount = 99; //Fake tooth count represents no sync
  secondDerivEnabled = false;
  decoderIsSequential = true;

  //Note that these angles are for every rising and falling edge

  toothAngles[0] = 350; //Falling edge of tooth #1
  toothAngles[1] = 100; //Rising edge of tooth #2
  toothAngles[2] = 170; //Falling edge of tooth #2
  toothAngles[3] = 280; //Rising edge of tooth #1

  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
}

void triggerPri_Miata9905()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap < triggerFilterTime ) { return; } //Debounce check. Pulses should never be less than triggerFilterTime

  toothCurrentCount++;
  if(toothCurrentCount == 1 || toothCurrentCount == 5) //Trigger is on CHANGE, hence 4 pulses = 1 crank rev
  {
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     currentStatus.startRevolutions++; //Counter
     //if ((startRevolutions & 15) == 1) { currentStatus.hasSync = false; } //Every 64 revolutions, force a resync with the cam
  }
  else if (!currentStatus.hasSync) { return; }

  addToothLogEntry(curGap);

  //Whilst this is an uneven tooth pattern, if the specific angle between the last 2 teeth is specified, 1st deriv prediction can be used
  if(toothCurrentCount == 1 || toothCurrentCount == 3) { triggerToothAngle = 70; triggerFilterTime = curGap; } //Trigger filter is set to whatever time it took to do 70 degrees (Next trigger is 110 degrees away)
  else { triggerToothAngle = 110; triggerFilterTime = (curGap * 3) >> 3; } //Trigger filter is set to (110*3)/8=41.25=41 degrees (Next trigger is 70 degrees away).

  curGap = curGap >> 1;

  toothLastMinusOneToothTime = toothLastToothTime;
  toothLastToothTime = curTime;
}
void triggerSec_Miata9905()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  if ( curGap2 < triggerSecFilterTime ) { return; }
  toothLastSecToothTime = curTime2;
  lastGap = curGap2;

  if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) || !currentStatus.hasSync)
  {
    triggerFilterTime = 1500;
    //Check the status of the crank trigger
    targetGap = (lastGap) >> 1; //The target gap is set at half the last tooth gap
    if ( curGap < targetGap) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the extra (3rd) tooth on the cam). This tooth is located at 421 crank degrees (aka 61 degrees) and therefore the last crank tooth seen was number 1 (At 350 degrees)
    {
      toothCurrentCount = 1;
      currentStatus.hasSync = true;
    }
  }
  //else { triggerFilterTime = 1500; } //reset filter time (ugly)
  return;
}


int getRPM_Miata9905()
{
  //During cranking, RPM is calculated 4 times per revolution, once for each tooth on the crank signal.
  //Because these signals aren't even (Alternating 110 and 70 degrees), this needs a special function
  if(currentStatus.RPM < configPage2.crankRPM)
  {
    int tempToothAngle;
    noInterrupts();
    tempToothAngle = triggerToothAngle;
    revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime); //Note that trigger tooth angle changes between 70 and 110 depending on the last tooth that was seen
    interrupts();
    revolutionTime = revolutionTime * 36;
    return (tempToothAngle * 60000000L) / revolutionTime;
  }
  else { return stdGetRPM(); }
}

int getCrankAngle_Miata9905(int timePerDegree)
{
    if(!currentStatus.hasSync) { return 0;}
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();

    int crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage2.triggerAngle; //Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was.
    //Estimate the number of degrees travelled since the last tooth}

    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Mazda AU version
Desc:
Note:
Tooth #2 is defined as the next crank tooth after the single cam tooth
Tooth number one is at 348* ATDC
*/
void triggerSetup_MazdaAU()
{
  triggerToothAngle = 108; //The number of degrees that passes from tooth to tooth (primary). This is the maximum gap
  toothCurrentCount = 99; //Fake tooth count represents no sync
  secondaryToothCount = 0; //Needed for the cam tooth tracking
  secondDerivEnabled = false;
  decoderIsSequential = true;

  toothAngles[0] = 348; //tooth #1
  toothAngles[1] = 96; //tooth #2
  toothAngles[2] = 168; //tooth #3
  toothAngles[3] = 276; //tooth #4

  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
  triggerFilterTime = 1500; //10000 rpm, assuming we're triggering on both edges off the crank tooth.
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
}

void triggerPri_MazdaAU()
{
  curTime = micros();
  curGap = curTime - toothLastToothTime;
  if ( curGap < triggerFilterTime ) { return; } //Filter check. Pulses should never be less than triggerFilterTime

  addToothLogEntry(curGap);

  toothCurrentCount++;
  if(toothCurrentCount == 1 || toothCurrentCount == 5) //Trigger is on CHANGE, hence 4 pulses = 1 crank rev
  {
     toothCurrentCount = 1; //Reset the counter
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.hasSync = true;
     currentStatus.startRevolutions++; //Counter
     //if ((startRevolutions & 15) == 1) { currentStatus.hasSync = false; } //Every 64 revolutions, force a resync with the cam. For testing only!
  }
  else if (!currentStatus.hasSync) { return; }

  // Locked cranking timing is available, fixed at 12* BTDC
  if ( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) && configPage2.ignCranklock )
  {
    if( toothCurrentCount == 1 ) { endCoil1Charge(); }
    else if( toothCurrentCount == 3 ) { endCoil2Charge(); }
  }

  //Whilst this is an uneven tooth pattern, if the specific angle between the last 2 teeth is specified, 1st deriv prediction can be used
  if(toothCurrentCount == 1 || toothCurrentCount == 3) { triggerToothAngle = 72; triggerFilterTime = curGap; } //Trigger filter is set to whatever time it took to do 72 degrees (Next trigger is 108 degrees away)
  else { triggerToothAngle = 108; triggerFilterTime = (curGap * 3) >> 3; } //Trigger filter is set to (108*3)/8=40 degrees (Next trigger is 70 degrees away).

  toothLastMinusOneToothTime = toothLastToothTime;
  toothLastToothTime = curTime;
}
void triggerSec_MazdaAU()
{
  curTime2 = micros();
  lastGap = curGap2;
  curGap2 = curTime2 - toothLastSecToothTime;
  //if ( curGap2 < triggerSecFilterTime ) { return; }
  toothLastSecToothTime = curTime2;

  //if(BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) || !currentStatus.hasSync) //Not sure if the cranking check is needed here
  if(!currentStatus.hasSync)
  {
    //we find sync by looking for the 2 teeth that are close together. The next crank tooth after that is the one we're looking for.
    //For the sake of this decoder, the lone cam tooth will be designated #1
    if(secondaryToothCount == 2)
    {
      toothCurrentCount = 1;
      currentStatus.hasSync = true;
    }
    else
    {
      triggerFilterTime = 1500; //In case the engine has been running and then lost sync.
      targetGap = (lastGap) >> 1; //The target gap is set at half the last tooth gap
      if ( curGap2 < targetGap) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the extra (3rd) tooth on the cam). This tooth is located at 421 crank degrees (aka 61 degrees) and therefore the last crank tooth seen was number 1 (At 350 degrees)
      {
        secondaryToothCount = 2;
      }
    }
    secondaryToothCount++;
  }

  return;
}


int getRPM_MazdaAU()
{
  if (!currentStatus.hasSync) { return 0; }

  //During cranking, RPM is calculated 4 times per revolution, once for each tooth on the crank signal.
  //Because these signals aren't even (Alternating 108 and 72 degrees), this needs a special function
  if(currentStatus.RPM < configPage2.crankRPM)
  {
    int tempToothAngle;
    noInterrupts();
    tempToothAngle = triggerToothAngle;
    revolutionTime = (toothLastToothTime - toothLastMinusOneToothTime); //Note that trigger tooth angle changes between 72 and 108 depending on the last tooth that was seen
    interrupts();
    revolutionTime = revolutionTime * 36;
    return (tempToothAngle * 60000000L) / revolutionTime;
  }
  else { return stdGetRPM(); }
}

int getCrankAngle_MazdaAU(int timePerDegree)
{
    if(!currentStatus.hasSync) { return 0;}
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();

    int crankAngle = toothAngles[(tempToothCurrentCount - 1)] + configPage2.triggerAngle; //Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was.
    //Estimate the number of degrees travelled since the last tooth}

    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

/*
Name: Non-360 Dual wheel
Desc: 2 wheels located either both on the crank or with the primary on the crank and the secondary on the cam.
Note: There can be no missing teeth on the primary wheel
*/
void triggerSetup_non360()
{
  triggerToothAngle = (360 * configPage2.TrigAngMul) / configPage2.triggerTeeth; //The number of degrees that passes from tooth to tooth multiplied by the additional multiplier
  toothCurrentCount = 255; //Default value
  triggerFilterTime = (int)(1000000 / (MAX_RPM / 60 * configPage2.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  secondDerivEnabled = false;
  decoderIsSequential = true;
  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
}


void triggerPri_non360()
{
  //This is not used, the trigger is identical to the dual wheel one, so that is used instead.
}

void triggerSec_non360()
{
  //This is not used, the trigger is identical to the dual wheel one, so that is used instead.
}

int getRPM_non360()
{
  if( !currentStatus.hasSync || toothCurrentCount == 0 ) { return 0; }
  if(currentStatus.RPM < configPage2.crankRPM) { return crankingGetRPM(configPage2.triggerTeeth); }
  return stdGetRPM();
}

int getCrankAngle_non360(int timePerDegree)
{
    //This is the current angle ATDC the engine is at. This is the last known position based on what tooth was last 'seen'. It is only accurate to the resolution of the trigger wheel (Eg 36-1 is 10 degrees)
    unsigned long tempToothLastToothTime;
    int tempToothCurrentCount;
    //Grab some variables that are used in the trigger code and assign them to temp variables.
    noInterrupts();
    tempToothCurrentCount = toothCurrentCount;
    tempToothLastToothTime = toothLastToothTime;
    interrupts();

    //Handle case where the secondary tooth was the last one seen
    if(tempToothCurrentCount == 0) { tempToothCurrentCount = configPage2.triggerTeeth; }

    //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    int crankAngle = (tempToothCurrentCount - 1) * triggerToothAngle;
    crankAngle = (crankAngle / configPage2.TrigAngMul) + configPage2.triggerAngle; //Have to divide by the multiplier to get back to actual crank angle.

    //Estimate the number of degrees travelled since the last tooth}
    long elapsedTime = micros() - tempToothLastToothTime;
    if(elapsedTime < SHRT_MAX ) { crankAngle += div((int)elapsedTime, timePerDegree).quot; } //This option is much faster, but only available for smaller values of elapsedTime
    else { crankAngle += ldiv(elapsedTime, timePerDegree).quot; }

    if (crankAngle >= 720) { crankAngle -= 720; }
    if (crankAngle > CRANK_ANGLE_MAX) { crankAngle -= CRANK_ANGLE_MAX; }
    if (crankAngle < 0) { crankAngle += 360; }

    return crankAngle;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Name: Nissan 360 tooth with cam
Desc:
Note:
*/
void triggerSetup_Nissan360()
{
  triggerFilterTime = (1000000 / (MAX_RPM / 60 * 360UL)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be disgarded as noise
  triggerSecFilterTime = (int)(1000000 / (MAX_RPM / 60 * 2)) / 2; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
  secondaryToothCount = 0; //Initially set to 0 prior to calculating the secondary window duration
  secondDerivEnabled = false;
  decoderIsSequential = true;
  toothCurrentCount = 1;
  triggerToothAngle = 2;
  MAX_STALL_TIME = (3333UL * triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)
}


void triggerPri_Nissan360()
{
   curTime = micros();
   //curGap = curTime - toothLastToothTime;
   //if ( curGap < triggerFilterTime ) { return; } //Pulses should never be less than triggerFilterTime, so if they are it means a false trigger.
   toothCurrentCount++; //Increment the tooth counter
   //addToothLogEntry(curGap); Disable tooth logging on this decoder due to overhead

   toothLastMinusOneToothTime = toothLastToothTime;
   toothLastToothTime = curTime;

   //if ( !currentStatus.hasSync ) { return; }

   if ( toothCurrentCount == 181) //1 complete crank revolution
   {
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.startRevolutions++;
   }
   else if ( toothCurrentCount == 361 ) //2 complete crank revolutions
   {
     toothCurrentCount = 1;
     toothOneMinusOneTime = toothOneTime;
     toothOneTime = curTime;
     currentStatus.startRevolutions++; //Counter
   }

   //setFilter(curGap); //Recalc the new filter value
}

void triggerSec_Nissan360()
{
  curTime2 = micros();
  curGap2 = curTime2 - toothLastSecToothTime;
  //if ( curGap2 < triggerSecFilterTime ) { return; }
  toothLastSecToothTime = curTime2;
  //triggerSecFilterTime = curGap2 >> 2; //Set filter at 25% of the current speed


  //Calculate number of primary teeth that this window has been active for
  if(secondaryToothCount == 0) { secondaryToothCount = toothCurrentCount; return; } //This occurs on the first rotation upon powerup
  if(READ_SEC_TRIGGER() == LOW) { secondaryToothCount = toothCurrentCount; return; } //This represents the start of a secondary window

  //If we reach here, we are at the end of a secondary window
  byte secondaryDuration = toothCurrentCount - secondaryToothCount; //How many primary teeth have passed during the duration of this secondary window

  if(!currentStatus.hasSync)
  {
    if(configPage1.nCylinders == 4)
    {
      if(secondaryDuration >= 15 || secondaryDuration <= 17) //Duration of window = 16 primary teeth
      {
        toothCurrentCount = 16; //End of first window (The longest) occurs 16 teeth after TDC
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 11 || secondaryDuration <= 13) //Duration of window = 12 primary teeth
      {
        toothCurrentCount = 102; //End of second window is after 90+12 primary teeth
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 7 || secondaryDuration <= 9) //Duration of window = 8 primary teeth
      {
        toothCurrentCount = 188; //End of third window is after 90+90+8 primary teeth
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 3 || secondaryDuration <= 5) //Duration of window = 4 primary teeth
      {
        toothCurrentCount = 274; //End of fourth window is after 90+90+90+4 primary teeth
        currentStatus.hasSync = true;
      }
    }
    else if(configPage1.nCylinders == 6)
    {
      if(secondaryDuration >= 23 || secondaryDuration <= 25) //Duration of window = 16 primary teeth
      {
        toothCurrentCount = 24; //End of first window (The longest) occurs 24 teeth after TDC
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 19 || secondaryDuration <= 21) //Duration of window = 12 primary teeth
      {
        toothCurrentCount = 84; //End of second window is after 60+20 primary teeth
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 15 || secondaryDuration <= 17) //Duration of window = 16 primary teeth
      {
        toothCurrentCount = 136; //End of third window is after 60+60+16 primary teeth
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 11 || secondaryDuration <= 13) //Duration of window = 12 primary teeth
      {
        toothCurrentCount = 192; //End of fourth window is after 60+60+60+12 primary teeth
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 7 || secondaryDuration <= 9) //Duration of window = 8 primary teeth
      {
        toothCurrentCount = 248; //End of fifth window is after 60+60+60+60+8 primary teeth
        currentStatus.hasSync = true;
      }
      else if(secondaryDuration >= 3 || secondaryDuration <= 5) //Duration of window = 4 primary teeth
      {
        toothCurrentCount = 304; //End of sixth window is after 60+60+60+60+60+4 primary teeth
        currentStatus.hasSync = true;
      }
    }
    else { currentStatus.hasSync = false; return ;} //This should really never happen
  }
  else
  {
    //Already have sync, but do a verify every 720 degrees.
    if(configPage1.nCylinders == 4)
    {
      if(secondaryDuration >= 15) //Duration of window = 16 primary teeth
      {
        toothCurrentCount = 16; //End of first window (The longest) occurs 16 teeth after TDC
      }
    }
    else if(configPage1.nCylinders == 6)
    {
      if(secondaryDuration >= 23) //Duration of window = 24 primary teeth
      {
        toothCurrentCount = 24; //End of first window (The longest) occurs 24 teeth after TDC
      }
    }
  }

}

int getRPM_Nissan360()
{
  //if(currentStatus.RPM < configPage2.crankRPM) { return crankingGetRPM(configPage2.triggerTeeth); }
  return stdGetRPM();
}

int getCrankAngle_Nissan360(int timePerDegree)
{
  //As each tooth represents 2 crank degrees, we only need to determine whether we're more or less than halfway between teeth to know whether to add another 1 degrees
  unsigned long halfTooth = (toothLastToothTime - toothLastMinusOneToothTime) >> 1;
  if ( (micros() - toothLastToothTime) > halfTooth)
  {
    //Means we're over halfway to the next tooth, so add on 1 degree
    return (toothCurrentCount * triggerToothAngle) + 1;
  }
  return (toothCurrentCount * triggerToothAngle);
}
