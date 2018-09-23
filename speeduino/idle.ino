/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "idle.h"
#include "maths.h"
#include "timers.h"
#include "src/PID_v1/PID_v1.h"

/*
These functions cover the PWM and stepper idle control
*/

/*
Idle Control
Currently limited to on/off control and open loop PWM and stepper drive
*/
integerPID idlePID(&currentStatus.longRPM, &idle_pid_target_value, &idle_cl_target_rpm, configPage6.idleKP, configPage6.idleKI, configPage6.idleKD, DIRECT); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

void initialiseIdle()
{
  //By default, turn off the PWM interrupt (It gets turned on below if needed)
  IDLE_TIMER_DISABLE();
  #if defined(CORE_AVR) //AVR chips use the ISR for this
    //No timer work required for AVRs. Timer is shared with the schedules and setup in there.

  #elif defined (CORE_TEENSY)

    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
    //FlexTimer 2 is used for idle
    FTM2_MODE |= FTM_MODE_WPDIS; // Write Protection Disable
    FTM2_MODE |= FTM_MODE_FTMEN; //Flex Timer module enable
    FTM2_MODE |= FTM_MODE_INIT;

    FTM2_SC = 0x00; // Set this to zero before changing the modulus
    FTM2_CNTIN = 0x0000; //Shouldn't be needed, but just in case
    FTM2_CNT = 0x0000; // Reset the count to zero
    FTM2_MOD = 0xFFFF; // max modulus = 65535

    /*
     * Enable the clock for FTM0/1
     * 00 No clock selected. Disables the FTM counter.
     * 01 System clock
     * 10 Fixed frequency clock (32kHz)
     * 11 External clock
     */
    FTM2_SC |= FTM_SC_CLKS(0b10);

    /*
    * Trim the slow clock from 32kHz down to 31.25kHz (The slowest it will go)
    * This is somewhat imprecise and documentation is not good.
    * I poked the chip until I figured out the values associated with 31.25kHz
    */
    MCG_C3 = 0x9B;

    /*
     * Set Prescaler
     * This is the slowest that the timer can be clocked (Without used the slow timer, which is too slow). It results in ticks of 2.13333uS on the teensy 3.5:
     * 32000 Hz = F_BUS
     * 128 * 1000000uS / F_BUS = 2.133uS
     *
     * 000 = Divide by 1
     * 001 Divide by 2
     * 010 Divide by 4
     * 011 Divide by 8
     * 100 Divide by 16
     * 101 Divide by 32
     * 110 Divide by 64
     * 111 Divide by 128
     */
    FTM2_SC |= FTM_SC_PS(0b0); //No prescaler

    //Setup the channels (See Pg 1014 of K64 DS).
    FTM2_C0SC &= ~FTM_CSC_MSB; //According to Pg 965 of the K64 datasheet, this should not be needed as MSB is reset to 0 upon reset, but the channel interrupt fails to fire without it
    FTM2_C0SC |= FTM_CSC_MSA; //Enable Compare mode
    FTM2_C0SC |= FTM_CSC_CHIE; //Enable channel compare interrupt

    // enable IRQ Interrupt
    NVIC_ENABLE_IRQ(IRQ_FTM2);
  }

  #endif

  //Initialising comprises of setting the 2D tables with the relevant values from the config pages
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:
      //Case 1 is on/off idle control
      if ((currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp)
      {
        digitalWrite(pinIdle1, HIGH);
        idleOn = true;
      }
      break;

    case IAC_ALGORITHM_PWM_OL:
      //Case 2 is PWM open loop
      iacPWMTable.xSize = 10;
      iacPWMTable.valueSize = SIZE_BYTE;
      iacPWMTable.values = configPage6.iacOLPWMVal;
      iacPWMTable.axisX = configPage6.iacBins;


      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.valueSize = SIZE_BYTE;
      iacCrankDutyTable.values = configPage6.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage6.iacCrankBins;

      idle_pin_port = portOutputRegister(digitalPinToPort(pinIdle1));
      idle_pin_mask = digitalPinToBitMask(pinIdle1);
      idle2_pin_port = portOutputRegister(digitalPinToPort(pinIdle2));
      idle2_pin_mask = digitalPinToBitMask(pinIdle2);
      #if defined(CORE_STM32)
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
      #else
        idle_pwm_max_count = 1000000L / (16 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #endif
      enableIdle();
      break;

    case IAC_ALGORITHM_PWM_CL:
      //Case 3 is PWM closed loop
      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.valueSize = SIZE_BYTE;
      iacClosedLoopTable.values = configPage6.iacCLValues;
      iacClosedLoopTable.axisX = configPage6.iacBins;

      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.valueSize = SIZE_BYTE;
      iacCrankDutyTable.values = configPage6.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage6.iacCrankBins;

      idle_pin_port = portOutputRegister(digitalPinToPort(pinIdle1));
      idle_pin_mask = digitalPinToBitMask(pinIdle1);
      idle2_pin_port = portOutputRegister(digitalPinToPort(pinIdle2));
      idle2_pin_mask = digitalPinToBitMask(pinIdle2);
      #if defined(CORE_STM32)
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
      #else
        idle_pwm_max_count = 1000000L / (16 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #endif
      idlePID.SetOutputLimits(percentage(configPage2.iacCLminDuty, idle_pwm_max_count), percentage(configPage2.iacCLmaxDuty, idle_pwm_max_count));
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on

      idleCounter = 0;
      break;

    case IAC_ALGORITHM_STEP_OL:
      //Case 2 is Stepper open loop
      iacStepTable.xSize = 10;
      iacStepTable.valueSize = SIZE_BYTE;
      iacStepTable.values = configPage6.iacOLStepVal;
      iacStepTable.axisX = configPage6.iacBins;

      iacCrankStepsTable.xSize = 4;
      iacCrankStepsTable.valueSize = SIZE_BYTE;
      iacCrankStepsTable.values = configPage6.iacCrankSteps;
      iacCrankStepsTable.axisX = configPage6.iacCrankBins;
      iacStepTime = configPage6.iacStepTime * 1000;

      completedHomeSteps = 0;
      idleStepper.curIdleStep = 0;
      idleStepper.stepperStatus = SOFF;
      break;

    case IAC_ALGORITHM_STEP_CL:
      //Case 5 is Stepper closed loop
      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.valueSize = SIZE_BYTE;
      iacClosedLoopTable.values = configPage6.iacCLValues;
      iacClosedLoopTable.axisX = configPage6.iacBins;

      iacCrankStepsTable.xSize = 4;
      iacCrankStepsTable.valueSize = SIZE_BYTE;
      iacCrankStepsTable.values = configPage6.iacCrankSteps;
      iacCrankStepsTable.axisX = configPage6.iacCrankBins;
      iacStepTime = configPage6.iacStepTime * 1000;

      completedHomeSteps = 0;
      idleCounter = 0;
      idleStepper.curIdleStep = 0;
      idleStepper.stepperStatus = SOFF;

      idlePID.SetOutputLimits(0, (configPage6.iacStepHome * 3)); //Maximum number of steps probably needs its own setting
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on
      break;

    default:
      //Well this just shouldn't happen
      break;
  }
  idleInitComplete = configPage6.iacAlgorithm; //Sets which idle method was initialised
  currentStatus.idleLoad = 0;
  #if defined(CORE_STM32) //Need to be initialised last due to instant interrupt
    Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    if(idle_pwm_max_count > 0) { Timer1.attachInterrupt(4, idleInterrupt);} //on first flash the configPage4.iacAlgorithm is invalid
    Timer1.resume();
  #endif
}

void idleControl()
{
  if(idleInitComplete != configPage6.iacAlgorithm) { initialiseIdle(); }
  if(currentStatus.RPM > 0) { enableIdle(); }

  //Check whether the idleUp is active
  if(configPage2.idleUpEnabled == true)
  {
    if(configPage2.idleUpPolarity == 0) { currentStatus.idleUpActive = !digitalRead(pinIdleUp); } //Normal mode (ground switched)
    else { currentStatus.idleUpActive = digitalRead(pinIdleUp); } //Inverted mode (5v activates idleUp)
  }
  else { currentStatus.idleUpActive = false; }

  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:      //Case 1 is on/off idle control
      if ( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp) //All temps are offset by 40 degrees
      {
        digitalWrite(pinIdle1, HIGH);
        idleOn = true;
      }
      else if (idleOn) { digitalWrite(pinIdle1, LOW); idleOn = false; }
      break;

    case IAC_ALGORITHM_PWM_OL:      //Case 2 is PWM open loop
      //Check for cranking pulsewidth
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleDuty = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        if(currentStatus.idleUpActive == true) { currentStatus.idleDuty += configPage2.idleUpAdder; } //Add Idle Up amount if active
        if( currentStatus.idleDuty == 0 ) { disableIdle(); break; }
        idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
        idleOn = true;
      }
      else
      {
        //Standard running
        currentStatus.idleDuty = table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        if(currentStatus.idleUpActive == true) { currentStatus.idleDuty += configPage2.idleUpAdder; } //Add Idle Up amount if active
        if( currentStatus.idleDuty == 0 ) { disableIdle(); break; }
        idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
        currentStatus.idleLoad = currentStatus.idleDuty >> 1;
        idleOn = true;
      }
      break;

    case IAC_ALGORITHM_PWM_CL:    //Case 3 is PWM closed loop
        //No cranking specific value for closed loop (yet?)
        idle_cl_target_rpm = table2D_getValue(&iacClosedLoopTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) * 10; //All temps are offset by 40 degrees
        if( (idleCounter & 31) == 1) { idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD); } //This only needs to be run very infrequently, once every 32 calls to idleControl(). This is approx. once per second

        idlePID.Compute();
        idle_pwm_target_value = idle_pid_target_value;
        if( idle_pwm_target_value == 0 ) { disableIdle(); }
        currentStatus.idleLoad = ((unsigned long)(idle_pwm_target_value * 100UL) / idle_pwm_max_count) >> 1;
        if(currentStatus.idleUpActive == true) { currentStatus.idleDuty += configPage2.idleUpAdder; } //Add Idle Up amount if active
        //idle_pwm_target_value = 104;

        idleCounter++;
      break;

    case IAC_ALGORITHM_STEP_OL:    //Case 4 is open loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if( (checkForStepping() == false) && (isStepperHomed() == true) ) //Check that homing is complete and that there's not currently a step already taking place. MUST BE IN THIS ORDER!
      {
        //Check for cranking pulsewidth
        if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
        {
          //Currently cranking. Use the cranking table
          idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
          if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
          doStep();
        }
        else if( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < iacStepTable.axisX[IDLE_TABLE_SIZE-1])
        {
          //Standard running
          if ((mainLoopCount & 255) == 1)
          {
            //Only do a lookup of the required value around 4 times per second. Any more than this can create too much jitter and require a hyster value that is too high
            idleStepper.targetIdleStep = table2D_getValue(&iacStepTable, (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
            if(currentStatus.idleUpActive == true) { idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
            iacStepTime = configPage6.iacStepTime * 1000;
          }
          doStep();
        }
        currentStatus.idleLoad = idleStepper.curIdleStep >> 1; //Current step count (Divided by 2 for byte)
      }
      break;

    case IAC_ALGORITHM_STEP_CL://Case 5 is closed loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if( (checkForStepping() == false) && (isStepperHomed() == true) ) //Check that homing is complete and that there's not currently a step already taking place. MUST BE IN THIS ORDER!
      {
        if( (idleCounter & 31) == 1)
        {
          //This only needs to be run very infrequently, once every 32 calls to idleControl(). This is approx. once per second
          idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
          iacStepTime = configPage6.iacStepTime * 1000;
        }

        idle_cl_target_rpm = table2D_getValue(&iacClosedLoopTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) * 10; //All temps are offset by 40 degrees
        if(currentStatus.idleUpActive == true) { idle_pid_target_value += configPage2.idleUpAdder; } //Add Idle Up amount if active
        idlePID.Compute();
        idleStepper.targetIdleStep = idle_pid_target_value;

        doStep();
        currentStatus.idleLoad = idleStepper.curIdleStep >> 1; //Current step count (Divided by 2 for byte)
        idleCounter++;
      }
      break;

    default:
      //There really should be a valid idle type
      break;
  }
}

/*
Checks whether the stepper has been homed yet. If it hasn't, will handle the next step
Returns:
True: If the system has been homed. No other action is taken
False: If the motor has not yet been homed. Will also perform another homing step.
*/
static inline byte isStepperHomed()
{
  bool isHomed = true; //As it's the most common scenario, default value is true
  if( completedHomeSteps < (configPage6.iacStepHome * 3) ) //Home steps are divided by 3 from TS
  {
    digitalWrite(pinStepperDir, STEPPER_BACKWARD); //Sets stepper direction to backwards
    digitalWrite(pinStepperEnable, LOW); //Enable the DRV8825
    digitalWrite(pinStepperStep, HIGH);
    idleStepper.stepStartTime = micros_safe();
    idleStepper.stepperStatus = STEPPING;
    completedHomeSteps++;
    idleOn = true;
    isHomed = false;
  }
  return isHomed;
}

/*
Checks whether a step is currently underway or whether the motor is in 'cooling' state (ie whether it's ready to begin another step or not)
Returns:
True: If a step is underway or motor is 'cooling'
False: If the motor is ready for another step
*/
static inline byte checkForStepping()
{
  bool isStepping = false;
  if( (idleStepper.stepperStatus == STEPPING) || (idleStepper.stepperStatus == COOLING) )
  {
    if(micros_safe() > (idleStepper.stepStartTime + iacStepTime) )
    {
      if(idleStepper.stepperStatus == STEPPING)
      {
        //Means we're currently in a step, but it needs to be turned off
        digitalWrite(pinStepperStep, LOW); //Turn off the step
        idleStepper.stepStartTime = micros_safe();
        idleStepper.stepperStatus = COOLING; //'Cooling' is the time the stepper needs to sit in LOW state before the next step can be made
        isStepping = true;
      }
      else
      {
        //Means we're in COOLING status but have been in this state long enough. Go into off state
        idleStepper.stepperStatus = SOFF;
        digitalWrite(pinStepperEnable, HIGH); //Disable the DRV8825
      }
    }
    else
    {
      //Means we're in a step, but it doesn't need to turn off yet. No further action at this time
      isStepping = true;
    }
  }
  return isStepping;
}

/*
Performs a step
*/
static inline void doStep()
{
  if ( (idleStepper.targetIdleStep <= (idleStepper.curIdleStep - configPage6.iacStepHyster)) || (idleStepper.targetIdleStep >= (idleStepper.curIdleStep + configPage6.iacStepHyster)) ) //Hysteris check
  {
    if(idleStepper.targetIdleStep < idleStepper.curIdleStep) { digitalWrite(pinStepperDir, STEPPER_BACKWARD); idleStepper.curIdleStep--; }//Sets stepper direction to backwards
    else if (idleStepper.targetIdleStep > idleStepper.curIdleStep) { digitalWrite(pinStepperDir, STEPPER_FORWARD); idleStepper.curIdleStep++; }//Sets stepper direction to forwards

    digitalWrite(pinStepperEnable, LOW); //Enable the DRV8825
    digitalWrite(pinStepperStep, HIGH);
    idleStepper.stepStartTime = micros_safe();
    idleStepper.stepperStatus = STEPPING;
    idleOn = true;
  }
}

//This function simply turns off the idle PWM and sets the pin low
static inline void disableIdle()
{
  if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) )
  {
    IDLE_TIMER_DISABLE();
    digitalWrite(pinIdle1, LOW);
  }
  else if ( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) )
  {
    //Only disable the stepper motor if homing is completed
    if( (checkForStepping() == false) && (isStepperHomed() == true) )
    {
      digitalWrite(pinStepperEnable, HIGH); //Disable the DRV8825
      idleStepper.targetIdleStep = idleStepper.curIdleStep; //Don't try to move anymore
    }
  }
}

//Any common functions associated with starting the Idle
//Typically this is enabling the PWM interrupt
static inline void enableIdle()
{
  if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) )
  {
    IDLE_TIMER_ENABLE();
  }
  else if ( (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OL) )
  {

  }
}

#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER4_COMPC_vect)
#elif defined (CORE_TEENSY) || defined (CORE_STM32)
static inline void idleInterrupt() //Most ARM chips can simply call a function
#endif
{
  if (idle_pwm_state)
  {
    if (configPage6.iacPWMdir == 0)
    {
      //Normal direction
      *idle_pin_port &= ~(idle_pin_mask);  // Switch pin to low (1 pin mode)
      if(configPage6.iacChannels == 1) { *idle2_pin_port |= (idle2_pin_mask); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }
    else
    {
      //Reversed direction
      *idle_pin_port |= (idle_pin_mask);  // Switch pin high
      if(configPage6.iacChannels == 1) { *idle2_pin_port &= ~(idle2_pin_mask); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }
    IDLE_COMPARE = IDLE_COUNTER + (idle_pwm_max_count - idle_pwm_cur_value);
    idle_pwm_state = false;
  }
  else
  {
    if (configPage6.iacPWMdir == 0)
    {
      //Normal direction
      *idle_pin_port |= (idle_pin_mask);  // Switch pin high
      if(configPage6.iacChannels == 1) { *idle2_pin_port &= ~(idle2_pin_mask); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }
    else
    {
      //Reversed direction
      *idle_pin_port &= ~(idle_pin_mask);  // Switch pin to low (1 pin mode)
      if(configPage6.iacChannels == 1) { *idle2_pin_port |= (idle2_pin_mask); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }
    IDLE_COMPARE = IDLE_COUNTER + idle_pwm_target_value;
    idle_pwm_cur_value = idle_pwm_target_value;
    idle_pwm_state = true;
  }
}
