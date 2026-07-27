#include "../../pins/inputPin.h"
#include "../../pins/outputPin.h"
#include "../../../unit_testing.h"
#include "../../../config_pages.h"
#include "../../../globals.h"
#include "../../../units.h"

TESTABLE_STATIC inputPin_t n2o_arming_pin;
TESTABLE_STATIC outputPin_t n2o_stage1_pin;
TESTABLE_STATIC outputPin_t n2o_stage2_pin;

static __attribute__((optimize("Os"))) uint8_t getN2oArmPinPolarity(const config10 &page10)
{
  if(page10.n2o_pin_polarity == 1U) 
  { 
    return INPUT_PULLUP; 
  }
  return INPUT;
}

static __attribute__((optimize("Os"))) void initialiseN2oArmPin(const config10 &page10)
{
  if(configPage10.n2o_enable!=0U && !pinIsReserved(page10.n2o_arming_pin))
  {
    // The pin modes are only set if the if n2o is enabled to prevent them conflicting 
    // with other inputs. 
    n2o_arming_pin.setPin(page10.n2o_arming_pin, getN2oArmPinPolarity(page10));
  }
}

static __attribute__((optimize("Os"))) void initialiseN2oPins(const config10 &page10)
{
  n2o_stage1_pin.setPin(page10.n2o_stage1_pin, OUTPUT);
  n2o_stage2_pin.setPin(page10.n2o_stage2_pin, OUTPUT);
  initialiseN2oArmPin(page10);
}

void __attribute__((optimize("Os"))) initialiseNitrous(void)
{
  initialiseN2oPins(configPage10);

  //This is a safety check that will be true if the board is uninitialised. This prevents hangs on a new board that could otherwise try to write to an invalid pin port/mask (Without this a new Teensy 4.x hangs on startup)
  //The n2o_minTPS variable is capped at 100 by TS, so 255 indicates a new board.
  if(configPage10.n2o_minTPS == 255) { configPage10.n2o_enable = 0; }

  currentStatus.nitrous_status = NITROUS_OFF;
}

void nitrousControl(void)
{
  currentStatus.nitrousActive = false;
  currentStatus.nitrous_status = NITROUS_OFF; //Reset the current state

  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = n2o_arming_pin.isPinHigh();
    if (configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; } //If nitrous is active when pin is low, flip the reading (n2o_pin_polarity = 0 = active when High)

    //Perform the main checks to see if nitrous is ready
    if( (isArmed == true) && (currentStatus.coolant > temperatureRemoveOffset(configPage10.n2o_minCLT)) && (currentStatus.TPS > configPage10.n2o_minTPS) && (currentStatus.O2 < configPage10.n2o_maxAFR) && (currentStatus.MAP < (uint16_t)(configPage10.n2o_maxMAP * 2U)) )
    {
      //Config page values are divided by 100 to fit within a byte. Multiply them back out to real values. 
      uint16_t realStage1MinRPM = (uint16_t)configPage10.n2o_stage1_minRPM * 100;
      uint16_t realStage1MaxRPM = (uint16_t)configPage10.n2o_stage1_maxRPM * 100;
      uint16_t realStage2MinRPM = (uint16_t)configPage10.n2o_stage2_minRPM * 100;
      uint16_t realStage2MaxRPM = (uint16_t)configPage10.n2o_stage2_maxRPM * 100;

      //The nitrous state is set to 0 and then the subsequent stages are added
      // OFF    = 0
      // STAGE1 = 1
      // STAGE2 = 2
      // BOTH   = 3 (ie STAGE1 + STAGE2 = BOTH)
      if( (currentStatus.RPM > realStage1MinRPM) && (currentStatus.RPM < realStage1MaxRPM) )
      {
        currentStatus.nitrous_status += NITROUS_STAGE1;
        currentStatus.nitrousActive = true;
        n2o_stage1_pin.setPinHigh();
      }
      if(configPage10.n2o_enable == NITROUS_STAGE2) //This is really just a sanity check
      {
        if( (currentStatus.RPM > realStage2MinRPM) && (currentStatus.RPM < realStage2MaxRPM) )
        {
          currentStatus.nitrous_status += NITROUS_STAGE2;
          currentStatus.nitrousActive = true;
          n2o_stage2_pin.setPinHigh();
        }
      }
    }
  }

  if (currentStatus.nitrousActive == false)
  {
    if(configPage10.n2o_enable > 0)
    {
      n2o_stage1_pin.setPinLow();
      n2o_stage2_pin.setPinLow();
    }
  }
}
