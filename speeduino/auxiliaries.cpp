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

enum class NextInterruptEvent : uint8_t
{
  None = 0,
  VVT1Off = 1 << 0,
  VVT1On = 1 << 1,
  VVT2Off = 1 << 2,
  VVT2On = 1 << 3,
  BothOff = VVT1Off | VVT2Off,
  BothOn = VVT1On | VVT2On,
};

inline NextInterruptEvent operator|(NextInterruptEvent lhs, NextInterruptEvent rhs) {
    return static_cast<NextInterruptEvent>(
        static_cast<uint8_t>(lhs) |
        static_cast<uint8_t>(rhs)
    );
}

// 3. Overload the bitwise AND operator for checking flags
inline NextInterruptEvent operator&(NextInterruptEvent lhs, NextInterruptEvent rhs) {
    return static_cast<NextInterruptEvent>(
        static_cast<uint8_t>(lhs) &
        static_cast<uint8_t>(rhs)
    );
}

TESTABLE_STATIC VvtOutputChannel vvtChannel1;
TESTABLE_STATIC VvtOutputChannel vvtChannel2;
TESTABLE_STATIC NextInterruptEvent nextVVT;
TESTABLE_STATIC uint32_t vvtWarmStartTime;
TESTABLE_STATIC inputPin_t wmiTankEmptyPin;

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

  wmiTankEmptyPin.setPin(pinNumbers.pinWMIEmpty);
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

static void setTimerState(void) noexcept
{
  if( vvtChannel1.isPartialDuty() || vvtChannel2.isPartialDuty() )
  {
    ENABLE_VVT_TIMER();
  }
  else
  {
    DISABLE_VVT_TIMER();
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
    // LCOV_EXCL_BR_START
    if( configPage4.TrigPattern == 9 ) { currentStatus.vvt1.angle = getCamAngle_Miata9905(); }
    // LCOV_EXCL_BR_STOP

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
    setTimerState();
  }
}

static bool isWmiTankEmpty(void)
{
  return configPage10.wmiEmptyEnabled
      && (configPage10.wmiEmptyPolarity!=wmiTankEmptyPin.isPinHigh())
      ;
}

static bool isWmiActive(const statuses &current, const config10 &page10)
{
  return (current.TPS >= page10.wmiTPS) 
      && (current.RPM >= RPM_COARSE.toUser(page10.wmiRPM))
      && (current.MAP >= MAP.toUser(page10.wmiMAP)) 
      && (current.IAT >= TEMPERATURE.toUser(page10.wmiIAT))
      ;
}

static uint8_t calculateWmiPw(const statuses &current, const config10 &page10)
{
    uint16_t wmiPw = 0;

    if (configPage10.wmiEnabled && !current.wmiTankEmpty && isWmiActive(current, page10))
    {
      switch(configPage10.wmiMode)
      {
      case WMI_MODE_SIMPLE:
        // Simple mode - Output is turned on when preset boost level is reached
        wmiPw = 200;
        break;
      case WMI_MODE_PROPORTIONAL:
        // Proportional Mode - Output PWM is proportionally controlled between two MAP values - MAP Value 1 = PWM:0% / MAP Value 2 = PWM:100%
        wmiPw = map(currentStatus.MAP, MAP.toUser(configPage10.wmiMAP), MAP.toUser(configPage10.wmiMAP2), 0, 200);
        break;
      case WMI_MODE_OPENLOOP:
        //  Mapped open loop - Output PWM follows 2D map value (RPM vs MAP) Cell value contains desired PWM% [range 0-100%]
        wmiPw = get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM);
        break;
      case WMI_MODE_CLOSEDLOOP:
        // Mapped closed loop - Output PWM follows injector duty cycle with 2D correction map applied (RPM vs MAP). 
        // Cell value contains correction value% [nom 100%]
        {
          uint16_t basePw = clamp((int32_t)fuelSchedule1.pw + configPage10.wmiOffset, (int32_t)0, (int32_t)UINT16_MAX);
          wmiPw = halfPercentage(get3DTableValue(&wmiTable, currentStatus.MAP, currentStatus.RPM), basePw);
        }
        break;
      default: // Unknown mode
        break;
      }
    }

    return (uint8_t)clamp(wmiPw, (uint16_t)0, (uint16_t)200);
}

// Water methanol injection control
void wmiControl(void)
{
  currentStatus.wmiTankEmpty = configPage10.wmiEnabled && isWmiTankEmpty();
  currentStatus.wmiPW = calculateWmiPw(currentStatus, configPage10);

  if (configPage10.wmiEnabled)
  {
    ATOMIC() {
      vvtChannel2.setTargetDutyFromDuty(currentStatus.wmiPW);
      digitalWrite(pinNumbers.pinWMIEnabled, vvtChannel2.isNoDuty() ? LOW : HIGH);
      setTimerState();
    }
  }
}

#if defined(UNIT_TEST)
uint16_t lastVvtComparatorOffset = 0;
#endif
static void setVvtTimerCompare(uint16_t offset)
{
#if defined(UNIT_TEST)
  lastVvtComparatorOffset = offset;
#endif
  // IRL a zero offset is bad.
  if (offset==0U)
  {
    offset = 1000;
  }
  SET_COMPARE(VVT_TIMER_COMPARE, VVT_TIMER_COUNTER + offset);
}

static bool isVvtOff(const VvtOutputChannel &channel)
{
  return channel.pin.isPinLow() || channel.isFullDuty();
}

static NextInterruptEvent overrideNextEvent(NextInterruptEvent next, const VvtOutputChannel &channel1, const VvtOutputChannel &channel2)
{
  if (!channel1.isPartialDuty() && !channel2.isPartialDuty())
  {
    return NextInterruptEvent::None;
  }
  if (isVvtOff(channel1) && isVvtOff(channel2))
  {
    return NextInterruptEvent::BothOn;
  }
  return next;
}

static void applyEventToChannel(VvtOutputChannel &channel, NextInterruptEvent event, NextInterruptEvent onEvent, NextInterruptEvent offEvent)
{
  if (channel.isPartialDuty())
  {
    if ((event & onEvent) == onEvent)
    {
      channel.pin.setPinHigh();
    }
    else if ((event & offEvent) == offEvent)
    {
      channel.pin.setPinLow();
    }
    else
    {
      // Do nothing, leave the channel in its current state
    }
  }
}

static NextInterruptEvent calculateNextInterrupt(NextInterruptEvent currentEvent, const VvtOutputChannel &channel1, const VvtOutputChannel &channel2, uint16_t &offset)
{
  if(currentEvent == NextInterruptEvent::BothOn)
  {
    if( (channel1.pin.isPinHigh()) && ((channel1.targetDuty <= channel2.targetDuty) || (channel2.pin.isPinLow())) )
    {
      offset = channel1.targetDuty;
      return (channel1.targetDuty == channel2.targetDuty) ? NextInterruptEvent::BothOff : NextInterruptEvent::VVT1Off;
    }
    else if( channel2.pin.isPinHigh() )
    {
      offset = channel2.targetDuty;
      return NextInterruptEvent::VVT2Off; //Next event is for PWM1
    }
    else 
    { 
      INTERNAL_TEST_ASSERT(false); // Shouldn't ever get here
    }
  }
  else if(currentEvent == NextInterruptEvent::VVT1Off)
  {
    if(channel2.pin.isPinHigh())
    { 
      offset = channel2.targetDuty - channel1.targetDuty; 
      return NextInterruptEvent::VVT2Off; //Next event is for PWM1
    }
    else
    { 
      offset = channel1.maxDuty - channel1.targetDuty;
      return NextInterruptEvent::BothOff; //Next event is for both PWM
    }
  }
  else if (currentEvent == NextInterruptEvent::VVT2Off)
  {
    if(channel1.pin.isPinHigh()) 
    { 
      offset = channel1.targetDuty - channel2.targetDuty; 
      return NextInterruptEvent::VVT1Off; //Next event is for PWM0
    }
    else
    { 
      offset = channel2.maxDuty - channel2.targetDuty;
      return NextInterruptEvent::BothOff; //Next event is for both PWM
    }
  }
  else if (currentEvent == NextInterruptEvent::BothOff)
  {
    if(!channel1.isFullDuty()) //Don't toggle if at 100%
    {
      offset = channel1.maxDuty - channel1.targetDuty;
    }
    if(!channel2.isFullDuty()) //Don't toggle if at 100%
    {
      offset = channel2.maxDuty - channel2.targetDuty;
    }
    return NextInterruptEvent::BothOn; //Next event is for both PWM
  }
  else
  {
      INTERNAL_TEST_ASSERT(currentEvent == NextInterruptEvent::None);
  }
  offset = 0;
  return NextInterruptEvent::None;
}

//The interrupt to control the VVT PWM
void vvtInterrupt(void)
{
  nextVVT = overrideNextEvent(nextVVT, vvtChannel1, vvtChannel2);

  applyEventToChannel(vvtChannel1, nextVVT, NextInterruptEvent::VVT1On, NextInterruptEvent::VVT1Off);
  applyEventToChannel(vvtChannel2, nextVVT, NextInterruptEvent::VVT2On, NextInterruptEvent::VVT2Off);

  uint16_t offset = 0;
  nextVVT = calculateNextInterrupt(nextVVT, vvtChannel1, vvtChannel2, offset);
  setVvtTimerCompare(offset);
}