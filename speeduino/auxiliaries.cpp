/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/
#include "auxiliaries.h"
#include "globals.h"
#include "elapsed_time.h"
#include "maths.h"
#include "src/PID/integerPID.h"
#include "src/PID/integerPID_ideal.h"
#include "decoders.h"
#include "timers.h"
#include "preprocessor.h"
#include "units.h"
#include "atomic.h"
#include "src/pins/fastInputPin.h"
#include "src/pins/boardOutputPin.h"
#include "scheduler_fuel_controller.h"
#include "src/controllers/vvt/VvtOutputChannel.h"

TESTABLE_STATIC VvtOutputChannel vvtChannel1;
TESTABLE_STATIC VvtOutputChannel vvtChannel2;
TESTABLE_STATIC volatile char nextVVT;

static bool isWmiTankEmpty(void)
{
  if (configPage10.wmiEmptyEnabled) 
  {
    return (configPage10.wmiEmptyPolarity) ? digitalRead(pinNumbers.pinWMIEmpty) : !digitalRead(pinNumbers.pinWMIEmpty);
  }
  return true;
}

TESTABLE_STATIC uint32_t vvtWarmTime;
TESTABLE_STATIC bool vvtIsHot;
TESTABLE_STATIC bool vvtTimeHold;
TESTABLE_STATIC uint16_t vvt_pwm_max_count; //Used for variable PWM frequency

static integerPID vvtPID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
static integerPID vvt2PID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

static void setVvtPidTunings(integerPID &pid, const config10 &page10, bool isReverse)
{
  // LCOV_EXCL_BR_START
  int8_t multiplier = isReverse ? 1 : -1;
  pid.setTunings(PidTuningParameters(page10.vvtCLKP, page10.vvtCLKI, page10.vvtCLKD) * multiplier, millis(), 33);
  // LCOV_EXCL_BR_STOP
}

static void initialiseVvtPid(integerPID &pid, const config10 &page10, bool isReverse, int16_t currentAngle)
{
  pid.setOutputLimits(page10.vvtCLminDuty, page10.vvtCLmaxDuty);
  setVvtPidTunings(pid, page10, isReverse);
  pid.activate(currentAngle); //Turn PID on
}

void __attribute__((optimize("Os"))) initialiseAuxPWM(void)
{
  vvt_pwm_max_count = pwmFreqToTicks(FREQUENCY.toUser(configPage6.vvtFreq));

  vvtChannel1 = VvtOutputChannel();
  vvtChannel1.pin.setPin(pinNumbers.pinVVT_1, OUTPUT);
  vvtChannel2 = VvtOutputChannel();
  vvtChannel2.pin.setPin(pinNumbers.pinVVT_2, OUTPUT);

  currentStatus.wmiTankEmpty = false;
  currentStatus.wmiPW = 0;
  currentStatus.vvt1 = vvtStatus_t();
  currentStatus.vvt2 = vvtStatus_t();
  vvtTimeHold = false;
  vvtIsHot = currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt);
  vvtWarmTime = 0;

  if ((configPage6.vvtEnabled) && (configPage6.vvtMode == VVT_MODE_CLOSED_LOOP))
    {
      initialiseVvtPid(vvtPID, configPage10, configPage6.vvtPWMdir, currentStatus.vvt1.angle);
      if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
      {
        initialiseVvtPid(vvt2PID, configPage10, configPage4.vvt2PWMdir, currentStatus.vvt2.angle);
      }
    }

  if( (configPage6.vvtEnabled) || (configPage10.wmiEnabled) )
  {
    ENABLE_VVT_TIMER(); //Turn on the B compare unit (ie turn on the interrupt)
  }
}

void vvt1On(void)
{
  vvtChannel1.pin.setPinHigh();
}
void vvt1Off(void)
{
  vvtChannel1.pin.setPinLow();
}
void vvt2On(void)
{
  vvtChannel2.pin.setPinHigh();
}
void vvt2Off(void)
{
  vvtChannel2.pin.setPinLow();
}

void vvtControl(void)
{
  if( (configPage6.vvtEnabled == 1) && (currentStatus.coolant >= temperatureRemoveOffset(configPage4.vvtMinClt)) && (currentStatus.rotationStatus==EngineRotationStatus::Running))
  {
    if(vvtTimeHold == false) 
    {
      vvtWarmTime = runSecsX10;
      vvtTimeHold = true;
    }

    //Calculate the current cam angle for miata trigger
    if( configPage4.TrigPattern == 9 ) { currentStatus.vvt1.angle = getCamAngle_Miata9905(); }

    constexpr uint32_t VVT_TIME_DELAY_MULTIPLIER = 50;
    if( (vvtIsHot == true) || hasIntervalElapsed(runSecsX10, vvtWarmTime, configPage4.vvtDelay * VVT_TIME_DELAY_MULTIPLIER) )
    {
      vvtIsHot = true;

      if( (configPage6.vvtMode == VVT_MODE_OPEN_LOOP) || (configPage6.vvtMode == VVT_MODE_ONOFF) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1.duty = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentStatus.vvt1.duty = get3DTableValue(&vvtTable, currentStatus.MAP, currentStatus.RPM); }

        //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
        if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt1.duty < 200) ) { currentStatus.vvt1.duty = 0; }

        vvtChannel1.targetDuty = halfPercentage(currentStatus.vvt1.duty, vvt_pwm_max_count);

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          //Lookup VVT duty based on either MAP or TPS
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2.duty = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentStatus.vvt2.duty = get3DTableValue(&vvt2Table, currentStatus.MAP, currentStatus.RPM); }

          //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
          if( (configPage6.vvtMode == VVT_MODE_ONOFF) && (currentStatus.vvt2.duty < 200) ) { currentStatus.vvt2.duty = 0; }

          vvtChannel2.targetDuty = halfPercentage(currentStatus.vvt2.duty, vvt_pwm_max_count);
        }

      } //Open loop
      else if( (configPage6.vvtMode == VVT_MODE_CLOSED_LOOP) )
      {
        //Lookup VVT duty based on either MAP or TPS
        if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt1.targetAngle = get3DTableValue(&vvtTable, (currentStatus.TPS * 2U), currentStatus.RPM); }
        else { currentStatus.vvt1.targetAngle = get3DTableValue(&vvtTable, currentStatus.MAP, currentStatus.RPM); }

        if(BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_1HZ)) { //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
          setVvtPidTunings(vvtPID, configPage10, configPage6.vvtPWMdir);  
        }

        // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
        // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
        if ( currentStatus.vvt1.angle <=  configPage10.vvtCLMinAng || currentStatus.vvt1.angle > configPage10.vvtCLMaxAng )
        {
          currentStatus.vvt1.duty = 0;
          vvtChannel1.targetDuty = halfPercentage(currentStatus.vvt1.duty, vvt_pwm_max_count);
          currentStatus.vvt1.angleError = true;
        }
        //Check that we're not already at the angle we want to be
        else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt1.targetAngle == currentStatus.vvt1.angle) )
        {
          currentStatus.vvt1.duty = configPage10.vvtCLholdDuty;
          vvtChannel1.targetDuty = halfPercentage(currentStatus.vvt1.duty, vvt_pwm_max_count);
          vvtPID.reset(currentStatus.vvt1.angle);
          currentStatus.vvt1.angleError = false;
        }
        else
        {
          //If not already at target angle, calculate new value from PID
          int32_t pidOutput = 0;
          vvtPID.setSetPoint(currentStatus.vvt1.targetAngle);
          bool PID_compute = vvtPID.compute(millis(), currentStatus.vvt1.angle, &pidOutput);
          if(PID_compute == true) 
          { 
            currentStatus.vvt1.duty = (uint8_t)pidOutput;
            vvtChannel1.targetDuty = halfPercentage(currentStatus.vvt1.duty, vvt_pwm_max_count); 
          }
          currentStatus.vvt1.angleError = false;
        }

        if (configPage10.vvt2Enabled == 1) // same for VVT2 if it's enabled
        {
          if(configPage6.vvtLoadSource == VVT_LOAD_TPS) { currentStatus.vvt2.targetAngle = get3DTableValue(&vvt2Table, (currentStatus.TPS * 2U), currentStatus.RPM); }
          else { currentStatus.vvt2.targetAngle = get3DTableValue(&vvt2Table, currentStatus.MAP, currentStatus.RPM); }

          if( BIT_CHECK(currentStatus.LOOP_TIMER, BIT_TIMER_1HZ)) { //This only needs to be run very infrequently, once every 32 calls to vvtControl(). This is approx. once per second
            setVvtPidTunings(vvt2PID, configPage10, configPage4.vvt2PWMdir);
        }

          // safety check that the cam angles are ok. The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam angles, so if that happens, default to 0 duty.
          // This also prevents using zero or negative current angle values for PID adjustment, because those don't work in integer PID.
          if ( currentStatus.vvt2.angle <= configPage10.vvtCLMinAng || currentStatus.vvt2.angle > configPage10.vvtCLMaxAng )
          {
            currentStatus.vvt2.duty = 0;
            vvtChannel2.targetDuty = halfPercentage(currentStatus.vvt2.duty, vvt_pwm_max_count);
            currentStatus.vvt2.angleError = true;
          }
          //Check that we're not already at the angle we want to be
          else if((configPage6.vvtCLUseHold > 0) && (currentStatus.vvt2.targetAngle == currentStatus.vvt2.angle) )
          {
            currentStatus.vvt2.duty = configPage10.vvtCLholdDuty;
            vvtChannel2.targetDuty = halfPercentage(currentStatus.vvt2.duty, vvt_pwm_max_count);
            vvt2PID.reset(currentStatus.vvt2.angle);
            currentStatus.vvt2.angleError = false;
          }
          else
          {
            vvt2PID.setSetPoint(currentStatus.vvt2.targetAngle);
            //If not already at target angle, calculate new value from PID
            int32_t pidOutput = 0;
            bool PID_compute = vvt2PID.compute(millis(), currentStatus.vvt2.angle, &pidOutput);
            if(PID_compute == true) 
            { 
              currentStatus.vvt2.duty = (uint8_t)pidOutput;
              vvtChannel2.targetDuty = halfPercentage(currentStatus.vvt2.duty, vvt_pwm_max_count); 
            }
            currentStatus.vvt2.angleError = false;
          }
        }
      }

      //Set the PWM state based on the above lookups
      if( configPage10.wmiEnabled == 0 ) //Added possibility to use vvt and wmi at the same time
      {
        if( (currentStatus.vvt1.duty == 0) && (currentStatus.vvt2.duty == 0) )
        {
          //Make sure solenoid is off (0% duty)
          vvtChannel1.pin.setPinLow();
          vvtChannel2.pin.setPinLow();
          vvtChannel1.periodTicks = false;
          vvtChannel2.periodTicks = false;
          DISABLE_VVT_TIMER();
        }
        else if( (currentStatus.vvt1.duty >= 200) && (currentStatus.vvt2.duty >= 200) )
        {
          //Make sure solenoid is on (100% duty)
          vvtChannel1.pin.setPinHigh();
          vvtChannel2.pin.setPinHigh();
          vvtChannel1.periodTicks = true;
          vvtChannel2.periodTicks = true;
          DISABLE_VVT_TIMER();
        }
        else
        {
          //Duty cycle is between 0 and 100. Make sure the timer is enabled
          ENABLE_VVT_TIMER();
          if(currentStatus.vvt1.duty < 200) { vvtChannel1.periodTicks = false; }
          if(currentStatus.vvt2.duty < 200) { vvtChannel2.periodTicks = false; }
        }
      }
      else
      {
        if( currentStatus.vvt1.duty == 0 )
        {
          //Make sure solenoid is off (0% duty)
          vvtChannel1.pin.setPinLow();
          vvtChannel1.periodTicks = false;
        }
        else if( currentStatus.vvt1.duty >= 200 )
        {
          //Make sure solenoid is on (100% duty)
          vvtChannel1.pin.setPinHigh();
          vvtChannel1.periodTicks = true;
        }
        else
        {
          //Duty cycle is between 0 and 100. Make sure the timer is enabled
          ENABLE_VVT_TIMER();
          if(currentStatus.vvt1.duty < 200) { vvtChannel1.periodTicks = false; }
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
      currentStatus.vvt2.duty = 0;
      vvtChannel2.targetDuty = 0;
      vvtChannel2.pin.setPinLow();
      vvtChannel2.periodTicks = false;
    }
    currentStatus.vvt1.duty = 0;
    vvtChannel1.targetDuty = 0;
    vvtChannel1.pin.setPinLow();
    vvtChannel1.periodTicks = false;
    vvtTimeHold = false;
  } 
}

// Water methanol injection control
void wmiControl(void)
{
  int wmiPW = 0;
  
  // wmi can only work when vvt2 is disabled 
  if( (configPage10.vvt2Enabled == 0) && (configPage10.wmiEnabled >= 1) )
  {
    if( isWmiTankEmpty() )
    {
     currentStatus.wmiTankEmpty = false;
      if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPMdiv100 >= configPage10.wmiRPM) && ( (currentStatus.MAP / 2U) >= configPage10.wmiMAP) && ( temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT) )
      {
        switch(configPage10.wmiMode)
        {
        case WMI_MODE_SIMPLE:
          // Simple mode - Output is turned on when preset boost level is reached
          wmiPW = 200;
          break;
        case WMI_MODE_PROPORTIONAL:
          // Proportional Mode - Output PWM is proportionally controlled between two MAP values - MAP Value 1 = PWM:0% / MAP Value 2 = PWM:100%
          wmiPW = map(currentStatus.MAP/2U, configPage10.wmiMAP, configPage10.wmiMAP2, 0, 200);
          break;
        case WMI_MODE_OPENLOOP:
          //  Mapped open loop - Output PWM follows 2D map value (RPM vs MAP) Cell value contains desired PWM% [range 0-100%]
          wmiPW = get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM);
          break;
        case WMI_MODE_CLOSEDLOOP:
          // Mapped closed loop - Output PWM follows injector duty cycle with 2D correction map applied (RPM vs MAP). Cell value contains correction value% [nom 100%] 
          wmiPW = max(0, ((int)fuelSchedule1.pw + configPage10.wmiOffset)) * get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM) / 200;
          break;
        default:
          // Wrong mode
          wmiPW = 0;
          break;
        }
        if (wmiPW > 200) { wmiPW = 200; } //without this the duty can get beyond 100%
      }
    }
    else { currentStatus.wmiTankEmpty = true; }

    currentStatus.wmiPW = wmiPW;
    vvtChannel2.targetDuty = halfPercentage(currentStatus.wmiPW, vvt_pwm_max_count);

    if(wmiPW == 0)
    {
      // Make sure water pump is off
      vvtChannel2.pin.setPinLow();
      vvtChannel2.periodTicks = false;
      if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      digitalWrite(pinNumbers.pinWMIEnabled, LOW);
    }
    else
    {
      digitalWrite(pinNumbers.pinWMIEnabled, HIGH);
      if (wmiPW >= 200)
      {
        // Make sure water pump is on (100% duty)
        vvtChannel2.pin.setPinHigh();
        vvtChannel2.periodTicks = true;
        if( configPage6.vvtEnabled == 0 ) { DISABLE_VVT_TIMER(); }
      }
      else
      {
        vvtChannel2.periodTicks = false;
        ENABLE_VVT_TIMER();
      }
    }
  }
}

//The interrupt to control the VVT PWM
void vvtInterrupt(void)
{
  if ( ((vvtChannel1.pin.isPinLow()) || (vvtChannel1.periodTicks == true)) && ((vvtChannel2.pin.isPinLow()) || (vvtChannel2.periodTicks == true)) )
  {
    if( (vvtChannel1.targetDuty > 0) && (vvtChannel1.periodTicks == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      vvtChannel1.pin.setPinLow();
      #else
      vvtChannel1.pin.setPinHigh();
      #endif
    }
    if( (vvtChannel2.targetDuty > 0) && (vvtChannel2.periodTicks == false) ) //Don't toggle if at 0%
    {
      #if defined(CORE_TEENSY41)
      vvtChannel2.pin.setPinLow();
      #else
      vvtChannel2.pin.setPinHigh();
      #endif
    }

    if( (vvtChannel1.pin.isPinHigh()) && ((vvtChannel1.targetDuty <= vvtChannel2.targetDuty) || (vvtChannel2.pin.isPinLow())) )
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvtChannel1.targetDuty);
      vvtChannel1.compareTicks = vvtChannel1.targetDuty;
      vvtChannel2.compareTicks = vvtChannel2.targetDuty;
      if (vvtChannel1.targetDuty == vvtChannel2.targetDuty) { nextVVT = 2; } //Next event is for both PWM
      else { nextVVT = 0; } //Next event is for PWM0
    }
    else if( vvtChannel2.pin.isPinHigh() )
    {
      SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvtChannel2.targetDuty);
      vvtChannel1.compareTicks = vvtChannel1.targetDuty;
      vvtChannel2.compareTicks = vvtChannel2.targetDuty;
      nextVVT = 1; //Next event is for PWM1
    }
    else { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvt_pwm_max_count); } //Shouldn't ever get here
  }
  else
  {
    if(nextVVT == 0)
    {
      if(vvtChannel1.targetDuty < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvtChannel1.pin.setPinHigh();
        #else
        vvtChannel1.pin.setPinLow();
        #endif
        vvtChannel1.periodTicks = false;
      }
      else { vvtChannel1.periodTicks = true; }
      nextVVT = 1; //Next event is for PWM1
      if(vvtChannel2.pin.isPinHigh()){ SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvtChannel2.compareTicks - vvtChannel1.compareTicks) ); }
      else
      { 
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvtChannel1.compareTicks) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else if (nextVVT == 1)
    {
      if(vvtChannel2.targetDuty < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvtChannel2.pin.setPinHigh();
        #else
        vvtChannel2.pin.setPinLow();
        #endif
        vvtChannel2.periodTicks = false;
      }
      else { vvtChannel2.periodTicks = true; }
      nextVVT = 0; //Next event is for PWM0
      if(vvtChannel1.pin.isPinHigh()) { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvtChannel1.compareTicks - vvtChannel2.compareTicks) ); }
      else
      { 
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvtChannel2.compareTicks) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else
    {
      if(vvtChannel1.targetDuty < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
       #if defined(CORE_TEENSY41)
        vvtChannel1.pin.setPinHigh();
        #else
        vvtChannel1.pin.setPinLow();
        #endif
        vvtChannel1.periodTicks = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvtChannel1.compareTicks) );
      }
      else { vvtChannel1.periodTicks = true; }
      if(vvtChannel2.targetDuty < (long)vvt_pwm_max_count) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvtChannel2.pin.setPinHigh();
        #else
        vvtChannel2.pin.setPinLow();
        #endif
        vvtChannel2.periodTicks = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvt_pwm_max_count - vvtChannel2.compareTicks) );
      }
      else { vvtChannel2.periodTicks = true; }
    }
  }
}