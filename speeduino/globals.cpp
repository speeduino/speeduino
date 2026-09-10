/** @file
 * Instantiation of various (table2D, table3D) tables, volatile (interrupt modified) variables, Injector (1...8) enablement flags, etc.
 */
#include "globals.h"
#include "preprocessor.h"

struct table3d16RpmLoad fuelTable; ///< 16x16 fuel map
struct table3d16RpmLoad fuelTable2; ///< 16x16 fuel map
struct table3d16RpmLoad ignitionTable; ///< 16x16 ignition map
struct table3d16RpmLoad ignitionTable2; ///< 16x16 ignition map
struct table3d16RpmLoad afrTable; ///< 16x16 afr target map
struct table3d8RpmLoad stagingTable; ///< 8x8 fuel staging table
struct table3d8RpmLoad boostTable; ///< 8x8 boost map
struct table3d8RpmLoad boostTableLookupDuty; ///< 8x8 boost map lookup table
struct table3d8RpmLoad vvtTable; ///< 8x8 vvt map
struct table3d8RpmLoad vvt2Table; ///< 8x8 vvt2 map
struct table3d8RpmLoad wmiTable; ///< 8x8 wmi map
struct table3d6RpmLoad trimTables[INJ_CHANNELS];
struct table3d4RpmLoad dwellTable; ///< 4x4 Dwell map

//These are variables used across multiple files
uint8_t softLimitTime = 0; //The time (in 0.1 seconds, based on seclx10) that the soft limiter started
volatile uint16_t mainLoopCount; //Main loop counter (incremented at each main loop rev., used for maintaining currentStatus.loopsPerSecond)
volatile uint32_t toothHistory[TOOTH_LOG_SIZE]; ///< Tooth trigger history - delta time (in uS) from last tooth (Indexed by @ref toothHistoryIndex)
volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE];
// Some code relies on tooth log containing less than UINT8_MAX items.
static_assert(_countof(toothHistory)<UINT8_MAX, "Check all uses of toothHistory/toothHistoryIndex etc.");
volatile unsigned int toothHistoryIndex = 0; ///< Current index to @ref toothHistory array
volatile uint16_t ignitionCount; /**< The count of ignition events that have taken place since the engine started */
///< The number of crank degrees that the system track over. Typically 720 divided by the number of squirts per cycle (Eg 360 for wasted 2 squirt and 720 for sequential single squirt)
volatile uint32_t runSecsX10;
volatile uint32_t seclx10;

pinNumbers_t pinNumbers;

struct statuses currentStatus; /**< The master global "live" status struct. Contains all values that are updated frequently and used across modules */
struct config2 configPage2;
struct config4 configPage4;
struct config6 configPage6;
struct config9 configPage9;
struct config10 configPage10;
struct config13 configPage13;
struct config15 configPage15;

//These function do checks on a pin to determine if it is already in use by another (higher importance) active function
bool pinIsOutput(byte pin, const pinNumbers_t &pins)
{
  bool used = false;
  bool isIdlePWM = isPwmIac(configPage6);
  bool isIdleStepper = isStepperIac(configPage6);
  used = used 
      //Injector?
      || pins.injectorPins.isPinUsed(pin)
      //Ignition?
      || pins.coilPins.isPinUsed(pin);
  //Functions?
  if ((pin == pins.pinFuelPump)
  || ((pin == pins.pinFan) && ((configPage2.fanEnable == 1) || (configPage2.fanEnable == 2)))
  || ((pin == pins.pinVVT_1) && (configPage6.vvtEnabled > 0))
  || ((pin == pins.pinVVT_2) && (configPage10.wmiEnabled > 0))
  || ((pin == pins.pinVVT_2) && (configPage10.vvt2Enabled > 0))
  || ((pin == pins.pinBoost) && (configPage6.boostEnabled == 1))
  || ((pin == pins.pinIdle1) && isIdlePWM)
  || ((pin == pins.pinIdle2) && isIdlePWM && (configPage6.iacChannels == 1))
  || ((pin == pins.pinStepperEnable) && isIdleStepper)
  || ((pin == pins.pinStepperStep) && isIdleStepper)
  || ((pin == pins.pinStepperDir) && isIdleStepper)
  || (pin == pins.pinTachOut)
  || ((pin == pins.pinAirConComp) && (configPage15.airConEnable > 0))
  || ((pin == pins.pinAirConFan) && (configPage15.airConEnable > 0) && (configPage15.airConFanEnabled > 0)) )
  {
    used = true;
  }
  //Forbidden or hardware reserved? (Defined at board_xyz.h file)
  if ( pinIsReserved(pin) ) { used = true; }

  return used;
}

static inline bool pinIsSensor(byte pin, const pinNumbers_t &pins, const config2 &page2)
{
  return ((pin == pins.pinCLT) || (pin == pins.pinIAT) || (pin == pins.pinMAP) || (pin == pins.pinTPS) || (pin == pins.pinO2) || (pin == pins.pinBat) || ((pin == pins.pinFlex) && (page2.flexEnabled != 0)));
}

bool pinIsUsed(byte pin, const pinNumbers_t &pins)
{
  bool used = false;

  //Analog input?
  if ( pinIsSensor(pin, pins, configPage2) )
  {
    used = true;
  }
  //Functions?
  if ( pinIsOutput(pin, pins) )
  {
    used = true;
  }

  return used;
}
