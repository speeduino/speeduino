#include "fuelPumpController.h"
#include "fuelPumpController_detail.h"
#include "elapsed_time.h"

TESTABLE_STATIC fuelPumpController::detsil::pump_state_t pump_state;

static void fuelPumpOn(void)
{
  pump_state.pump_pin.setPinHigh();
}

static void fuelPumpOff(void)
{
  pump_state.pump_pin.setPinLow();
}

void __attribute__((optimize("Os"))) startPumpPriming(const statuses &current, const config2 &page2)
{
  if(page2.fpPrime!=0U)
  {
    pump_state.fpPrimeTime = current.secl;
    fuelPumpOn();
  }
  else
  {
    pump_state.fpPrimeTime = 0;
  }
  pump_state.isPrimingComplete = page2.fpPrime==0U;
}

static inline bool primingTimeExpired(const statuses &current, const config2 &page2)
{
  return hasIntervalElapsed(current.secl, pump_state.fpPrimeTime, page2.fpPrime);
}

void __attribute__((optimize("Os"))) initialiseFuelPump(const statuses &current, const config2 &page2, uint8_t pumpPin)
{
  pump_state = fuelPumpController::detsil::pump_state_t();
  pump_state.pump_pin.setPin(pumpPin, OUTPUT);
  fuelPumpOff();  //Initialise program with the fuel pump in the off state

  startPumpPriming(current, page2);
}

TESTABLE_STATIC void fuelPumpControlCore(const statuses &current, const config2 &page2)
{
  bool pumpOn = false;

  // Engine is rotating
  if (current.rotationStatus!=EngineRotationStatus::Stopped)
  {
    pumpOn = true;
    pump_state.offDelay = 2; //0.2 sec delay for debouncing in case of noise
  }
  else if(!pump_state.isPrimingComplete) // Engine not running and not primed
  {
    pump_state.isPrimingComplete = primingTimeExpired(current, page2);
    pumpOn = !pump_state.isPrimingComplete;
    pump_state.offDelay = 0;
  }
  else if(pump_state.offDelay == 0)
  { 
    pumpOn = false;  // not running and prime completed and off delay done, turn off pump.
  }
  else 
  { 
    pumpOn = true;
    --pump_state.offDelay; // count down off delay.
  }

  // Single place to align target fuel pump status with actual fuel pump state
  if (pumpOn) 
  { 
    fuelPumpOn(); 
  }
  else 
  { 
    fuelPumpOff(); 
  }
}

// LCOV_EXCL_START
void fuelPumpControl(const statuses &current, const config2 &page2)
{
  if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_10HZ))
  {
    fuelPumpControlCore(current, page2);
  }
}
// LCOV_EXCL_STOP