/** @file
 * Global defines, macros, struct definitions (@ref statuses, @ref config2, @ref config4, config*), extern-definitions (for globally accessible vars).
 * 
 * ### Note on configuration struct layouts
 * 
 * Once the struct members have been assigned to certain "role" (in certain SW version), they should not be "moved around"
 * as the structs are stored onto EEPROM as-is and the offset and size of member needs to remain constant. Also removing existing struct members
 * would disturb layouts. Because of this a certain amount unused old members will be left into the structs. For the storage related reasons also the
 * bit fields are defined in byte-size (or multiple of ...) chunks.
 * 
 * ### Config Structs and 2D, 3D Tables
 * 
 * The config* structures contain information coming from tuning SW (e.g. TS) for 2D and 3D tables, where looked up value is not a result of direct
 * array lookup, but from interpolation algorithm. Because of standard, reusable interpolation routines associated with structs table2D and table3D,
 * the values from config are copied from config* structs to table2D (table3D destined configurations are not stored in config* structures).
 * 
 * ### Board choice
 * There's a C-preprocessor based "#if defined" logic present in this header file based on the Arduino IDE compiler set CPU
 * (+board?) type, e.g. `__AVR_ATmega2560__`. This respectively drives (withi it's "#if defined ..." block):
 * - The setting of various BOARD_* C-preprocessor variables (e.g. BOARD_MAX_ADC_PINS)
 * - Setting of BOARD_H (Board header) file (e.g. "board_avr2560.h"), which is later used to include the header file
 *   - Seems Arduino ide implicitly compiles and links respective .ino file (by it's internal build/compilation rules) (?)
 * - Setting of CPU (?) CORE_* variables (e.g. CORE_AVR), that is used across codebase to distinguish CPU.
 */
#ifndef GLOBALS_H
#define GLOBALS_H
#include "table2d.h"
#include "table3d.h"
#include "statuses.h"
#include "config_pages.h"
#include "atomic.h"
#include "src/pins/pinNumbers_t.h"

#define CRANK_ANGLE_MAX (max(CRANK_ANGLE_MAX_IGN, CRANK_ANGLE_MAX_INJ))

#ifndef UNIT_TEST 
constexpr uint8_t TOOTH_LOG_SIZE = 127U;
#else
constexpr uint8_t TOOTH_LOG_SIZE = 1U;
#endif

#define INJ1_CMD_BIT      0
#define INJ2_CMD_BIT      1
#define INJ3_CMD_BIT      2
#define INJ4_CMD_BIT      3
#define INJ5_CMD_BIT      4
#define INJ6_CMD_BIT      5
#define INJ7_CMD_BIT      6
#define INJ8_CMD_BIT      7

#define IGN1_CMD_BIT      0
#define IGN2_CMD_BIT      1
#define IGN3_CMD_BIT      2
#define IGN4_CMD_BIT      3
#define IGN5_CMD_BIT      4
#define IGN6_CMD_BIT      5
#define IGN7_CMD_BIT      6
#define IGN8_CMD_BIT      7

extern struct table3d16RpmLoad fuelTable; //16x16 fuel map
extern struct table3d16RpmLoad fuelTable2; //16x16 fuel map
extern struct table3d16RpmLoad ignitionTable; //16x16 ignition map
extern struct table3d16RpmLoad ignitionTable2; //16x16 ignition map
extern struct table3d16RpmLoad afrTable; //16x16 afr target map
extern struct table3d8RpmLoad stagingTable; //8x8 fuel staging table
extern struct table3d8RpmLoad boostTable; //8x8 boost map
extern struct table3d8RpmLoad boostTableLookupDuty; //8x8 boost map
extern struct table3d8RpmLoad vvtTable; //8x8 vvt map
extern struct table3d8RpmLoad vvt2Table; //8x8 vvt map
extern struct table3d8RpmLoad wmiTable; //8x8 wmi map

using trimTable3d = table3d6RpmLoad; 

extern trimTable3d trim1Table; //6x6 Fuel trim 1 map
extern trimTable3d trim2Table; //6x6 Fuel trim 2 map
extern trimTable3d trim3Table; //6x6 Fuel trim 3 map
extern trimTable3d trim4Table; //6x6 Fuel trim 4 map
extern trimTable3d trim5Table; //6x6 Fuel trim 5 map
extern trimTable3d trim6Table; //6x6 Fuel trim 6 map
extern trimTable3d trim7Table; //6x6 Fuel trim 7 map
extern trimTable3d trim8Table; //6x6 Fuel trim 8 map

extern struct table3d4RpmLoad dwellTable; //4x4 Dwell map

extern byte fpPrimeTime; //The time (in seconds, based on currentStatus.secl) that the fuel pump started priming
extern uint8_t softLimitTime; //The time (in 0.1 seconds, based on seclx10) that the soft limiter started
extern volatile uint16_t mainLoopCount;
extern volatile unsigned long ms_counter; //A counter that increments once per ms
extern uint16_t fixedCrankingOverride;
extern volatile uint32_t toothHistory[TOOTH_LOG_SIZE];
extern volatile uint8_t compositeLogHistory[TOOTH_LOG_SIZE];
extern volatile unsigned int toothHistoryIndex;
extern unsigned long currentLoopTime; /**< The time (in uS) that the current mainloop started */
extern volatile uint16_t ignitionCount; /**< The count of ignition events that have taken place since the engine started */
extern int16_t CRANK_ANGLE_MAX_IGN;
extern int16_t CRANK_ANGLE_MAX_INJ;       ///< The number of crank degrees that the system track over. 360 for wasted / timed batch and 720 for sequential
extern volatile uint32_t runSecsX10;  /**< Counter of seconds since cranking commenced (similar to runSecs) but in increments of 0.1 seconds */
extern volatile uint32_t seclx10;     /**< Counter of seconds since powered commenced (similar to secl) but in increments of 0.1 seconds */
extern volatile byte HWTest_INJ;      /**< Each bit in this variable represents one of the injector channels and it's HW test status */
extern volatile byte HWTest_INJ_Pulsed; /**< Each bit in this variable represents one of the injector channels and it's 50% HW test status */
extern volatile byte HWTest_IGN;      /**< Each bit in this variable represents one of the ignition channels and it's HW test status */
extern volatile byte HWTest_IGN_Pulsed; /**< Each bit in this variable represents one of the ignition channels and it's 50% HW test status */

extern pinNumbers_t pinNumbers;

extern struct statuses currentStatus; //The global status object
extern struct config2 configPage2;
extern struct config4 configPage4;
extern struct config6 configPage6;
extern struct config9 configPage9;
extern struct config10 configPage10;
extern struct config13 configPage13;
extern struct config15 configPage15;

bool pinIsOutput(byte pin);
bool pinIsUsed(byte pin);

#endif // GLOBALS_H
