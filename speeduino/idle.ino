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

  //Pin masks must always be initialized, regardless of whether PWM idle is used. This is required for STM32 to prevent issues if the IRQ function fires on restat/overflow
  idle_pin_port = portOutputRegister(digitalPinToPort(pinIdle1));
  idle_pin_mask = digitalPinToBitMask(pinIdle1);
  idle2_pin_port = portOutputRegister(digitalPinToPort(pinIdle2));
  idle2_pin_mask = digitalPinToBitMask(pinIdle2);
  lastIdleUpValue = false;
  //Initialising comprises of setting the 2D tables with the relevant values from the config pages
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       
      //Case 0 is no idle control ('None')
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
      iacPWMTable.axisSize = SIZE_BYTE;
      iacPWMTable.values = configPage6.iacOLPWMVal;
      iacPWMTable.axisX = configPage6.iacBins;


      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.valueSize = SIZE_BYTE;
      iacCrankDutyTable.axisSize = SIZE_BYTE;
      iacCrankDutyTable.values = configPage6.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage6.iacCrankBins;

      #if defined(CORE_AVR)
        idle_pwm_max_count = 1000000L / (16 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #elif defined(CORE_TEENSY35)
        idle_pwm_max_count = 1000000L / (32 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 32uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #elif defined(CORE_TEENSY41)
        idle_pwm_max_count = 1000000L / (2 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #endif
      enableIdle();
      break;

    case IAC_ALGORITHM_PWM_CL:
      //Case 3 is PWM closed loop
      iacClosedLoopTable.xSize = 10;
      iacClosedLoopTable.valueSize = SIZE_BYTE;
      iacClosedLoopTable.axisSize = SIZE_BYTE;
      iacClosedLoopTable.values = configPage6.iacCLValues;
      iacClosedLoopTable.axisX = configPage6.iacBins;

      iacCrankDutyTable.xSize = 4;
      iacCrankDutyTable.valueSize = SIZE_BYTE;
      iacCrankDutyTable.axisSize = SIZE_BYTE;
      iacCrankDutyTable.values = configPage6.iacCrankDuty;
      iacCrankDutyTable.axisX = configPage6.iacCrankBins;

      #if defined(CORE_AVR)
        idle_pwm_max_count = 1000000L / (16 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #elif defined(CORE_TEENSY)
        idle_pwm_max_count = 1000000L / (32 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 32uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #elif defined(CORE_TEENSY41)
        idle_pwm_max_count = 1000000L / (2 * configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
      #endif
      idlePID.SetOutputLimits(percentage(configPage2.iacCLminDuty, idle_pwm_max_count<<2), percentage(configPage2.iacCLmaxDuty, idle_pwm_max_count<<2));
      idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD);
      idlePID.SetMode(AUTOMATIC); //Turn PID on

      idleCounter = 0;
      break;

    
    default:
      //Well this just shouldn't happen
      break;
  }
  idleInitComplete = configPage6.iacAlgorithm; //Sets which idle method was initialised
  currentStatus.idleLoad = 0;
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

  bool PID_computed = false;
  switch(configPage6.iacAlgorithm)
  {
    case IAC_ALGORITHM_NONE:       //Case 0 is no idle control ('None')
      break;

    case IAC_ALGORITHM_ONOFF:      //Case 1 is on/off idle control
      if ( (currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET) < configPage6.iacFastTemp) //All temps are offset by 40 degrees
      {
        digitalWrite(pinIdle1, HIGH);
        idleOn = true;
        BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
      }
      else if (idleOn)
      {
        digitalWrite(pinIdle1, LOW); 
        idleOn = false; 
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
      }
      break;

    case IAC_ALGORITHM_PWM_OL:      //Case 2 is PWM open loop
      //Check for cranking pulsewidth
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleDuty = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        
	  }
      else
      {
        
        if ( runSecsX10 < configPage2.idleTaperTime )
        {
          //Tapering between cranking IAC value and running
          currentStatus.idleDuty = map(runSecsX10, 0, configPage2.idleTaperTime,\
          table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET),\
          table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET));
        }
        else
        {
          //Standard running
          currentStatus.idleDuty = table2D_getValue(&iacPWMTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        //Sanders
       
          if (idleCounter < 20) {
                idleCounter ++;
               }
               if (idleCounter == 12){
               digitalWrite(pinACrelay, HIGH);
               }
               if (idleCounter == 19){
               digitalWrite(pinFan2, HIGH);           
               }
        }
      }
	  if(currentStatus.idleUpActive == true) { currentStatus.idleDuty += configPage2.idleUpAdder;}
      if (currentStatus.idleUpActive == false)
        {          
          if (idleCounter != 0){
            digitalWrite(pinFan2, LOW);
            digitalWrite(pinACrelay, LOW);
            idleCounter = 0;
          }     
        } 
      

      if( currentStatus.idleDuty > 100 ) { currentStatus.idleDuty = 100; } //Safety Check
      if( currentStatus.idleDuty == 0 ) 
      { 
        disableIdle();
        BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag off
        break; 
      }
      BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
      idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
      currentStatus.idleLoad = currentStatus.idleDuty;
      idleOn = true;      
      break;

    
    
    
    case IAC_ALGORITHM_PWM_CL:    //Case 3 is PWM closed loop
        //No cranking specific value for closed loop (yet?)
      if( BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK) )
      {
        //Currently cranking. Use the cranking table
        currentStatus.idleDuty = table2D_getValue(&iacCrankDutyTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        currentStatus.idleLoad = currentStatus.idleDuty;
        idle_pwm_target_value = percentage(currentStatus.idleDuty, idle_pwm_max_count);
        idle_pid_target_value = idle_pwm_target_value << 2; //Resolution increased
        idlePID.Initialize(); //Update output to smooth transition
      }
      else
      {
        currentStatus.CLIdleTarget = (byte)table2D_getValue(&iacClosedLoopTable, currentStatus.coolant + CALIBRATION_TEMPERATURE_OFFSET); //All temps are offset by 40 degrees
        
        //added this line to increase idle speed when AC is on        
        if(currentStatus.idleUpActive == true)
         {           
          if ( acDelayCounter <= 20 )
           {
             if ( acDelayCounter == 12) {digitalWrite(pinACrelay, HIGH);}
             if ( acDelayCounter == 19) {digitalWrite(pinFan2, HIGH);}
             acDelayCounter++;
           }
         }
         if ( lastIdleUpValue != currentStatus.idleUpActive)   //check to see if buton has been pressed
           {
            if(currentStatus.idleUpActive == true) //just transitioned to true
              {
                acDelayCounter = 0; //set a variable to the current idleCounter value to delay the fan and clutch
              }
            if(currentStatus.idleUpActive == false) 
              {                 
                digitalWrite(pinACrelay, LOW);
                digitalWrite(pinFan2, LOW);
              }
            }
            
                      
        
        idle_cl_target_rpm = (uint16_t)currentStatus.CLIdleTarget * 10; //Multiply the byte target value back out by 10
        if( (idleCounter & 31) == 1) { idlePID.SetTunings(configPage6.idleKP, configPage6.idleKI, configPage6.idleKD); } //This only needs to be run very infrequently, once every 32 calls to idleControl(). This is approx. once per second

        PID_computed = idlePID.Compute(true);
        if(PID_computed == true)
        {
          idle_pwm_target_value = idle_pid_target_value>>2; //increased resolution
          if( idle_pwm_target_value == 0 )
          { 
            disableIdle(); 
            BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag off
            break; 
          }
          BIT_SET(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag on
          currentStatus.idleLoad = ((unsigned long)(idle_pwm_target_value * 100UL) / idle_pwm_max_count);          
        }
        idleCounter++;
        lastIdleUpValue = currentStatus.idleUpActive;  //used to see when button changes states
      }  
      break;

      default:
        //shouldn't happen
        break;
  }
}

   

//This function simply turns off the idle PWM and sets the pin low
static inline void disableIdle()
{
      IDLE_TIMER_DISABLE();
    digitalWrite(pinIdle1, LOW);
  
  
  BIT_CLEAR(currentStatus.spark, BIT_SPARK_IDLE); //Turn the idle control flag off
  currentStatus.idleLoad = 0;
}

//Any common functions associated with starting the Idle
//Typically this is enabling the PWM interrupt
static inline void enableIdle() { IDLE_TIMER_ENABLE(); }

#if defined(CORE_AVR) //AVR chips use the ISR for this
ISR(TIMER1_COMPC_vect)
#else
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
