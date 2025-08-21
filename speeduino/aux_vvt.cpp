#include "aux_vvt.h"
#include "globals.h"
#include "src/PID_v1/PID_v1.h"
#include "units.h"
#include "decoders.h"
#include "timers.h"

static long vvt1_pwm_value;
long vvt2_pwm_value;
volatile unsigned int vvt1_pwm_cur_value;
volatile unsigned int vvt2_pwm_cur_value;
static long vvt_pid_target_angle;
static long vvt2_pid_target_angle;
static long vvt_pid_current_angle;
static long vvt2_pid_current_angle;
volatile bool vvt1_pwm_state;
volatile bool vvt2_pwm_state;
volatile bool vvt1_max_pwm;
volatile bool vvt2_max_pwm;
volatile char nextVVT;
byte vvtCounter;

uint32_t vvtWarmTime;
bool vvtIsHot;
bool vvtTimeHold;
uint16_t vvt_pwm_max_count; //Used for variable PWM frequency

volatile PORT_TYPE *vvt1_pin_port;
volatile PINMASK_TYPE vvt1_pin_mask;
volatile PORT_TYPE *vvt2_pin_port;
volatile PINMASK_TYPE vvt2_pin_mask;

integerPID vvtPID(&vvt_pid_current_angle, &currentStatus.vvt1Duty, &vvt_pid_target_angle, configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD, configPage6.vvtPWMdir); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
integerPID vvt2PID(&vvt2_pid_current_angle, &currentStatus.vvt2Duty, &vvt2_pid_target_angle, configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD, configPage4.vvt2PWMdir); //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

void initialiseVVT()
{
  vvt1_pin_port = portOutputRegister(digitalPinToPort(pinVVT_1));
  vvt1_pin_mask = digitalPinToBitMask(pinVVT_1);
  vvt2_pin_port = portOutputRegister(digitalPinToPort(pinVVT_2));
  vvt2_pin_mask = digitalPinToBitMask(pinVVT_2);

  if( configPage6.vvtEnabled > 0)
  {
    currentStatus.vvt1Angle = 0;
    currentStatus.vvt2Angle = 0;

    #if defined(CORE_AVR)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (16U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY35)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (32U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY41)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #endif

    if(configPage6.vvtMode == VVT_MODE_CLOSED_LOOP)
    {
      vvtPID.SetOutputLimits(configPage10.vvtCLminDuty, configPage10.vvtCLmaxDuty);
      vvtPID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
      vvtPID.SetSampleTime(33); //30Hz is 33,33ms
      vvtPID.SetMode(AUTOMATIC); //Turn PID on
      if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
      {
        vvt2PID.SetOutputLimits(configPage10.vvtCLminDuty, configPage10.vvtCLmaxDuty);
        vvt2PID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);
        vvt2PID.SetSampleTime(33); //30Hz is 33,33ms
        vvt2PID.SetMode(AUTOMATIC); //Turn PID on
      }
    }

    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
    vvtTimeHold = false;
    if (currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) { vvtIsHot = true; } //Checks to see if coolant's already at operating temperature
  }

  if( (configPage6.vvtEnabled == 0) && (configPage10.wmiEnabled >= 1) )
  {
    // config wmi pwm output to use vvt output
    #if defined(CORE_AVR)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (16U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY35)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (32U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 16uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #elif defined(CORE_TEENSY41)
      vvt_pwm_max_count = (uint16_t)(MICROS_PER_SEC / (2U * configPage6.vvtFreq * 2U)); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 512hz
    #endif
    BIT_CLEAR(currentStatus.status4, BIT_STATUS4_WMI_EMPTY);
    currentStatus.wmiPW = 0;
    vvt1_pwm_value = 0;
    vvt2_pwm_value = 0;
    ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)
  }

  currentStatus.vvt1Duty = 0;
  currentStatus.vvt2Duty = 0;
  vvtCounter = 0;
}

void vvtControl(void)
{
  if( (configPage6.vvtEnabled == 1) && (currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) && (BIT_CHECK(currentStatus.engine, BIT_ENGINE_RUN)))
  {
    if(vvtTimeHold == false) 
    {
      vvtWarmTime = runSecsX10;
      vvtTimeHold = true;
    }

    //Calculate the current cam angle for miata trigger
    if( configPage4.TrigPattern == 9 ) { currentStatus.vvt1Angle = getCamAngle_Miata9905(); }

    if( (vvtIsHot == true) || ((runSecsX10 - vvtWarmTime) >= (configPage4.vvtDelay * VVT_TIME_DELAY_MULTIPLIER)) ) 
    {
      vvtIsHot = true;

      if( (configPage6.vvtMode == VVT_MODE_OPEN_LOOP) || (configPage6.vvtMode == VVT_MODE_ONOFF) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1Duty = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentStatus.vvt1Duty = get3DTableValue(&vvtTable, (uint16_t)currentStatus.MAP, currentStatus.RPM); }

        //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
        if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt1Duty < 200) ) { currentStatus.vvt1Duty = 0; }

        vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          //Lookup VVT duty based on either MAP or TPS
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2Duty = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentStatus.vvt2Duty = get3DTableValue(&vvt2Table, (uint16_t)currentStatus.MAP, currentStatus.RPM); }

          //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
          if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt2Duty < 200) ) { currentStatus.vvt2Duty = 0; }

          vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
        }

      } //Open loop
      else if( (configPage6.vvtMode == VVT_MODE_CLOSED_LOOP) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1TargetAngle = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentStatus.vvt1TargetAngle = get3DTableValue(&vvtTable, (uint16_t)currentStatus.MAP, currentStatus.RPM); }

        if( (vvtCounter & 31) == 1) { vvtPID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);  //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
        vvtPID.SetControllerDirection(configPage6.vvtPWMdir); }

        // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
        // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
        if ( currentStatus.vvt1Angle <=  configPage10.vvtCLMinAng || currentStatus.vvt1Angle > configPage10.vvtCLMaxAng )
        {
          currentStatus.vvt1Duty = 0;
          vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);
          BIT_SET(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
        }
        //Check that we're not already at the angle we want to be
        else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt1TargetAngle == currentStatus.vvt1Angle) )
        {
          currentStatus.vvt1Duty = configPage10.vvtCLholdDuty;
          vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count);
          vvtPID.Initialize();
          BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
        }
        else
        {
          //This is dumb, but need to convert the current angle into a long pointer.
          vvt_pid_target_angle = (unsigned long)currentStatus.vvt1TargetAngle;
          vvt_pid_current_angle = (long)currentStatus.vvt1Angle;

          //If not already at target angle, calculate new value from PID
          bool PID_compute = vvtPID.Compute(true);
          //vvtPID.Compute2(currentStatus.vvt1TargetAngle, currentStatus.vvt1Angle, false);
          //vvt_pwm_target_value = percentage(40, vvt_pwm_max_count);
          //if (currentStatus.vvt1Angle > currentStatus.vvt1TargetAngle) { vvt_pwm_target_value = 0; }
          if(PID_compute == true) { vvt1_pwm_value = halfPercentage(currentStatus.vvt1Duty, vvt_pwm_max_count); }
          BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT1_ERROR);
        }

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2TargetAngle = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentStatus.vvt2TargetAngle = get3DTableValue(&vvt2Table, (uint16_t)currentStatus.MAP, currentStatus.RPM); }

          if( (vvtCounter & 31) == 1) { vvt2PID.SetTunings(configPage10.vvtCLKP, configPage10.vvtCLKI, configPage10.vvtCLKD);  //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
          vvt2PID.SetControllerDirection(configPage4.vvt2PWMdir); }

          // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
          // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
          if ( currentStatus.vvt2Angle <= configPage10.vvtCLMinAng || currentStatus.vvt2Angle > configPage10.vvtCLMaxAng )
          {
            currentStatus.vvt2Duty = 0;
            vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
            BIT_SET(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
          }
          //Check that we're not already at the angle we want to be
          else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt2TargetAngle == currentStatus.vvt2Angle) )
          {
            currentStatus.vvt2Duty = configPage10.vvtCLholdDuty;
            vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count);
            vvt2PID.Initialize();
            BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
          }
          else
          {
            //This is dumb, but need to convert the current angle into a long pointer.
            vvt2_pid_target_angle = (unsigned long)currentStatus.vvt2TargetAngle;
            vvt2_pid_current_angle = (long)currentStatus.vvt2Angle;
            //If not already at target angle, calculate new value from PID
            bool PID_compute = vvt2PID.Compute(true);
            if(PID_compute == true) { vvt2_pwm_value = halfPercentage(currentStatus.vvt2Duty, vvt_pwm_max_count); }
            BIT_CLEAR(currentStatus.status4, BIT_STATUS4_VVT2_ERROR);
          }
        }
        vvtCounter++;
      }

      //Set the PWM state based on the above lookups
      if( configPage10.wmiEnabled == 0 ) //Added possibility to use vvt and wmi at the same time
      {
        if( (currentStatus.vvt1Duty == 0) && (currentStatus.vvt2Duty == 0) )
        {
          //Make sure solenoid is off (0% duty)
          VVT1_PIN_OFF();
          VVT2_PIN_OFF();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
          vvt2_pwm_state = false;
          vvt2_max_pwm = false;
          DISABLE_VVT_TIMER();
        }
        else if( (currentStatus.vvt1Duty >= 200) && (currentStatus.vvt2Duty >= 200) )
        {
          //Make sure solenoid is on (100% duty)
          VVT1_PIN_ON();
          VVT2_PIN_ON();
          vvt1_pwm_state = true;
          vvt1_max_pwm = true;
          vvt2_pwm_state = true;
          vvt2_max_pwm = true;
          DISABLE_VVT_TIMER();
        }
        else
        {
          //Duty cycle is between 0 and 100. Make sure the timer is enabled
          ENABLE_VVT_TIMER();
          if(currentStatus.vvt1Duty < 200) { vvt1_max_pwm = false; }
          if(currentStatus.vvt2Duty < 200) { vvt2_max_pwm = false; }
        }
      }
      else
      {
        if( currentStatus.vvt1Duty == 0 )
        {
          //Make sure solenoid is off (0% duty)
          VVT1_PIN_OFF();
          vvt1_pwm_state = false;
          vvt1_max_pwm = false;
        }
        else if( currentStatus.vvt1Duty >= 200 )
        {
          //Make sure solenoid is on (100% duty)
          VVT1_PIN_ON();
          vvt1_pwm_state = true;
          vvt1_max_pwm = true;
        }
        else
        {
          //Duty cycle is between 0 and 100. Make sure the timer is enabled
          ENABLE_VVT_TIMER();
          if(currentStatus.vvt1Duty < 200) { vvt1_max_pwm = false; }
        }
      }
    }
  }
  else 
  { 
    if (configPage10.wmiEnabled == 0)
    {
      // Disable timer channel
      DISABLE_VVT_TIMER();
      currentStatus.vvt2Duty = 0;
      vvt2_pwm_value = 0;
      vvt2_pwm_state = false;
      vvt2_max_pwm = false;
    }
    currentStatus.vvt1Duty = 0;
    vvt1_pwm_value = 0;
    vvt1_pwm_state = false;
    vvt1_max_pwm = false;
    vvtTimeHold = false;
  } 
}

//The interrupt to control the VVT PWM
#if defined(CORE_AVR)
  ISR(TIMER1_COMPB_vect) //cppcheck-suppress misra-c2012-8.2
#else
  void vvtInterrupt(void) //Most ARM chips can simply call a function
#endif
{
  if ( ((vvt1_pwm_state == false) || (vvt1_max_pwm == true)) && ((vvt2_pwm_state == false) || (vvt2_max_pwm == true)) )
  {
    if( (vvt1_pwm_value > 0) && (vvt1_max_pwm == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      VVT1_PIN_OFF();
      #else
      VVT1_PIN_ON();
      #endif
      vvt1_pwm_state = true;
    }
    if( (vvt2_pwm_value > 0) && (vvt2_max_pwm == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      VVT2_PIN_OFF();
      #else
      VVT2_PIN_ON();
      #endif
      vvt2_pwm_state = true;
    }

    if( (vvt1_pwm_state == true) && ((vvt1_pwm_value <= vvt2_pwm_value) || (vvt2_pwm_state == false)) )
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt1_pwm_value);
      vvt1_pwm_cur_value = vvt1_pwm_value;
      vvt2_pwm_cur_value = vvt2_pwm_value;
      if (vvt1_pwm_value == vvt2_pwm_value) { nextVVT = 2; } //Next event is for both PWM
      else { nextVVT = 0; } //Next event is for PWM0
    }
    else if( vvt2_pwm_state == true )
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt2_pwm_value);
      vvt1_pwm_cur_value = vvt1_pwm_value;
      vvt2_pwm_cur_value = vvt2_pwm_value;
      nextVVT = 1; //Next event is for PWM1
    }
    else { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt_pwm_max_count); } //Shouldn't ever get here
  }
  else
  {
    if(nextVVT == 0)
    {
      if(vvt1_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        VVT1_PIN_ON();
        #else
        VVT1_PIN_OFF();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
      }
      else { vvt1_max_pwm = true; }
      nextVVT = 1; //Next event is for PWM1
      if(vvt2_pwm_state == true){ SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt2_pwm_cur_value - vvt1_pwm_cur_value) ); }
      else
      { 
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else if (nextVVT == 1)
    {
      if(vvt2_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        VVT2_PIN_ON();
        #else
        VVT2_PIN_OFF();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
      }
      else { vvt2_max_pwm = true; }
      nextVVT = 0; //Next event is for PWM0
      if(vvt1_pwm_state == true) { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt1_pwm_cur_value - vvt2_pwm_cur_value) ); }
      else
      { 
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else
    {
      if(vvt1_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
       #if defined(CORE_TEENSY41)
        VVT1_PIN_ON();
        #else
        VVT1_PIN_OFF();
        #endif
        vvt1_pwm_state = false;
        vvt1_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt1_pwm_cur_value) );
      }
      else { vvt1_max_pwm = true; }
      if(vvt2_pwm_value < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        VVT2_PIN_ON();
        #else
        VVT2_PIN_OFF();
        #endif
        vvt2_pwm_state = false;
        vvt2_max_pwm = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvt2_pwm_cur_value) );
      }
      else { vvt2_max_pwm = true; }
    }
  }
}