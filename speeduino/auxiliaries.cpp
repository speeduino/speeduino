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
TESTABLE_STATIC uint32_t vvtWarmStartTime;

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
  vvtChannel1 = VvtOutputChannel(pinNumbers.pinVVT_1, FREQUENCY.toUser(configPage6.vvtFreq));
  vvtChannel2 = VvtOutputChannel(pinNumbers.pinVVT_2, FREQUENCY.toUser(configPage6.vvtFreq));

  currentStatus.wmiTankEmpty = false;
  currentStatus.wmiPW = 0;
  currentStatus.vvt1 = vvtStatus_t();
  currentStatus.vvt2 = vvtStatus_t();
  vvtWarmStartTime = 0;

  configPage10.vvt2Enabled = configPage10.vvt2Enabled && configPage6.vvtEnabled && !configPage10.wmiEnabled;

  if (configPage6.vvtMode == VVT_MODE_CLOSED_LOOP)
  {
    if (configPage6.vvtEnabled)
    {
      initialiseVvtPid(vvtPID, configPage10, configPage6.vvtPWMdir, currentStatus.vvt1.angle);
    }
    if (configPage10.vvt2Enabled) // same for VVT2 if it's enabled
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

static uint16_t getVvtLoad(const statuses &current, const config6 &page6)
{
  if (page6.vvtLoadSource == VVT_LOAD_TPS)
  {
    return (current.TPS * 2U);
  }
   return current.MAP;
}

static void updateVvtDutyOl(vvtStatus_t &vvtStatus, const statuses &current, const config6 &page6, const table3d8RpmLoad &lookupTable)
{
  // Lookup VVT duty based on either MAP or TPS
  vvtStatus.duty = get3DTableValue(&lookupTable, getVvtLoad(current, page6), current.RPM);
}

static void updateVvtDutyOnOff(vvtStatus_t &vvtStatus, const statuses &current, const config6 &page6, const table3d8RpmLoad &lookupTable)
{
  // Lookup VVT duty based on either MAP or TPS
  vvtStatus.duty = get3DTableValue(&lookupTable, getVvtLoad(current, page6), current.RPM);

  //VVT table can be used for controlling on/off switching. If this is turned on, then disregard any interpolation or non-binary values
  if(vvtStatus.duty < 200U) { 
    vvtStatus.duty = 0; 
  }
}

static void updateVvtDutyCl(vvtStatus_t &vvtStatus, integerPID &pid, const statuses &current, const config6 &page6, const config10 &page10, const table3d8RpmLoad &lookupTable)
{
  // Lookup VVT *angle* based on either MAP or TPS
  vvtStatus.targetAngle = get3DTableValue(&lookupTable, getVvtLoad(current, page6), current.RPM);

  // LCOV_EXCL_START
  if(BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1HZ)) { //This only needs to be run very infrequently, once per second
    setVvtPidTunings(pid, page10, page6.vvtPWMdir);  
  }
  // LCOV_EXCL_STOP

  // Safety check that the cam angles are ok. 
  // The engine will be totally undriveable if the cam sensor is faulty and giving wrong cam 
  // angles, so if that happens default to 0 duty.
  if ( vvtStatus.angle <=  page10.vvtCLMinAng || vvtStatus.angle > page10.vvtCLMaxAng )
  {
    vvtStatus.duty = 0;
    vvtStatus.angleError = true;
  }
  // Check that we're not already at the angle we want to be
  else if((page6.vvtCLUseHold) && (vvtStatus.targetAngle == vvtStatus.angle) )
  {
    vvtStatus.duty = page10.vvtCLholdDuty;
    pid.reset(vvtStatus.angle);
    vvtStatus.angleError = false;
  }
  else // If not already at target angle, calculate new value from PID
  {
    int32_t pidOutput = 0;
    pid.setSetPoint(vvtStatus.targetAngle);
    if(pid.compute(millis(), vvtStatus.angle, &pidOutput)) 
    { 
      vvtStatus.duty = (uint8_t)pidOutput;
    }
    vvtStatus.angleError = false;
  }
}

static void updateVvtDuty(vvtStatus_t &vvtStatus, integerPID &pid, const statuses &current, const config6 &page6, const config10 &page10, const table3d8RpmLoad &lookupTable)
{
  if(page6.vvtMode == VVT_MODE_OPEN_LOOP)
  {
    updateVvtDutyOl(vvtStatus, current, page6, lookupTable);
  }
  else if(page6.vvtMode == VVT_MODE_CLOSED_LOOP)
  {
    updateVvtDutyCl(vvtStatus, pid, current, page6, page10, lookupTable);
  }
  else
  {
    updateVvtDutyOnOff(vvtStatus, current, page6, lookupTable);
  }
}

static void setTimerState(const config10 &page10)
{
  if( !page10.wmiEnabled )
  {
    if( vvtChannel1.isOff() && vvtChannel2.isOff() )
    {
      DISABLE_VVT_TIMER();
    }
    else if( vvtChannel1.isOnFull() && vvtChannel2.isOnFull() )
    {
      DISABLE_VVT_TIMER();
    }
    else
    {
      ENABLE_VVT_TIMER();
    }
  }
  else
  {
    if (vvtChannel1.isOnPartial())
    {
      //Duty cycle is between 0 and 100. Make sure the timer is enabled
      ENABLE_VVT_TIMER();
    }
  }
}

static bool isVvtActive(const statuses &current, const config4 &page4, const config6 &page6)
{
  return (page6.vvtEnabled)
  && (current.coolant >= temperatureRemoveOffset(page4.vvtMinClt))
  && (current.rotationStatus==EngineRotationStatus::Running)
  ;
}

void vvtControl(void)
{
  if( isVvtActive(currentStatus, configPage4, configPage6) )
  {
    if(vvtWarmStartTime == 0U) 
    {
      vvtWarmStartTime = runSecsX10;
    }

    //Calculate the current cam angle for miata trigger
    if( configPage4.TrigPattern == 9 ) { currentStatus.vvt1.angle = getCamAngle_Miata9905(); }

    if(hasIntervalElapsed(runSecsX10, vvtWarmStartTime, TIME_TWO_MILLIS.toUser(configPage4.vvtDelay)) )
    {
      updateVvtDuty(currentStatus.vvt1, vvtPID, currentStatus, configPage6, configPage10, vvtTable);
      if (configPage10.vvt2Enabled) // same for VVT2 if it's enabled
      {
        updateVvtDuty(currentStatus.vvt2, vvt2PID, currentStatus, configPage6, configPage10, vvt2Table);
      }
    }
  }
  else 
  { 
    if (!configPage10.wmiEnabled)
    {
      currentStatus.vvt2.duty = 0;
    }
    currentStatus.vvt1.duty = 0;
    vvtWarmStartTime = 0;
  } 

  ATOMIC() {
    vvtChannel1.setTargetDutyFromDuty(currentStatus.vvt1.duty);
    if (configPage10.vvt2Enabled) // same for VVT2 if it's enabled
    {
      vvtChannel2.setTargetDutyFromDuty(currentStatus.vvt2.duty);
    }
    setTimerState(configPage10);
  }
}

static bool isWmiTankEmpty(void)
{
  if (configPage10.wmiEmptyEnabled) 
  {
    return (configPage10.wmiEmptyPolarity) ? digitalRead(pinNumbers.pinWMIEmpty) : !digitalRead(pinNumbers.pinWMIEmpty);
  }
  return true;
}

// Water methanol injection control
void wmiControl(void)
{
  if (configPage10.wmiEnabled)
  {
    currentStatus.wmiTankEmpty = !isWmiTankEmpty();

    uint16_t wmiPW = 0;
    if (!currentStatus.wmiTankEmpty)
    {
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
          wmiPW = (std::max)(0, ((int)fuelSchedule1.pw + configPage10.wmiOffset)) * get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM) / 200;
          break;
        default:
          // Wrong mode
          wmiPW = 0;
          break;
        }
        if (wmiPW > 200) { wmiPW = 200; } //without this the duty can get beyond 100%
      }
    }

    currentStatus.wmiPW = wmiPW;
    vvtChannel2.setTargetDutyFromDuty(currentStatus.wmiPW);
    if (vvtChannel2.isOnPartial())
    {
        ENABLE_VVT_TIMER();
    }
    else
    {
      if( !configPage6.vvtEnabled ) { DISABLE_VVT_TIMER(); }

    }
    digitalWrite(pinNumbers.pinWMIEnabled, vvtChannel2.isOff() ? LOW : HIGH);
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
    else { SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + vvtChannel1.maxDuty); } //Shouldn't ever get here
  }
  else
  {
    if(nextVVT == 0)
    {
      if(vvtChannel1.targetDuty < vvtChannel1.maxDuty) //Don't toggle if at 100%
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
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvtChannel1.maxDuty - vvtChannel1.compareTicks) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else if (nextVVT == 1)
    {
      if(vvtChannel2.targetDuty < vvtChannel2.maxDuty) //Don't toggle if at 100%
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
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvtChannel2.maxDuty - vvtChannel2.compareTicks) );
        nextVVT = 2; //Next event is for both PWM
      }
    }
    else
    {
      if(vvtChannel1.targetDuty < vvtChannel1.maxDuty) //Don't toggle if at 100%
      {
       #if defined(CORE_TEENSY41)
        vvtChannel1.pin.setPinHigh();
        #else
        vvtChannel1.pin.setPinLow();
        #endif
        vvtChannel1.periodTicks = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvtChannel1.maxDuty - vvtChannel1.compareTicks) );
      }
      else { vvtChannel1.periodTicks = true; }
      if(vvtChannel2.targetDuty < vvtChannel2.maxDuty) //Don't toggle if at 100%
      {
        #if defined(CORE_TEENSY41)
        vvtChannel2.pin.setPinHigh();
        #else
        vvtChannel2.pin.setPinLow();
        #endif
        vvtChannel2.periodTicks = false;
        SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + (vvtChannel2.maxDuty - vvtChannel2.compareTicks) );
      }
      else { vvtChannel2.periodTicks = true; }
    }
  }
}