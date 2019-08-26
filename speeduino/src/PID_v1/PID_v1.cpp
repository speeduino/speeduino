/**********************************************************************************************
 * Arduino PID Library - Version 1.0.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "PID_v1.h"

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(long* Input, long* Output, long* Setpoint,
        byte Kp, byte Ki, byte Kd, byte ControllerDirection)
{

    myOutput = Output;
    myInput = Input;
    mySetpoint = Setpoint;
	inAuto = false;

	PID::SetOutputLimits(0, 255);				//default output limit corresponds to
												//the arduino pwm limits

    SampleTime = 100;							//default Controller Sample Time is 0.1 seconds

    PID::SetControllerDirection(ControllerDirection);
    PID::SetTunings(Kp, Ki, Kd);

    lastTime = millis()-SampleTime;
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool PID::Compute()
{
   if(!inAuto) return false;
   unsigned long now = millis();
   SampleTime = (now - lastTime);
   //if(timeChange>=SampleTime)
   {
      /*Compute all the working error variables*/
	  long input = *myInput;
      long error = *mySetpoint - input;
      ITerm += (ki * error)/100;
      if(ITerm > outMax) ITerm= outMax;
      else if(ITerm < outMin) ITerm= outMin;
      long dInput = (input - lastInput);

      /*Compute PID Output*/
      long output = (kp * error)/100 + ITerm- (kd * dInput)/100;

	  if(output > outMax) { output = outMax; }
      else if(output < outMin) { output = outMin; }
	  *myOutput = output;

      /*Remember some variables for next time*/
      lastInput = input;
      //lastTime = now;
	  return true;
   }
   //else return false;
}


/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID::SetTunings(byte Kp, byte Ki, byte Kd)
{
   dispKp = Kp; dispKi = Ki; dispKd = Kd;

   /*
   double SampleTimeInSec = ((double)SampleTime)/1000;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;
   */
  long InverseSampleTimeInSec = 100000 / SampleTime;
  kp = Kp;
  ki = (Ki * 100) / InverseSampleTimeInSec;
  kd = (Kd * InverseSampleTimeInSec) / 100;

  if(controllerDirection ==REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void PID::SetSampleTime(int NewSampleTime)
{
   if (NewSampleTime > 0)
   {
      unsigned long ratioX1000  = (unsigned long)(NewSampleTime * 1000) / (unsigned long)SampleTime;
      ki = (ki * ratioX1000) / 1000;
      //kd /= ratio;
      kd = (kd * 1000) / ratioX1000;
      SampleTime = (unsigned long)NewSampleTime;
   }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID::SetOutputLimits(long Min, long Max)
{
   if(Min >= Max) return;
   outMin = Min;
   outMax = Max;

   if(inAuto)
   {
	   if(*myOutput > outMax) *myOutput = outMax;
	   else if(*myOutput < outMin) *myOutput = outMin;

	   if(ITerm > outMax) ITerm= outMax;
	   else if(ITerm < outMin) ITerm= outMin;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID::SetMode(int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
    if(newAuto == !inAuto)
    {  /*we just went from manual to auto*/
        PID::Initialize();
    }
    inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize()
{
   ITerm = *myOutput;
   lastInput = *myInput;
   if(ITerm > outMax) ITerm = outMax;
   else if(ITerm < outMin) ITerm = outMin;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(byte Direction)
{
   if(inAuto && Direction !=controllerDirection)
   {
	  kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
int16_t PID::GetKp(){ return  dispKp; }
int16_t PID::GetKi(){ return  dispKi;}
int16_t PID::GetKd(){ return  dispKd;}
int PID::GetMode(){ return  inAuto ? AUTOMATIC : MANUAL;}
int PID::GetDirection(){ return controllerDirection;}

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
/**
 * @brief A standard integer PID controller. 
 *
 * @param Input Pointer to the variable holding the current value that is to be controlled. Eg In an idle control this would point to RPM
 * @param Output The address in the page that should be returned. This is as per the page definition in the ini
 * 
 * @return byte The current target advance value in degrees
 */
integerPID::integerPID(long* Input, long* Output, long* Setpoint,
        byte Kp, byte Ki, byte Kd, byte ControllerDirection)
{

    myOutput = Output;
   myInput = Input;
    mySetpoint = Setpoint;
	inAuto = false;

	integerPID::SetOutputLimits(0, 255);				//default output limit corresponds to
												//the arduino pwm limits

    SampleTime = 250;							//default Controller Sample Time is 0.25 seconds. This is the 4Hz control time for Idle and VVT

    integerPID::SetControllerDirection(ControllerDirection);
    integerPID::SetTunings(Kp, Ki, Kd);

    lastTime = millis()-SampleTime;
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool integerPID::Compute(bool pOnE)
{
   if(!inAuto) return false;
   unsigned long now = millis();
   //SampleTime = (now - lastTime);
   unsigned long timeChange = (now - lastTime);
   if(timeChange >= SampleTime)
   {
      /*Compute all the working error variables*/
	  long input = *myInput;
      long error = *mySetpoint - input;
      long dInput = (input - lastInput);

      outputSum += (ki * error)/1024; //Note that ki is multiplied by 1024 so it must be divided by 1024 here
      if(outputSum > outMax) { outputSum = outMax; }
      else if(outputSum < outMin) { outputSum = outMin; }
      
      /*Compute PID Output*/
      long output;
      
      if(pOnE)
      {
         output = (kp * error) + outputSum - ((kd * dInput)/128);
      }
      else
      {
         outputSum -= kp * dInput;
         if(outputSum > outMax) { outputSum = outMax; }
         else if(outputSum < outMin) { outputSum = outMin; }

         output = outputSum - ((kd * dInput)/128);
      }
      

	   if(output > outMax) output = outMax;
      else if(output < outMin) output = outMin;
	   *myOutput = output;

      /*Remember some variables for next time*/
      lastInput = input;
      lastTime = now;

      return true;
   }
   else return false;
}



/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void integerPID::SetTunings(byte Kp, byte Ki, byte Kd)
{
   if ( dispKp == Kp && dispKi == Ki && dispKd == Kd ) return; //Only do anything if one of the values has changed
   dispKp = Kp; dispKi = Ki; dispKd = Kd;

   /*
   double SampleTimeInSec = ((double)SampleTime)/1000;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;
   */
  long InverseSampleTimeInSec = 1000 / SampleTime;
  kp = Kp;
  ki = (long)((long)Ki * 1024) / InverseSampleTimeInSec;
  kd = ((long)Kd * InverseSampleTimeInSec);

  if(controllerDirection == REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void integerPID::SetSampleTime(uint16_t NewSampleTime)
{
   if (SampleTime == (unsigned long)NewSampleTime) return; //If new value = old value, no action required.
   SampleTime = NewSampleTime;

   //This resets the tuning values with the appropriate new scaling
   //The +1/-1 is there just so that this doesn't trip the check at the beginning of the SetTunings() function
   SetTunings(dispKp+1, dispKi+1, dispKd+1);
   SetTunings(dispKp-1, dispKi-1, dispKd-1);
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void integerPID::SetOutputLimits(long Min, long Max)
{
   if(Min >= Max) return;
   outMin = Min;
   outMax = Max;

   if(inAuto)
   {
	   if(*myOutput > outMax) *myOutput = outMax;
	   else if(*myOutput < outMin) *myOutput = outMin;

	   if(outputSum > outMax) { outputSum = outMax; }
	   else if(outputSum < outMin) { outputSum = outMin; }
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void integerPID::SetMode(int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
    if(newAuto == !inAuto)
    {  /*we just went from manual to auto*/
        integerPID::Initialize();
    }
    inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void integerPID::Initialize()
{
   outputSum = *myOutput;
   lastInput = *myInput;
   lastMinusOneInput = *myInput;
   if(outputSum > outMax) { outputSum = outMax; }
   else if(outputSum < outMin) { outputSum = outMin; }
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void integerPID::SetControllerDirection(byte Direction)
{
   if(inAuto && Direction !=controllerDirection)
   {
	  kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
int integerPID::GetMode(){ return  inAuto ? AUTOMATIC : MANUAL;}
int integerPID::GetDirection(){ return controllerDirection;}

//************************************************************************************************************************
#define limitMultiplier 100 //How much outMin and OutMax must be multiplied by to get them in the same scale as the output

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
integerPID_ideal::integerPID_ideal(long* Input, uint16_t* Output, uint16_t* Setpoint, uint16_t* Sensitivity, byte* SampleTime,
                                   byte Kp, byte Ki, byte Kd, byte ControllerDirection)
{

    myOutput = Output;
    myInput = (long*)Input;
    mySetpoint = Setpoint;
    mySensitivity = Sensitivity;
    mySampleTime = SampleTime;

	  integerPID_ideal::SetOutputLimits(20, 80);				//default output limits

    integerPID_ideal::SetControllerDirection(ControllerDirection);
    integerPID_ideal::SetTunings(Kp, Ki, Kd);

    lastTime = millis()- *mySampleTime;
    lastError = 0;
}

/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool integerPID_ideal::Compute()
{
   unsigned long now = millis();
   //SampleTime = (now - lastTime);
   unsigned long timeChange = (now - lastTime);
   if(timeChange >= *mySampleTime)
   {
      /*Compute all the working error variables*/
      uint16_t sensitivity = 10001 - (*mySensitivity * 2);
      long unitless_setpoint = (((long)*mySetpoint - 0) * 10000L) / (sensitivity - 0);
      long unitless_input = (((long)*myInput - 0) * 10000L) / (sensitivity - 0);
      long error = unitless_setpoint - unitless_input;

      ITerm += error;

      uint16_t bias = 50; //Base target DC%
      long output = 0;

      if(ki != 0)
      {
        output = ((outMax - bias) * limitMultiplier * 100) / (long)ki;
        if (output < 0) { output = 0; }
      }
      if (ITerm > output) { ITerm = output; }

      if(ki != 0)
      {
        output = ((bias - outMin) * limitMultiplier * 100) / (long)ki;
        if (output < 0) { output = 0; }
      }
      else { output = 0; }
      if (ITerm < -output) { ITerm = -output; }

      /*Compute PID Output*/
      output = (kp * error) + (ki * ITerm) + (kd * (error - lastError));
      output = (bias * limitMultiplier) + (output / 10); //output is % multipled by 1000. To get % with 2 decimal places, divide it by 10. Likewise, bias is % in whole numbers. Multiply it by 100 to get it with 2 places.

      if(output > (outMax * limitMultiplier)) { output  = (outMax * limitMultiplier);  }
      if(output < (outMin * limitMultiplier)) { output  = (outMin * limitMultiplier);  }

	    *myOutput = output;

      /*Remember some variables for next time*/
      lastTime = now;
      lastError = error;

      return true;
   }
   else return false;
}


/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void integerPID_ideal::SetTunings(byte Kp, byte Ki, byte Kd)
{
   if ( dispKp == Kp && dispKi == Ki && dispKd == Kd ) return; //Only do anything if one of the values has changed
   dispKp = Kp; dispKi = Ki; dispKd = Kd;

  kp = Kp;
  ki = Ki;
  kd = Kd;

  if(controllerDirection == REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void integerPID_ideal::SetOutputLimits(long Min, long Max)
{
   if(Min < Max)
   {
     outMin = Min;
     outMax = Max;
   }
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void integerPID_ideal::Initialize()
{
   ITerm = 0;
   lastInput = *myInput;
   lastError = 0;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void integerPID_ideal::SetControllerDirection(byte Direction)
{
   if(Direction != controllerDirection)
   {
	    kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
int integerPID_ideal::GetDirection(){ return controllerDirection;}
