/** @file
 * Instantiation of various (table2D, table3D) tables, volatile (interrupt modified) variables, Injector (1...8) enablement flags, etc.
 */
#include "globals.h"

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
volatile unsigned long ms_counter = 0; //A counter that increments once per ms
uint16_t fixedCrankingOverride = 0;
volatile uint32_t toothHistory[TOOTH_LOG_SIZE]; ///< Tooth trigger history - delta time (in uS) from last tooth (Indexed by @ref toothHistoryIndex)
volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE];
// Some code relies on tooth log containing less than UINT8_MAX items.
static_assert(_countof(toothHistory)<UINT8_MAX, "Check all uses of toothHistory/toothHistoryIndex etc.");
volatile unsigned int toothHistoryIndex = 0; ///< Current index to @ref toothHistory array
unsigned long currentLoopTime; /**< The time (in uS) that the current mainloop started */
volatile uint16_t ignitionCount; /**< The count of ignition events that have taken place since the engine started */
int16_t CRANK_ANGLE_MAX_IGN = 360;
int16_t CRANK_ANGLE_MAX_INJ = 360; ///< The number of crank degrees that the system track over. Typically 720 divided by the number of squirts per cycle (Eg 360 for wasted 2 squirt and 720 for sequential single squirt)
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
bool pinIsOutput(byte pin)
{
  bool used = false;
  bool isIdlePWM = isPwmIac(configPage6);
  bool isIdleStepper = isStepperIac(configPage6);
  //Injector?
  for (uint8_t index=0; index<min((uint8_t)pinNumbers.injectorPins.size(), (uint8_t)configPage2.nInjectors); ++index)
  {
    used = used || (pin==pinNumbers.injectorPins[index]);
  }
  //Ignition?
  if ((pin == pinNumbers.pinCoil1)
  || ((pin == pinNumbers.pinCoil2) && (currentStatus.maxIgnOutputs > 1))
  || ((pin == pinNumbers.pinCoil3) && (currentStatus.maxIgnOutputs > 2))
  || ((pin == pinNumbers.pinCoil4) && (currentStatus.maxIgnOutputs > 3))
  || ((pin == pinNumbers.pinCoil5) && (currentStatus.maxIgnOutputs > 4))
  || ((pin == pinNumbers.pinCoil6) && (currentStatus.maxIgnOutputs > 5))
  || ((pin == pinNumbers.pinCoil7) && (currentStatus.maxIgnOutputs > 6))
  || ((pin == pinNumbers.pinCoil8) && (currentStatus.maxIgnOutputs > 7)))
  {
    used = true;
  }
  //Functions?
  if ((pin == pinNumbers.pinFuelPump)
  || ((pin == pinNumbers.pinFan) && (configPage2.fanEnable == 1))
  || ((pin == pinNumbers.pinVVT_1) && (configPage6.vvtEnabled > 0))
  || ((pin == pinNumbers.pinVVT_2) && (configPage10.wmiEnabled > 0))
  || ((pin == pinNumbers.pinVVT_2) && (configPage10.vvt2Enabled > 0))
  || ((pin == pinNumbers.pinBoost) && (configPage6.boostEnabled == 1))
  || ((pin == pinNumbers.pinIdle1) && isIdlePWM)
  || ((pin == pinNumbers.pinIdle2) && isIdlePWM && (configPage6.iacChannels == 1))
  || ((pin == pinNumbers.pinStepperEnable) && isIdleStepper)
  || ((pin == pinNumbers.pinStepperStep) && isIdleStepper)
  || ((pin == pinNumbers.pinStepperDir) && isIdleStepper)
  || (pin == pinNumbers.pinTachOut)
  || ((pin == pinNumbers.pinAirConComp) && (configPage15.airConEnable > 0))
  || ((pin == pinNumbers.pinAirConFan) && (configPage15.airConEnable > 0) && (configPage15.airConFanEnabled > 0)) )
  {
    used = true;
  }
  //Forbidden or hardware reserved? (Defined at board_xyz.h file)
  if ( pinIsReserved(pin) ) { used = true; }

  return used;
}

#define pinIsSensor(pin)    ( ((pin) == pinNumbers.pinCLT) || ((pin) == pinNumbers.pinIAT) || ((pin) == pinNumbers.pinMAP) || ((pin) == pinNumbers.pinTPS) || ((pin) == pinNumbers.pinO2) || ((pin) == pinNumbers.pinBat) || (((pin) == pinNumbers.pinFlex) && (configPage2.flexEnabled != 0)) )

bool pinIsUsed(byte pin)
{
  bool used = false;

  //Analog input?
  if ( pinIsSensor(pin) )
  {
    used = true;
  }
  //Functions?
  if ( pinIsOutput(pin) )
  {
    used = true;
  }

  return used;
}
