#include "fuelPumpController.h"
#include "../../../board_definition.h"
#include "../../pins/fastOutputPin.h"
#include "../../pins/outputPin.h"
#include "../../../globals.h"

static boardOutputPin_t pump_pin;
TESTABLE_STATIC uint8_t fpPrimeTime = 0; ///< The time (in seconds, based on @ref statuses.secl) that the fuel pump started priming

void fuelPumpOn(void)
{
  ATOMIC() { 
    pump_pin.setPinHigh();
    currentStatus.fuelPumpOn = true;
  }
}
void fuelPumpOff(void)
{
  ATOMIC() { 
    pump_pin.setPinLow();
    currentStatus.fuelPumpOn = false;
  }
}

void __attribute__((optimize("Os"))) startPumpPriming(statuses &current, const config2 &page2)
{
  if(page2.fpPrime!=0U)
  {
    fpPrimeTime = current.secl;
    fuelPumpOn();
  }
  else
  {
    fpPrimeTime = 0;
  }
  current.fpPrimed = page2.fpPrime==0U;
}

static inline bool primingTimeExpired(const statuses &current, const config2 &page2)
{
  return (current.secl>=fpPrimeTime) // Unlikely, but prevent unsigned overflow
      && ((current.secl - fpPrimeTime) >= page2.fpPrime);
}

void __attribute__((optimize("Os"))) stopPumpPriming(statuses &current, const config2 &page2)
{
  //Check whether fuel pump priming is complete
  if(current.fpPrimed == false)
  {
    if (primingTimeExpired(current, page2))
    {
      current.fpPrimed = true; //Mark the priming as being completed
      if(current.RPM == 0)
      {
        //If we reach here then the priming is complete, however only turn off the fuel pump if the engine isn't running
        fuelPumpOff();
      }
    }
  }
}

void __attribute__((optimize("Os"))) initialiseFuelPump(statuses &current, const config2 &page2, uint8_t pumpPin)
{
  pump_pin.setPin(pumpPin, OUTPUT);
  fuelPumpOff();  //Initialise program with the fuel pump in the off state

  startPumpPriming(current, page2);
}
