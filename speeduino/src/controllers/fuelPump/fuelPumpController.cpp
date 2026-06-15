#include "fuelPumpController.h"
#include "fuelPumpController_detail.h"

TESTABLE_STATIC fuelPumpController::detsil::pump_state_t pump_state;

void fuelPumpOn(void)
{
    if (!pump_state.isPumpOn)
    {
        pump_state.pump_pin.setPinHigh();
        pump_state.isPumpOn = true;
    }
}
void fuelPumpOff(void)
{
    if (pump_state.isPumpOn)
    {
        pump_state.pump_pin.setPinLow();
        pump_state.isPumpOn = false;
    }
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
  return (current.secl>=pump_state.fpPrimeTime) // Unlikely, but prevent unsigned overflow
      && ((current.secl - pump_state.fpPrimeTime) >= page2.fpPrime);
}

void __attribute__((optimize("Os"))) stopPumpPriming(const statuses &current, const config2 &page2)
{
  //Check whether fuel pump priming is complete
  if(!pump_state.isPrimingComplete && primingTimeExpired(current, page2))
  {
    pump_state.isPrimingComplete = true; //Mark the priming as being completed
    if(current.RPM == 0)
    {
      //If we reach here then the priming is complete, however only turn off the fuel pump if the engine isn't running
      fuelPumpOff();
    }
  }
}

void __attribute__((optimize("Os"))) initialiseFuelPump(const statuses &current, const config2 &page2, uint8_t pumpPin)
{
  pump_state = fuelPumpController::detsil::pump_state_t();
  pump_state.pump_pin.setPin(pumpPin, OUTPUT);
  pump_state.isPumpOn = true; // This forces fuelPumpOff() to run.
  fuelPumpOff();  //Initialise program with the fuel pump in the off state

  startPumpPriming(current, page2);
}
