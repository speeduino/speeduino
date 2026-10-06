/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "idle.h"
#include "idleController_state.h"
#include "elapsed_time.h"
#include "maths.h"
#include "preprocessor.h"
#include "units.h"
#include "globals.h"

using namespace idleController::detail;
TESTABLE_STATIC state_t _idleState;

#define STEPPER_FORWARD 0
#define STEPPER_BACKWARD 1
#define STEPPER_POWER_WHEN_ACTIVE 0

#define STEPPER_LESS_AIR_DIRECTION() ((configPage9.iacStepperInv == 0) ? STEPPER_BACKWARD : STEPPER_FORWARD)
#define STEPPER_MORE_AIR_DIRECTION() ((configPage9.iacStepperInv == 0) ? STEPPER_FORWARD : STEPPER_BACKWARD)

constexpr table2D_u8_u8_10 iacPWMTable(&configPage6.iacBins, &configPage6.iacOLPWMVal);
constexpr table2D_u8_u8_10 iacStepTable(&configPage6.iacBins, &configPage6.iacOLStepVal);
//Open loop tables specifically for cranking
constexpr table2D_u8_u8_4 iacCrankStepsTable(&configPage6.iacCrankBins, &configPage6.iacCrankSteps);
constexpr table2D_u8_u8_4 iacCrankDutyTable(&configPage6.iacCrankBins, &configPage6.iacCrankDuty);

/*
These functions cover the PWM and stepper idle control
*/

/*
Idle Control
Currently limited to on/off control and open loop PWM and stepper drive
*/

//Any common functions associated with starting the Idle
//Typically this is enabling the PWM interrupt
static inline void enableIdle(void)
{
  if (isPwmIac(configPage6))
  {
    IDLE_TIMER_ENABLE();
  }
}

static inline void initialiseIdleUpOutput(void)
{
  if (configPage2.idleUpOutputInv) { _idleState.idleUpOutputHIGH = LOW; _idleState.idleUpOutputLOW = HIGH; }
  else { _idleState.idleUpOutputHIGH = HIGH; _idleState.idleUpOutputLOW = LOW; }

  if(configPage2.idleUpEnabled) { digitalWrite(pinNumbers.pinIdleUpOutput, _idleState.idleUpOutputLOW); } //Initialise program with the idle up output in the off state if it is enabled. 
  currentStatus.idleUpOutputActive = false;
}

static void setIdlePidTunings(const config6 &page6)
{
  _idleState.idlePID.setTunings(PidTuningParameters(page6.idleKP, page6.idleKI, page6.idleKD), millis(), 250); //4Hz means 250ms
  _idleState.idlePID.setSetPoint(_idleState.idle_cl_target_rpm);
}

static void configureIdlePID(const config6 &page6, uint32_t minOutput, uint32_t maxOutput, uint16_t initialTarget)
{
    _idleState.idlePID.setOutputLimits(minOutput, maxOutput);
    setIdlePidTunings(page6);
    _idleState.idle_pid_target_value = initialTarget;
    _idleState.idlePID.activate(currentStatus.RPM); //Turn PID on
}

void initialiseIdle(bool forcehoming)
{
  //By default, turn off the PWM interrupt (It gets turned on below if needed)
  IDLE_TIMER_DISABLE();

  //Pin masks must always be initialised, regardless of whether PWM idle is used. This is required for STM32 to prevent issues if the IRQ function fires on restart/overflow
  _idleState.idle_pin.setPin(pinNumbers.pinIdle1, OUTPUT);
  _idleState.idle2_pin.setPin(pinNumbers.pinIdle2, OUTPUT);

  _idleState.idle_pwm_max_count = pwmFreqToTicks(FREQUENCY.toUser(configPage6.idleFreq));
  
  //Initialising comprises of setting the 2D tables with the relevant values from the config pages
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       
      //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:
      //Case 1 is on/off idle control
      if ((temperatureAddOffset(currentStatus.coolant)) < configPage6.iacFastTemp)
      {
        _idleState.idle_pin.setPinHigh();
        _idleState.idleOn = true;
      }
      break;

    case IAC_ALGORITHM_PWM_OL:
      //Case 2 is PWM open loop
      enableIdle();
      break;

    case IAC_ALGORITHM_PWM_OLCL:
      //Case 6 is PWM closed loop with open loop table used as feed forward
      configureIdlePID(configPage6, 
                        percentage(configPage2.iacCLminValue, _idleState.idle_pwm_max_count<<2), 
                        percentage(configPage2.iacCLmaxValue, _idleState.idle_pwm_max_count<<2), 
                        0);
      _idleState.idleCounter = 0;
      break;

    case IAC_ALGORITHM_PWM_CL:
      //Case 3 is PWM closed loop
      configureIdlePID( configPage6, 
                        percentage(configPage2.iacCLminValue, _idleState.idle_pwm_max_count<<2), 
                        percentage(configPage2.iacCLmaxValue, _idleState.idle_pwm_max_count<<2),
                        table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)));
      _idleState.idleCounter = 0;
      break;

    case IAC_ALGORITHM_STEP_OL:
      //Case 2 is Stepper open loop
      _idleState.iacStepTime_uS = configPage6.iacStepTime * 1000;
      _idleState.iacCoolTime_uS = configPage9.iacCoolTime * 1000;

      if (forcehoming)
      {
        //Change between modes running make engine stall
        _idleState.completedHomeSteps = 0;
        _idleState.idleStepper.curIdleStep = 0;
        _idleState.idleStepper.stepperStatus = StepperStatus::SOFF;
      }

      configPage6.iacPWMrun = false; // just in case. This needs to be false with stepper idle
      break;

    case IAC_ALGORITHM_STEP_CL:
      //Case 5 is Stepper closed loop
      _idleState.iacStepTime_uS = configPage6.iacStepTime * 1000;
      _idleState.iacCoolTime_uS = configPage9.iacCoolTime * 1000;

      if (forcehoming)
      {
        //Change between modes running make engine stall
        _idleState.completedHomeSteps = 0;
        _idleState.idleStepper.curIdleStep = 0;
         _idleState.idleStepper.stepperStatus = StepperStatus::SOFF;
      }

      configureIdlePID(configPage6, 
                       (configPage2.iacCLminValue * 3)<<2, 
                       (configPage2.iacCLmaxValue * 3)<<2,
                       currentStatus.CLIdleTarget * 3);
      configPage6.iacPWMrun = false; // just in case. This needs to be false with stepper idle
      break;

    case IAC_ALGORITHM_STEP_OLCL:
      //Case 7 is Stepper closed loop with open loop table used as feed forward
      _idleState.iacStepTime_uS = configPage6.iacStepTime * 1000;
      _idleState.iacCoolTime_uS = configPage9.iacCoolTime * 1000;

      if (forcehoming)
      {
        //Change between modes running make engine stall
        _idleState.completedHomeSteps = 0;
        _idleState.idleStepper.curIdleStep = 0;
         _idleState.idleStepper.stepperStatus = StepperStatus::SOFF;
      }

      configureIdlePID(configPage6, 
                       (configPage2.iacCLminValue * 3)<<2,
                       (configPage2.iacCLmaxValue * 3)<<2, //Maximum number of steps; always less than home steps count.
                       0);
      configPage6.iacPWMrun = false; // just in case. This needs to be false with stepper idle
      break;

    default:
      //Well this just shouldn't happen
      break;
  }

  initialiseIdleUpOutput();

  _idleState.idleInitComplete = configPage6.iacAlgorithm; //Sets which idle method was initialised
  currentStatus.idleLoad = 0;
}

/*
Checks whether a step is currently underway or whether the motor is in 'cooling' state (ie whether it's ready to begin another step or not)
Returns:
True: If a step is underway or motor is 'cooling'
False: If the motor is ready for another step
*/
static inline uint8_t checkForStepping(void)
{
  bool isStepping = false;
  unsigned int timeCheck;
  
  if( (_idleState.idleStepper.stepperStatus == StepperStatus::STEPPING) || (_idleState.idleStepper.stepperStatus == StepperStatus::COOLING) )
  {
    if (_idleState.idleStepper.stepperStatus == StepperStatus::STEPPING)
    {
      timeCheck = _idleState.iacStepTime_uS;
    }
    else 
    {
      timeCheck = _idleState.iacCoolTime_uS;
    }

    if( hasIntervalElapsed(micros(), _idleState.idleStepper.stepStartTime, timeCheck) )
    {         
      if(_idleState.idleStepper.stepperStatus == StepperStatus::STEPPING)
      {
        //Means we're currently in a step, but it needs to be turned off
        digitalWrite(pinNumbers.pinStepperStep, LOW); //Turn off the step
        _idleState.idleStepper.stepStartTime = micros();

	//Set status to StepperStatus::COOLING. In next cycle, status will be set to SOFF and set stepper power OFF based on given settings
        _idleState.idleStepper.stepperStatus = StepperStatus::COOLING; //'Cooling' is the time the stepper needs to sit in LOW state before the next step can be made
                  
        isStepping = true;
      }
      else
      {
        //Means we're in StepperStatus::COOLING status but have been in this state long enough. Go into off state
         _idleState.idleStepper.stepperStatus = StepperStatus::SOFF;
        if(configPage9.iacStepperPower == STEPPER_POWER_WHEN_ACTIVE) 
        { 
          //Disable the DRV8825, but only if we're at the final step in this cycle or within the hysteresis range. 
          if ( (_idleState.idleStepper.curIdleStep >= (_idleState.idleStepper.targetIdleStep - configPage6.iacStepHyster)) && (_idleState.idleStepper.curIdleStep <= (_idleState.idleStepper.targetIdleStep + configPage6.iacStepHyster))) //Hysteresis check
          { 
            digitalWrite(pinNumbers.pinStepperEnable, HIGH); 
          } 
        }
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
static inline void doStep(void)
{
  int16_t error = _idleState.idleStepper.targetIdleStep - _idleState.idleStepper.curIdleStep;
  if ( (error < -((int8_t)configPage6.iacStepHyster)) || (error > configPage6.iacStepHyster) ) //Hysteresis check
  {
    // the home position for a stepper is pintle fully seated, i.e. no airflow.
    if (error < 0)
    {
      // we are moving toward the home position (reducing air)
      digitalWrite(pinNumbers.pinStepperDir, STEPPER_LESS_AIR_DIRECTION() );
      _idleState.idleStepper.curIdleStep--;
    }
    else
    {
      // we are moving away from the home position (adding air).
      digitalWrite(pinNumbers.pinStepperDir, STEPPER_MORE_AIR_DIRECTION() );
      _idleState.idleStepper.curIdleStep++;
    }

    digitalWrite(pinNumbers.pinStepperEnable, LOW); //Enable the DRV8825
    digitalWrite(pinNumbers.pinStepperStep, HIGH);
    _idleState.idleStepper.stepStartTime = micros();
    _idleState.idleStepper.stepperStatus = StepperStatus::STEPPING;
    _idleState.idleOn = true;

    currentStatus.idleOn = true;
  }
  else
  {
    currentStatus.idleOn = false;
  }
}

static inline uint8_t calculateIdleLoad(const config9 &page9, const StepperIdle &idleState)
{
  if (IAC_STEPS.toUser(page9.iacMaxSteps) > (uint16_t)UINT8_MAX ) 
  { 
    return idleState.curIdleStep / 2; 
  }
  return idleState.curIdleStep;
}

/*
Clamps the target step count to the configured max steps (including any idle-up
adder, to prevent over-opening), updates the reported idleLoad and performs a step.
*/
static inline void updateIdleStepAndLoad(statuses &current, const config9 &page9, StepperIdle &idleState)
{
  //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
  idleState.targetIdleStep = clamp((uint16_t)idleState.targetIdleStep, (uint16_t)0U, IAC_STEPS.toUser(page9.iacMaxSteps));
  current.idleLoad = calculateIdleLoad(page9, idleState);
  doStep();
}

/*
Checks whether the stepper has been homed yet. If it hasn't, will handle the next step
Returns:
True: If the system has been homed. No other action is taken
False: If the motor has not yet been homed. Will also perform another homing step.
*/
static inline uint8_t isStepperHomed(void)
{
  bool isHomed = true; //As it's the most common scenario, default value is true
  if( _idleState.completedHomeSteps < (configPage6.iacStepHome * 3) ) //Home steps are divided by 3 from TS
  {
    digitalWrite(pinNumbers.pinStepperDir, STEPPER_LESS_AIR_DIRECTION() ); //homing the stepper closes off the air bleed
    digitalWrite(pinNumbers.pinStepperEnable, LOW); //Enable the DRV8825
    digitalWrite(pinNumbers.pinStepperStep, HIGH);
    _idleState.idleStepper.stepStartTime = micros();
    _idleState.idleStepper.stepperStatus = StepperStatus::STEPPING;
    _idleState.completedHomeSteps++;
    _idleState.idleOn = true;
    isHomed = false;
  }
  return isHomed;
}

void idleControl(void)
{
  if( _idleState.idleInitComplete != configPage6.iacAlgorithm) { initialiseIdle(false); }
  if( (currentStatus.RPM > 0) || (configPage6.iacPWMrun == true) ) { enableIdle(); }

  //Check whether the idleUp is active
  if (configPage2.idleUpEnabled == true)
  {
    if (configPage2.idleUpPolarity == 0) { currentStatus.idleUpActive = !digitalRead(pinNumbers.pinIdleUp); } //Normal mode (ground switched)
    else { currentStatus.idleUpActive = digitalRead(pinNumbers.pinIdleUp); } //Inverted mode (5v activates idleUp)

    if (configPage2.idleUpOutputEnabled  == true)
    {
      if (currentStatus.idleUpActive == true)
      {
        digitalWrite(pinNumbers.pinIdleUpOutput, _idleState.idleUpOutputHIGH);
        currentStatus.idleUpOutputActive = true;
      }
      else
      {
        digitalWrite(pinNumbers.pinIdleUpOutput, _idleState.idleUpOutputLOW);
        currentStatus.idleUpOutputActive = false;
      }      
    }
  }
  else { currentStatus.idleUpActive = false; }

  bool PID_computed = false;
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:      //Case 1 is on/off idle control
      if ( (temperatureAddOffset(currentStatus.coolant)) < configPage6.iacFastTemp) //All temps are offset by 40 degrees
      {
        _idleState.idle_pin.setPinHigh();
        _idleState.idleOn = true;
        currentStatus.idleOn = true;
		    currentStatus.idleLoad = 100;
      }
      else if (_idleState.idleOn)
      {
        _idleState.idle_pin.setPinLow();
        _idleState.idleOn = false; 
        currentStatus.idleOn = false;
		    currentStatus.idleLoad = 0;
      }
      break;

    case IAC_ALGORITHM_PWM_OL:      //Case 2 is PWM open loop
      //Check for cranking pulsewidth
      if( currentStatus.rotationStatus==EngineRotationStatus::Cranking )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
        _idleState.idleTaper = 0;
      }
      else if ( currentStatus.rotationStatus!=EngineRotationStatus::Running)
      {
        if( configPage6.iacPWMrun == true)
        {
          //Engine is not running or cranking, but the run before crank flag is set. Use the cranking table
          currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
          _idleState.idleTaper = 0;
        }
      }
      else
      {
        if ( _idleState.idleTaper < configPage2.idleTaperTime )
        {
          //Tapering between cranking IAC value and running
          currentStatus.idleLoad = map(_idleState.idleTaper, 0, configPage2.idleTaperTime,\
          table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)),\
          table2D_getValue(&iacPWMTable, temperatureAddOffset(currentStatus.coolant)));
          if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_10HZ) ) { _idleState.idleTaper++; }
        }
        else
        {
          //Standard running
          currentStatus.idleLoad = table2D_getValue(&iacPWMTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
        }
        // Add air conditioning idle-up - we only do this if the engine is running (A/C should never engage with engine off).
        if(configPage15.airConIdleSteps>0 && currentStatus.acStatus.turningOn == true) { currentStatus.idleLoad += configPage15.airConIdleSteps; }
      }

      if(currentStatus.idleUpActive == true) { currentStatus.idleLoad += configPage2.idleUpAdder; } //Add Idle Up amount if active
      
      if( currentStatus.idleLoad > 100 ) { currentStatus.idleLoad = 100; } //Safety Check
      _idleState.idle_pwm_target_value = percentage(currentStatus.idleLoad, _idleState.idle_pwm_max_count);
      
      break;

    case IAC_ALGORITHM_PWM_CL:    //Case 3 is PWM closed loop
        //No cranking specific value for closed loop (yet?)
      if( currentStatus.rotationStatus==EngineRotationStatus::Cranking )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
        _idleState.idle_pwm_target_value = percentage(currentStatus.idleLoad, _idleState.idle_pwm_max_count);
        _idleState.idle_pid_target_value = _idleState.idle_pwm_target_value << 2; //Resolution increased
        _idleState.idlePID.reset(currentStatus.RPM); //Update output to smooth transition
      }
      else if ( currentStatus.rotationStatus!=EngineRotationStatus::Running)
      {
        if( configPage6.iacPWMrun == true)
        {
          //Engine is not running or cranking, but the run before crank flag is set. Use the cranking table
          currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
          _idleState.idle_pwm_target_value = percentage(currentStatus.idleLoad, _idleState.idle_pwm_max_count);
        }
      }
      else
      {
        _idleState.idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10; //Multiply the byte target value back out by 10
        if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_1HZ) ) { setIdlePidTunings(configPage6); } //Re-read the PID settings once per second
        
        PID_computed = _idleState.idlePID.compute(millis(), currentStatus.RPM, &_idleState.idle_pid_target_value);
        long TEMP_idle_pwm_target_value;
        if(PID_computed == true)
        {
          TEMP_idle_pwm_target_value = _idleState.idle_pid_target_value;
          
          // Add an offset to the duty cycle, outside of the closed loop. When tuned correctly, the extra load from
          // the air conditioning should exactly cancel this out and the PID loop will be relatively unaffected.
          if(configPage15.airConIdleSteps>0 && currentStatus.acStatus.turningOn == true)
          {
            // Add air conditioning idle-up
            // We are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count
            TEMP_idle_pwm_target_value += percentage(configPage15.airConIdleSteps, _idleState.idle_pwm_max_count<<2);
            if(TEMP_idle_pwm_target_value > (_idleState.idle_pwm_max_count<<2)) { TEMP_idle_pwm_target_value = (_idleState.idle_pwm_max_count<<2); }
          }

          // Fixed this by putting it here, however I have not tested it. It used to be after the calculation of _idleState.idle_pwm_target_value, meaning the percentage would update in currentStatus, but the idle would not actually increase.
          if(currentStatus.idleUpActive == true)
          { 
            // Add Idle Up amount if active
            // Again, we use configPage15.airConIdleSteps * _idleState.idle_pwm_max_count / 100 because we are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count
            TEMP_idle_pwm_target_value += percentage(configPage2.idleUpAdder, _idleState.idle_pwm_max_count<<2);
            if(TEMP_idle_pwm_target_value > (_idleState.idle_pwm_max_count<<2)) { TEMP_idle_pwm_target_value = (_idleState.idle_pwm_max_count<<2); }
          }

          // Now assign the real PWM value
          _idleState.idle_pwm_target_value = TEMP_idle_pwm_target_value>>2; //increased resolution
          currentStatus.idleLoad = fast_div32_16((uint32_t)(_idleState.idle_pwm_target_value * 100UL), _idleState.idle_pwm_max_count);
        }
        _idleState.idleCounter++;
      }
      break;


    case IAC_ALGORITHM_PWM_OLCL: //case 6 is PWM Open Loop table as feedforward term plus closed loop. 
      //No cranking specific value for closed loop (yet?)
      if( currentStatus.rotationStatus==EngineRotationStatus::Cranking )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
        _idleState.idle_pwm_target_value = percentage(currentStatus.idleLoad, _idleState.idle_pwm_max_count);
        _idleState.idle_pid_target_value = _idleState.idle_pwm_target_value << 2; //Resolution increased
        _idleState.idlePID.reset(currentStatus.RPM); //Update output to smooth transition
      }
      else if ( currentStatus.rotationStatus!=EngineRotationStatus::Running)
      {
        if( configPage6.iacPWMrun == true)
        {
          //Engine is not running or cranking, but the run before crank flag is set. Use the cranking table
          currentStatus.idleLoad = table2D_getValue(&iacCrankDutyTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
          _idleState.idle_pwm_target_value = percentage(currentStatus.idleLoad, _idleState.idle_pwm_max_count);
        }
      }
      else
      {
        //Read the OL table as feedforward term
        _idleState.FeedForwardTerm = percentage(table2D_getValue(&iacPWMTable, temperatureAddOffset(currentStatus.coolant)), _idleState.idle_pwm_max_count<<2); //All temps are offset by 40 degrees
        
        // Add an offset to the feed forward term. When tuned correctly, the extra load from the air conditioning
        // should exactly cancel this out and the PID loop will be relatively unaffected.
        if(configPage15.airConIdleSteps>0 && currentStatus.acStatus.turningOn == true)
        {
          // Add air conditioning idle-up
          // We are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count <<2 (PWM count * 4)
          _idleState.FeedForwardTerm += percentage(configPage15.airConIdleSteps, (_idleState.idle_pwm_max_count<<2));
          if(_idleState.FeedForwardTerm > (_idleState.idle_pwm_max_count<<2)) { _idleState.FeedForwardTerm = (_idleState.idle_pwm_max_count<<2); }
        }
        
        // Fixed this by putting it here, however I have not tested it. It used to be after the calculation of _idleState.idle_pwm_target_value, meaning the percentage would update in currentStatus, but the idle would not actually increase.
        if(currentStatus.idleUpActive == true)
        { 
          // Add Idle Up amount if active
          // Again, we are adding percentage steps, but the loop doesn't operate in percentage steps - it works in PWM count <<2 (PWM count * 4)
          _idleState.FeedForwardTerm += percentage(configPage2.idleUpAdder, (_idleState.idle_pwm_max_count<<2));
          if(_idleState.FeedForwardTerm > (_idleState.idle_pwm_max_count<<2)) { _idleState.FeedForwardTerm = (_idleState.idle_pwm_max_count<<2); }
        }
        
    
        _idleState.idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10U; //Multiply the byte target value back out by 10
        if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_1HZ) ) { setIdlePidTunings(configPage6); } //Re-read the PID settings once per second
        if((currentStatus.RPM - _idleState.idle_cl_target_rpm > configPage2.iacRPMlimitHysteresis*10) || (currentStatus.TPS > configPage2.iacTPSlimit)){ //reset integral to zero when TPS is bigger than set value in TS (opening throttle so not idle anymore). OR when RPM higher than Idle Target + RPM Histeresis (coming back from high rpm with throttle closed)
          _idleState.idlePID.resetIntegeral();
        }
        
        _idleState.idlePID.setFeedForwardTerm(_idleState.FeedForwardTerm);
        PID_computed = _idleState.idlePID.compute(millis(), currentStatus.RPM, &_idleState.idle_pid_target_value);

        if(PID_computed == true)
        {
          _idleState.idle_pwm_target_value = _idleState.idle_pid_target_value>>2; //increased resolution
          currentStatus.idleLoad = ((unsigned long)(_idleState.idle_pwm_target_value * 100UL) / _idleState.idle_pwm_max_count);
        }
        _idleState.idleCounter++;
      }
        
    break;


    case IAC_ALGORITHM_STEP_OL:    //Case 4 is open loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if( (checkForStepping() == false) && (isStepperHomed() == true) ) //Check that homing is complete and that there's not currently a step already taking place. MUST BE IN THIS ORDER!
      {
        //Check for cranking pulsewidth
        if( currentStatus.rotationStatus!=EngineRotationStatus::Running ) //If ain't running it means off or cranking
        {
          //Currently cranking. Use the cranking table
          _idleState.idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, temperatureAddOffset(currentStatus.coolant)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
          if(currentStatus.idleUpActive == true) { _idleState.idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
          _idleState.idleTaper = 0;
        }
        else
        {
          //Standard running
          if (BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_10HZ) && (currentStatus.RPM > 0))
          {
            if ( _idleState.idleTaper < configPage2.idleTaperTime )
            {
              //Tapering between cranking IAC value and running
              _idleState.idleStepper.targetIdleStep = map(_idleState.idleTaper, 0, configPage2.idleTaperTime,\
              table2D_getValue(&iacCrankStepsTable, temperatureAddOffset(currentStatus.coolant)) * 3,\
              table2D_getValue(&iacStepTable, temperatureAddOffset(currentStatus.coolant)) * 3);
              if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_10HZ) ) { _idleState.idleTaper++; }
            }
            else
            {
              //Standard running
              _idleState.idleStepper.targetIdleStep = table2D_getValue(&iacStepTable, temperatureAddOffset(currentStatus.coolant)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
            }
            if(currentStatus.idleUpActive == true) { _idleState.idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
            
            // Add air conditioning idle-up - we only do this if the engine is running (A/C should never engage with engine off).
            if(configPage15.airConIdleSteps>0 && currentStatus.acStatus.turningOn == true) { _idleState.idleStepper.targetIdleStep += configPage15.airConIdleSteps; }
            
            _idleState.iacStepTime_uS = configPage6.iacStepTime * 1000;
            _idleState.iacCoolTime_uS = configPage9.iacCoolTime * 1000;
          }
        }
        updateIdleStepAndLoad(currentStatus, configPage9, _idleState.idleStepper);
      }
      break;

    case IAC_ALGORITHM_STEP_OLCL:  //Case 7 is closed+open loop stepper control
    case IAC_ALGORITHM_STEP_CL:    //Case 5 is closed loop stepper control
      //First thing to check is whether there is currently a step going on and if so, whether it needs to be turned off
      if( (checkForStepping() == false) && (isStepperHomed() == true) ) //Check that homing is complete and that there's not currently a step already taking place. MUST BE IN THIS ORDER!
      {
        if( currentStatus.rotationStatus!=EngineRotationStatus::Running ) //If ain't running it means off or cranking
        {
          //Currently cranking. Use the cranking table
          _idleState.idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, temperatureAddOffset(currentStatus.coolant)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
          //Note: Idle Up amount is added to targetIdleStep after this if/else block, common to all engine states. Adding it here as well would apply it twice.

          //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
          if (_idleState.idleStepper.targetIdleStep > (configPage9.iacMaxSteps * 3) )
          {
            _idleState.idleStepper.targetIdleStep = configPage9.iacMaxSteps * 3;
          }
          
          _idleState.idleTaper = 0;
          _idleState.idle_pid_target_value = _idleState.idleStepper.targetIdleStep << 2; //Resolution increased
          _idleState.idlePID.resetIntegeral();
          _idleState.FeedForwardTerm = _idleState.idle_pid_target_value;
        }
        else 
        {
          if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_10HZ) )
          {
            _idleState.idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10; //Multiply the byte target value back out by 10
            if( _idleState.idleTaper < configPage2.idleTaperTime )
            {
              uint16_t minValue = table2D_getValue(&iacCrankStepsTable, temperatureAddOffset(currentStatus.coolant)) * 3;
              if( _idleState.idle_pid_target_value < minValue<<2 ) { _idleState.idle_pid_target_value = minValue<<2; }
              uint16_t maxValue = _idleState.idle_pid_target_value>>2;
              if( configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL ) { maxValue = table2D_getValue(&iacStepTable, temperatureAddOffset(currentStatus.coolant)) * 3; }

              //Tapering between cranking IAC value and running
              _idleState.FeedForwardTerm = map(_idleState.idleTaper, 0, configPage2.idleTaperTime, minValue, maxValue)<<2;
              _idleState.idleTaper++;
              _idleState.idle_pid_target_value = _idleState.FeedForwardTerm;
            }
            else if (configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL)
            {
              //Standard running
              _idleState.FeedForwardTerm = (table2D_getValue(&iacStepTable, temperatureAddOffset(currentStatus.coolant)) * 3)<<2; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
              //reset integral to zero when TPS is bigger than set value in TS (opening throttle so not idle anymore). OR when RPM higher than Idle Target + RPM Hysteresis (coming back from high rpm with throttle closed) 
              if (((currentStatus.RPM - _idleState.idle_cl_target_rpm) > configPage2.iacRPMlimitHysteresis*10) || (currentStatus.TPS > configPage2.iacTPSlimit) || _idleState.lastDFCOValue )
              {
                _idleState.idlePID.resetIntegeral();
              }
            }
            else { _idleState.FeedForwardTerm = _idleState.idle_pid_target_value; }
          }

          _idleState.idlePID.setFeedForwardTerm(_idleState.FeedForwardTerm);
          PID_computed = _idleState.idlePID.compute(millis(), currentStatus.RPM, &_idleState.idle_pid_target_value);

          //If DFCO conditions are met keep output from changing
          if( (currentStatus.TPS > configPage2.iacTPSlimit) || _idleState.lastDFCOValue
          || ((configPage6.iacAlgorithm == IAC_ALGORITHM_STEP_OLCL) && (_idleState.idleTaper < configPage2.idleTaperTime)) )
          {
            _idleState.idle_pid_target_value = _idleState.FeedForwardTerm;
          }
          _idleState.idleStepper.targetIdleStep = _idleState.idle_pid_target_value>>2; //Increase resolution

          // Add air conditioning idle-up - we only do this if the engine is running (A/C should never engage with engine off).
          if(configPage15.airConIdleSteps>0 && currentStatus.acStatus.turningOn == true) { _idleState.idleStepper.targetIdleStep += configPage15.airConIdleSteps; }
        }
        
        if(currentStatus.idleUpActive == true) { _idleState.idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active
        
        updateIdleStepAndLoad(currentStatus, configPage9, _idleState.idleStepper);
      }
      if (BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_1HZ)) //Use timer flag instead idle count
      {
        //This only needs to be run very infrequently, once per second
        setIdlePidTunings(configPage6);
        _idleState.iacStepTime_uS = configPage6.iacStepTime * 1000;
        _idleState.iacCoolTime_uS = configPage9.iacCoolTime * 1000;
      }
      break;

    default:
      //There really should be a valid idle type
      break;
  }
  _idleState.lastDFCOValue = currentStatus.isDFCOActive;

  //Check for 100% and 0% DC on PWM idle
  if (isPwmIac(configPage6))
  {
    if(currentStatus.idleLoad >= 100)
    {
      currentStatus.idleOn = true;
      IDLE_TIMER_DISABLE();
      if (configPage6.iacPWMdir == 0)
      {
        //Normal direction
        _idleState.idle_pin.setPinHigh();  // Switch pin high
        if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinLow(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
      }
      else
      {
        //Reversed direction
        _idleState.idle_pin.setPinLow();  // Switch pin to low
        if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinHigh(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
      }
    }
    else if (currentStatus.idleLoad == 0)
    {
      disableIdle();
    }
    else
    {
      currentStatus.idleOn = true;
      IDLE_TIMER_ENABLE();
    }
  }
}


//This function simply turns off the idle PWM and sets the pin low
void disableIdle(void)
{
  if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) )
  {
    IDLE_TIMER_DISABLE();
    if (configPage6.iacPWMdir == 0)
    {
      //Normal direction
      _idleState.idle_pin.setPinLow();  // Switch pin to low
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinHigh(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }
    else
    {
      //Reversed direction
      _idleState.idle_pin.setPinHigh();  // Switch pin high
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinLow(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }
  }
  else if( isStepperIac(configPage6) )
  {
    //Only disable the stepper motor if homing is completed
    if( (checkForStepping() == false) && (isStepperHomed() == true) )
    {
        /* for open loop stepper we should just move to the cranking position when
           disabling idle, since the only time this function is called in this scenario
           is if the engine stops.
        */
        _idleState.idleStepper.targetIdleStep = table2D_getValue(&iacCrankStepsTable, temperatureAddOffset(currentStatus.coolant)) * 3; //All temps are offset by 40 degrees. Step counts are divided by 3 in TS. Multiply back out here
        if(currentStatus.idleUpActive == true) { _idleState.idleStepper.targetIdleStep += configPage2.idleUpAdder; } //Add Idle Up amount if active?

        //limit to the configured max steps. This must include any idle up adder, to prevent over-opening.
        if (_idleState.idleStepper.targetIdleStep > (configPage9.iacMaxSteps * 3) )
        {
          _idleState.idleStepper.targetIdleStep = configPage9.iacMaxSteps * 3;
        }
        _idleState.idle_pid_target_value = _idleState.idleStepper.targetIdleStep<<2;
    }
  }
  currentStatus.idleOn = false;
  currentStatus.idleLoad = 0;
}

void idleInterrupt(void)
{
  if (_idleState.idle_pwm_state)
  {
    if (configPage6.iacPWMdir == 0)
    {
      //Normal direction
      #if defined (CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
      _idleState.idle_pin.setPinHigh();
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinLow(); }
      #else
      _idleState.idle_pin.setPinLow();  // Switch pin to low (1 pin mode)
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinHigh(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
      #endif
    }
    else
    {
      //Reversed direction
      #if defined (CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
      _idleState.idle_pin.setPinLow();
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinHigh(); }
      #else
      _idleState.idle_pin.setPinHigh();  // Switch pin high
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinLow(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
      #endif
    }
    SET_COMPARE(IDLE_COMPARE, IDLE_COUNTER + (_idleState.idle_pwm_max_count - _idleState.idle_pwm_cur_value) );
    _idleState.idle_pwm_state = false;
  }
  else
  {
    if (configPage6.iacPWMdir == 0)
    {
      //Normal direction
      #if defined (CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
      _idleState.idle_pin.setPinLow();
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinHigh(); }
      #else
      _idleState.idle_pin.setPinHigh();  // Switch pin high
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinLow(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
      #endif
    }
    else
    {
      //Reversed direction
      #if defined (CORE_TEENSY41) //PIT TIMERS count down and have opposite effect on PWM
      _idleState.idle_pin.setPinHigh();
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinLow(); }
      #else
      _idleState.idle_pin.setPinLow();  // Switch pin to low (1 pin mode)
      if(configPage6.iacChannels == 1) { _idleState.idle2_pin.setPinHigh(); } //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
      #endif
    }
    SET_COMPARE(IDLE_COMPARE, IDLE_COUNTER + _idleState.idle_pwm_target_value);
    _idleState.idle_pwm_cur_value = _idleState.idle_pwm_target_value;
    _idleState.idle_pwm_state = true;
  }
}
