#include "nitrousController.h"
#include "nitrousController_state.h"
#include "unit_testing.h"
#include "units.h"

TESTABLE_STATIC nitrous::detail::state_t _n2oState;

static_assert(NITROUS_BOTH==NITROUS_STAGE1+NITROUS_STAGE2, "Check nitrous stage flag values");

static __attribute__((optimize("Os"))) uint8_t getN2oArmPinPolarity(const config10 &page10)
{
  if(page10.n2o_pin_polarity == 1U) 
  { 
    return INPUT_PULLUP; 
  }
  return INPUT;
}

static inline bool isStage1Enabled(const config10 &page10)
{
  return isNitrousStage1(page10.n2o_enable);
}
static inline bool isStage2Enabled(const config10 &page10)
{
  return isNitrousStage2(page10.n2o_enable);
}

static __attribute__((optimize("Os"))) void initialiseN2oPins(const config10 &page10)
{
  if (isStage1Enabled(page10))
  {
    _n2oState.stage1Pin.setPin(page10.n2o_stage1_pin, OUTPUT);
    _n2oState.armingPin.setPin(page10.n2o_arming_pin, getN2oArmPinPolarity(page10));
  }
  if (isStage2Enabled(page10))
  {
    _n2oState.stage2Pin.setPin(page10.n2o_stage2_pin, OUTPUT);
    _n2oState.armingPin.setPin(page10.n2o_arming_pin, getN2oArmPinPolarity(page10));
  }
}

static inline bool isValidPin(uint8_t pinNum)
{
  return (pinNum!=NOT_A_PIN)
// LCOV_EXCL_BR_START
   && !pinIsReserved(pinNum);
// LCOV_EXCL_BR_STOP
}

void __attribute__((optimize("Os"))) initialiseNitrous(statuses &current, config10 &page10)
{
  _n2oState = nitrous::detail::state_t();

  // It's either stage 1 or both: stage 2 on it's own makes no sense
  if (page10.n2o_enable==NITROUS_STAGE2)
  {
    page10.n2o_enable = NITROUS_BOTH;
  }

  // This is a safety check that will be true if the board is uninitialised. This prevents hangs on a new
  // board that could otherwise try to write to an invalid pin port/mask (Without this a new Teensy 4.x hangs on startup)
  // The n2o_minTPS variable is capped at 100 by TS, so 255 indicates a new board.
  if ( (page10.n2o_minTPS == 255) 
    // Check the pins are not in use
    || (!isValidPin(page10.n2o_stage1_pin))
    || (!isValidPin(page10.n2o_arming_pin))
    || (isStage2Enabled(page10) && !isValidPin(page10.n2o_stage2_pin))
  )
  { 
    page10.n2o_enable = NITROUS_OFF; 
  }

  initialiseN2oPins(page10);

  current.nitrousStatus = NITROUS_OFF;
}

static inline bool isArmed(const statuses &current, const config10 &page10)
{
    bool isArmed = _n2oState.armingPin.isPinHigh()!=page10.n2o_pin_polarity; //If nitrous is active when pin is low, flip the reading (n2o_pin_polarity = 0 = active when High)

    //Perform the main checks to see if nitrous is ready
    return isArmed 
        && (current.coolant > temperatureRemoveOffset(page10.n2o_minCLT)) 
        && (current.TPS > page10.n2o_minTPS)
        && (current.O2 < page10.n2o_maxAFR) 
        && (current.MAP < MAP.toUser(page10.n2o_maxMAP))
        ;
}

static inline bool isStage1Active(const statuses &current, const config10 &page10)
{
    return (current.RPM > RPM_COARSE.toUser(page10.n2o_stage1_minRPM)) 
        && (current.RPM < RPM_COARSE.toUser(page10.n2o_stage1_maxRPM));
}

static inline bool isStage2Active(const statuses &current, const config10 &page10)
{
    return isStage2Enabled(page10) 
        && (current.RPM > RPM_COARSE.toUser(page10.n2o_stage2_minRPM)) 
        && (current.RPM < RPM_COARSE.toUser(page10.n2o_stage2_maxRPM));
}

static inline uint8_t calcStatus(const statuses &current, const config10 &page10)
{
  //The nitrous state is set to 0 and then the subsequent stages are added
  // OFF    = 0
  // STAGE1 = 1
  // STAGE2 = 2
  // BOTH   = 3 (ie STAGE1 + STAGE2 = BOTH)
  uint8_t status = NITROUS_OFF;

  if(_n2oState.armingPin.isValid() && isArmed(current, page10))
  {
    if(isStage1Active(current, page10))
    {
      status += NITROUS_STAGE1;
    }
    if(isStage2Active(current, page10))
    {
      status += NITROUS_STAGE2;
    }
  }
  return status;
}

static inline void setPinState(uint8_t status)
{
  if (isNitrousStage1(status))
  {
    _n2oState.stage1Pin.setPinHigh();
  }
  else
  {
    _n2oState.stage1Pin.setPinLow();
  }

  if (isNitrousStage2(status))
  {
    _n2oState.stage2Pin.setPinHigh();
  }
  else
  {
    _n2oState.stage2Pin.setPinLow();
  }
}

TESTABLE_STATIC void nitrousControlCore(statuses &current, const config10 &page10)
{
  current.nitrousStatus = calcStatus(current, page10);
  setPinState(current.nitrousStatus);
}

void nitrousControl(statuses &current, const config10 &page10)
{
  if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_4HZ))
  {
    nitrousControlCore(current, page10);  
  }
}