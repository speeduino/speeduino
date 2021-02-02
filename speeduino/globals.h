#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include "table.h"
#include <assert.h>
#include "logger.h"

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
  #define BOARD_MAX_DIGITAL_PINS 54 //digital pins +1
  #define BOARD_MAX_IO_PINS 70 //digital pins + analog channels + 1
#ifndef LED_BUILTIN
  #define LED_BUILTIN 13
#endif
  #define CORE_AVR
  #define BOARD_H "board_avr2560.h"
  #define INJ_CHANNELS 4
  #define IGN_CHANNELS 5

  #if defined(__AVR_ATmega2561__)
    //This is a workaround to avoid having to change all the references to higher ADC channels. We simply define the channels (Which don't exist on the 2561) as being the same as A0-A7
    //These Analog inputs should never be used on any 2561 board defintion (Because they don't exist on the MCU), so it will not cause any isses
    #define A8  A0
    #define A9  A1
    #define A10  A2
    #define A11  A3
    #define A12  A4
    #define A13  A5
    #define A14  A6
    #define A15  A7
  #endif

  //#define TIMER5_MICROS

#elif defined(CORE_TEENSY)
  #if defined(__MK64FX512__) || defined(__MK66FX1M0__)
    #define CORE_TEENSY35
    #define BOARD_H "board_teensy35.h"
    #define SD_LOGGING //SD logging enabled by default for Teensy 3.5 as it has the slot built in
  #elif defined(__IMXRT1062__)
    #define CORE_TEENSY41
    #define BOARD_H "board_teensy41.h"
  #endif
  #define INJ_CHANNELS 8
  #define IGN_CHANNELS 8

#elif defined(STM32_MCU_SERIES) || defined(ARDUINO_ARCH_STM32) || defined(STM32)
  #define CORE_STM32
  #if defined(STM32F407xx) //F407 can do 8x8 STM32F401/STM32F411 not
   #define INJ_CHANNELS 8
   #define IGN_CHANNELS 8
  #else
   #define INJ_CHANNELS 4
   #define IGN_CHANNELS 5
  #endif

//Select one for EEPROM,the default is EEPROM emulation on internal flash.
//#define SRAM_AS_EEPROM /*Use 4K battery backed SRAM, requires a 3V continuous source (like battery) connected to Vbat pin */
//#define USE_SPI_EEPROM PB0 /*Use M25Qxx SPI flash */
//#define FRAM_AS_EEPROM /*Use FRAM like FM25xxx, MB85RSxxx or any SPI compatible */

  #ifndef word
    #define word(h, l) ((h << 8) | l) //word() function not defined for this platform in the main library
  #endif
  
  
  #if defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_BLUEPILL_F103CB) \
   || defined(ARDUINO_BLACKPILL_F401CC) || defined(ARDUINO_BLACKPILL_F411CE)
    //STM32 Pill boards
    #ifndef NUM_DIGITAL_PINS
      #define NUM_DIGITAL_PINS 35
    #endif
    #ifndef LED_BUILTIN
      #define LED_BUILTIN PB1 //Maple Mini
    #endif
  #elif defined(STM32F407xx)
    #ifndef NUM_DIGITAL_PINS
      #define NUM_DIGITAL_PINS 75
    #endif
  #endif

  #if defined(STM32_CORE_VERSION)
    #define BOARD_H "board_stm32_official.h"
  #else
    #define CORE_STM32_GENERIC
    #define BOARD_H "board_stm32_generic.h"
  #endif

  //Specific mode for Bluepill due to its small flash size. This disables a number of strings from being compiled into the flash
  #if defined(MCU_STM32F103C8) || defined(MCU_STM32F103CB)
    #define SMALL_FLASH_MODE
  #endif

  #define BOARD_MAX_DIGITAL_PINS NUM_DIGITAL_PINS
  #define BOARD_MAX_IO_PINS NUM_DIGITAL_PINS
  #if __GNUC__ < 7 //Already included on GCC 7
  extern "C" char* sbrk(int16_t incr); //Used to freeRam
  #endif
  #ifndef digitalPinToInterrupt
  inline uint32_t  digitalPinToInterrupt(uint32_t Interrupt_pin) { return Interrupt_pin; } //This isn't included in the stm32duino libs (yet)
  #endif
#elif defined(__SAMD21G18A__)
  #define BOARD_H "board_samd21.h"
  #define CORE_SAMD21
#else
  #error Incorrect board selected. Please select the correct board (Usually Mega 2560) and upload again
#endif

//This can only be included after the above section
#include BOARD_H //Note that this is not a real file, it is defined in globals.h. 

//Handy bitsetting macros
#define BIT_SET(a,b) ((a) |= (1U<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1U<<(b)))
#define BIT_CHECK(var,pos) !!((var) & (1U<<(pos)))
#define BIT_TOGGLE(var,pos) ((var)^= 1UL << (pos))

#define interruptSafe(c) (noInterrupts(); {c} interrupts();) //Wraps any code between nointerrupt and interrupt calls

#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000

//Define the load algorithm
#define LOAD_SOURCE_MAP         0
#define LOAD_SOURCE_TPS         1
#define LOAD_SOURCE_IMAPEMAP    2

//Define bit positions within engine virable
#define BIT_ENGINE_RUN      0   // Engine running
#define BIT_ENGINE_CRANK    1   // Engine cranking
#define BIT_ENGINE_ASE      2   // after start enrichment (ASE)
#define BIT_ENGINE_WARMUP   3   // Engine in warmup
#define BIT_ENGINE_ACC      4   // in acceleration mode (TPS accel)
#define BIT_ENGINE_DCC      5   // in deceleration mode
#define BIT_ENGINE_MAPACC   6   // MAP acceleration mode
#define BIT_ENGINE_MAPDCC   7   // MAP decelleration mode

//Define masks for Status1
#define BIT_STATUS1_INJ1           0  //inj1
#define BIT_STATUS1_INJ2           1  //inj2
#define BIT_STATUS1_INJ3           2  //inj3
#define BIT_STATUS1_INJ4           3  //inj4
#define BIT_STATUS1_DFCO           4  //Decelleration fuel cutoff
#define BIT_STATUS1_BOOSTCUT       5  //Fuel component of MAP based boost cut out
#define BIT_STATUS1_TOOTHLOG1READY 6  //Used to flag if tooth log 1 is ready
#define BIT_STATUS1_TOOTHLOG2READY 7  //Used to flag if tooth log 2 is ready (Log is not currently used)

//Define masks for spark variable
#define BIT_SPARK_HLAUNCH         0  //Hard Launch indicator
#define BIT_SPARK_SLAUNCH         1  //Soft Launch indicator
#define BIT_SPARK_HRDLIM          2  //Hard limiter indicator
#define BIT_SPARK_SFTLIM          3  //Soft limiter indicator
#define BIT_SPARK_BOOSTCUT        4  //Spark component of MAP based boost cut out
#define BIT_SPARK_ERROR           5  // Error is detected
#define BIT_SPARK_IDLE            6  // idle on
#define BIT_SPARK_SYNC            7  // Whether engine has sync or not

#define BIT_SPARK2_FLATSH         0  //Flat shift hard cut
#define BIT_SPARK2_FLATSS         1  //Flat shift soft cut
#define BIT_SPARK2_SPARK2_ACTIVE  2
#define BIT_SPARK2_UNUSED4        3
#define BIT_SPARK2_UNUSED5        4
#define BIT_SPARK2_UNUSED6        5
#define BIT_SPARK2_UNUSED7        6
#define BIT_SPARK2_UNUSED8        7

#define BIT_TIMER_1HZ             0
#define BIT_TIMER_4HZ             1
#define BIT_TIMER_10HZ            2
#define BIT_TIMER_15HZ            3
#define BIT_TIMER_30HZ            4

#define BIT_STATUS3_RESET_PREVENT 0 //Indicates whether reset prevention is enabled
#define BIT_STATUS3_NITROUS       1
#define BIT_STATUS3_FUEL2_ACTIVE  2
#define BIT_STATUS3_VSS_REFRESH   3
#define BIT_STATUS3_HALFSYNC      4 //shows if there is only sync from primary trigger, but not from secondary.
#define BIT_STATUS3_NSQUIRTS1     5
#define BIT_STATUS3_NSQUIRTS2     6
#define BIT_STATUS3_NSQUIRTS3     7

#define VALID_MAP_MAX 1022 //The largest ADC value that is valid for the MAP sensor
#define VALID_MAP_MIN 2 //The smallest ADC value that is valid for the MAP sensor

#ifndef UNIT_TEST 
#define TOOTH_LOG_SIZE      127
#define TOOTH_LOG_BUFFER    128 //256
#else
#define TOOTH_LOG_SIZE      1
#define TOOTH_LOG_BUFFER    1 //256
#endif

#define COMPOSITE_LOG_PRI   0
#define COMPOSITE_LOG_SEC   1
#define COMPOSITE_LOG_TRIG  2
#define COMPOSITE_LOG_SYNC  3

#define INJ_PAIRED          0
#define INJ_SEMISEQUENTIAL  1
#define INJ_BANKED          2
#define INJ_SEQUENTIAL      3

#define OUTPUT_CONTROL_DIRECT   0
#define OUTPUT_CONTROL_MC33810  10

#define IGN_MODE_WASTED     0
#define IGN_MODE_SINGLE     1
#define IGN_MODE_WASTEDCOP  2
#define IGN_MODE_SEQUENTIAL 3
#define IGN_MODE_ROTARY     4

#define SEC_TRIGGER_SINGLE  0
#define SEC_TRIGGER_4_1     1

#define ROTARY_IGN_FC       0
#define ROTARY_IGN_FD       1
#define ROTARY_IGN_RX8      2

#define BOOST_MODE_SIMPLE   0
#define BOOST_MODE_FULL     1

#define WMI_MODE_SIMPLE       0
#define WMI_MODE_PROPORTIONAL 1
#define WMI_MODE_OPENLOOP     2
#define WMI_MODE_CLOSEDLOOP   3

#define HARD_CUT_FULL       0
#define HARD_CUT_ROLLING    1

#define SIZE_BYTE           8
#define SIZE_INT            16

#define EVEN_FIRE           0
#define ODD_FIRE            1

#define EGO_ALGORITHM_SIMPLE  0
#define EGO_ALGORITHM_PID     2

#define STAGING_MODE_TABLE  0
#define STAGING_MODE_AUTO   1

#define NITROUS_OFF         0
#define NITROUS_STAGE1      1
#define NITROUS_STAGE2      2
#define NITROUS_BOTH        3

#define PROTECT_CUT_OFF     0
#define PROTECT_CUT_IGN     1
#define PROTECT_CUT_FUEL    2
#define PROTECT_CUT_BOTH    3
#define PROTECT_IO_ERROR    7

#define AE_MODE_TPS         0
#define AE_MODE_MAP         1

#define AE_MODE_MULTIPLIER  0
#define AE_MODE_ADDER       1

#define KNOCK_MODE_OFF      0
#define KNOCK_MODE_DIGITAL  1
#define KNOCK_MODE_ANALOG   2

#define FUEL2_MODE_OFF      0
#define FUEL2_MODE_MULTIPLY 1
#define FUEL2_MODE_ADD      2
#define FUEL2_MODE_CONDITIONAL_SWITCH   3
#define FUEL2_MODE_INPUT_SWITCH 4

#define SPARK2_MODE_OFF      0
#define SPARK2_MODE_MULTIPLY 1
#define SPARK2_MODE_ADD      2
#define SPARK2_MODE_CONDITIONAL_SWITCH   3
#define SPARK2_MODE_INPUT_SWITCH 4

#define FUEL2_CONDITION_RPM 0
#define FUEL2_CONDITION_MAP 1
#define FUEL2_CONDITION_TPS 2
#define FUEL2_CONDITION_ETH 3

#define SPARK2_CONDITION_RPM 0
#define SPARK2_CONDITION_MAP 1
#define SPARK2_CONDITION_TPS 2
#define SPARK2_CONDITION_ETH 3

#define RESET_CONTROL_DISABLED             0
#define RESET_CONTROL_PREVENT_WHEN_RUNNING 1
#define RESET_CONTROL_PREVENT_ALWAYS       2
#define RESET_CONTROL_SERIAL_COMMAND       3

#define OPEN_LOOP_BOOST     0
#define CLOSED_LOOP_BOOST   1

#define SOFT_LIMIT_FIXED        0
#define SOFT_LIMIT_RELATIVE     1

#define VVT_MODE_ONOFF      0
#define VVT_MODE_OPEN_LOOP  1
#define VVT_MODE_CLOSED_LOOP 2
#define VVT_LOAD_MAP      0
#define VVT_LOAD_TPS      1

#define MULTIPLY_MAP_MODE_OFF   0
#define MULTIPLY_MAP_MODE_BARO  1
#define MULTIPLY_MAP_MODE_100   2

#define FOUR_STROKE         0
#define TWO_STROKE          1

#define GOING_LOW         0
#define GOING_HIGH        1

#define MAX_RPM 18000 //This is the maximum rpm that the ECU will attempt to run at. It is NOT related to the rev limiter, but is instead dictates how fast certain operations will be allowed to run. Lower number gives better performance

#define BATTV_COR_MODE_WHOLE 0
#define BATTV_COR_MODE_OPENTIME 1

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

#define ENGINE_PROTECT_BIT_RPM  0
#define ENGINE_PROTECT_BIT_MAP  1
#define ENGINE_PROTECT_BIT_OIL  2
#define ENGINE_PROTECT_BIT_AFR  3

//Table sizes
#define CALIBRATION_TABLE_SIZE 512
#define CALIBRATION_TEMPERATURE_OFFSET 40 // All temperature measurements are stored offset by 40 degrees. This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 215
#define OFFSET_FUELTRIM 127 //The fuel trim tables are offset by 128 to allow for -128 to +128 values
#define OFFSET_IGNITION 40 //Ignition values from the main spark table are offset 40 degrees downards to allow for negative spark timing

#define SERIAL_BUFFER_THRESHOLD 32 // When the serial buffer is filled to greater than this threshold value, the serial processing operations will be performed more urgently in order to avoid it overflowing. Serial buffer is 64 bytes long, so the threshold is set at half this as a reasonable figure

#ifndef CORE_TEENSY41
  #define FUEL_PUMP_ON() *pump_pin_port |= (pump_pin_mask)
  #define FUEL_PUMP_OFF() *pump_pin_port &= ~(pump_pin_mask)
#else
  //Special compatibility case for TEENSY 41 (for now)
  #define FUEL_PUMP_ON() digitalWrite(pinFuelPump, HIGH);
  #define FUEL_PUMP_OFF() digitalWrite(pinFuelPump, LOW);
#endif

extern const char TSfirmwareVersion[] PROGMEM;

extern const uint8_t data_structure_version; //This identifies the data structure when reading / writing.
#define NUM_PAGES     15
extern const uint16_t npage_size[NUM_PAGES]; /**< This array stores the size (in bytes) of each configuration page */
#define MAP_PAGE_SIZE 288

extern struct table3D fuelTable; //16x16 fuel map
extern struct table3D fuelTable2; //16x16 fuel map
extern struct table3D ignitionTable; //16x16 ignition map
extern struct table3D ignitionTable2; //16x16 ignition map
extern struct table3D afrTable; //16x16 afr target map
extern struct table3D stagingTable; //8x8 fuel staging table
extern struct table3D boostTable; //8x8 boost map
extern struct table3D vvtTable; //8x8 vvt map
extern struct table3D wmiTable; //8x8 wmi map
extern struct table3D trim1Table; //6x6 Fuel trim 1 map
extern struct table3D trim2Table; //6x6 Fuel trim 2 map
extern struct table3D trim3Table; //6x6 Fuel trim 3 map
extern struct table3D trim4Table; //6x6 Fuel trim 4 map
extern struct table2D taeTable; //4 bin TPS Acceleration Enrichment map (2D)
extern struct table2D maeTable;
extern struct table2D WUETable; //10 bin Warm Up Enrichment map (2D)
extern struct table2D ASETable; //4 bin After Start Enrichment map (2D)
extern struct table2D ASECountTable; //4 bin After Start duration map (2D)
extern struct table2D PrimingPulseTable; //4 bin Priming pulsewidth map (2D)
extern struct table2D crankingEnrichTable; //4 bin cranking Enrichment map (2D)
extern struct table2D dwellVCorrectionTable; //6 bin dwell voltage correction (2D)
extern struct table2D injectorVCorrectionTable; //6 bin injector voltage correction (2D)
extern struct table2D injectorAngleTable; //4 bin injector timing curve (2D)
extern struct table2D IATDensityCorrectionTable; //9 bin inlet air temperature density correction (2D)
extern struct table2D baroFuelTable; //8 bin baro correction curve (2D)
extern struct table2D IATRetardTable; //6 bin ignition adjustment based on inlet air temperature  (2D)
extern struct table2D idleTargetTable; //10 bin idle target table for idle timing (2D)
extern struct table2D idleAdvanceTable; //6 bin idle advance adjustment table based on RPM difference  (2D)
extern struct table2D CLTAdvanceTable; //6 bin ignition adjustment based on coolant temperature  (2D)
extern struct table2D rotarySplitTable; //8 bin ignition split curve for rotary leading/trailing  (2D)
extern struct table2D flexFuelTable;  //6 bin flex fuel correction table for fuel adjustments (2D)
extern struct table2D flexAdvTable;   //6 bin flex fuel correction table for timing advance (2D)
extern struct table2D flexBoostTable; //6 bin flex fuel correction table for boost adjustments (2D)
extern struct table2D fuelTempTable;  //6 bin fuel temperature correction table for fuel adjustments (2D)
extern struct table2D knockWindowStartTable;
extern struct table2D knockWindowDurationTable;
extern struct table2D oilPressureProtectTable;
extern struct table2D wmiAdvTable; //6 bin wmi correction table for timing advance (2D)

//These are for the direct port manipulation of the injectors, coils and aux outputs
extern volatile PORT_TYPE *inj1_pin_port;
extern volatile PINMASK_TYPE inj1_pin_mask;
extern volatile PORT_TYPE *inj2_pin_port;
extern volatile PINMASK_TYPE inj2_pin_mask;
extern volatile PORT_TYPE *inj3_pin_port;
extern volatile PINMASK_TYPE inj3_pin_mask;
extern volatile PORT_TYPE *inj4_pin_port;
extern volatile PINMASK_TYPE inj4_pin_mask;
extern volatile PORT_TYPE *inj5_pin_port;
extern volatile PINMASK_TYPE inj5_pin_mask;
extern volatile PORT_TYPE *inj6_pin_port;
extern volatile PINMASK_TYPE inj6_pin_mask;
extern volatile PORT_TYPE *inj7_pin_port;
extern volatile PINMASK_TYPE inj7_pin_mask;
extern volatile PORT_TYPE *inj8_pin_port;
extern volatile PINMASK_TYPE inj8_pin_mask;

extern volatile PORT_TYPE *ign1_pin_port;
extern volatile PINMASK_TYPE ign1_pin_mask;
extern volatile PORT_TYPE *ign2_pin_port;
extern volatile PINMASK_TYPE ign2_pin_mask;
extern volatile PORT_TYPE *ign3_pin_port;
extern volatile PINMASK_TYPE ign3_pin_mask;
extern volatile PORT_TYPE *ign4_pin_port;
extern volatile PINMASK_TYPE ign4_pin_mask;
extern volatile PORT_TYPE *ign5_pin_port;
extern volatile PINMASK_TYPE ign5_pin_mask;
extern volatile PORT_TYPE *ign6_pin_port;
extern volatile PINMASK_TYPE ign6_pin_mask;
extern volatile PORT_TYPE *ign7_pin_port;
extern volatile PINMASK_TYPE ign7_pin_mask;
extern volatile PORT_TYPE *ign8_pin_port;
extern volatile PINMASK_TYPE ign8_pin_mask;

extern volatile PORT_TYPE *tach_pin_port;
extern volatile PINMASK_TYPE tach_pin_mask;
extern volatile PORT_TYPE *pump_pin_port;
extern volatile PINMASK_TYPE pump_pin_mask;

extern volatile PORT_TYPE *flex_pin_port;
extern volatile PINMASK_TYPE flex_pin_mask;

extern volatile PORT_TYPE *triggerPri_pin_port;
extern volatile PINMASK_TYPE triggerPri_pin_mask;
extern volatile PORT_TYPE *triggerSec_pin_port;
extern volatile PINMASK_TYPE triggerSec_pin_mask;

//These need to be here as they are used in both speeduino.ino and scheduler.ino
extern bool channel1InjEnabled;
extern bool channel2InjEnabled;
extern bool channel3InjEnabled;
extern bool channel4InjEnabled;
extern bool channel5InjEnabled;
extern bool channel6InjEnabled;
extern bool channel7InjEnabled;
extern bool channel8InjEnabled;

extern int16_t ignition1EndAngle;
extern int16_t ignition2EndAngle;
extern int16_t ignition3EndAngle;
extern int16_t ignition4EndAngle;
extern int16_t ignition5EndAngle;
extern int16_t ignition6EndAngle;
extern int16_t ignition7EndAngle;
extern int16_t ignition8EndAngle;

extern int16_t ignition1StartAngle;
extern int16_t ignition2StartAngle;
extern int16_t ignition3StartAngle;
extern int16_t ignition4StartAngle;
extern int16_t ignition5StartAngle;
extern int16_t ignition6StartAngle;
extern int16_t ignition7StartAngle;
extern int16_t ignition8StartAngle;

//These are variables used across multiple files
extern const uint8_t PROGMEM fsIntIndex[31];
extern bool initialisationComplete; //Tracks whether the setup() function has run completely
extern uint8_t fpPrimeTime; //The time (in seconds, based on currentStatus.secl) that the fuel pump started priming
extern volatile uint16_t mainLoopCount;
extern unsigned long revolutionTime; //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
extern volatile unsigned long timer5_overflow_count; //Increments every time counter 5 overflows. Used for the fast version of micros()
extern volatile unsigned long ms_counter; //A counter that increments once per ms
extern uint16_t fixedCrankingOverride;
extern bool clutchTrigger;
extern bool previousClutchTrigger;
extern volatile uint32_t toothHistory[TOOTH_LOG_BUFFER];
extern volatile uint8_t compositeLogHistory[TOOTH_LOG_BUFFER];
extern volatile bool fpPrimed; //Tracks whether or not the fuel pump priming has been completed yet
extern volatile uint16_t toothHistoryIndex;
extern volatile uint8_t toothHistorySerialIndex;
extern unsigned long currentLoopTime; /**< The time (in uS) that the current mainloop started */
extern unsigned long previousLoopTime; /**< The time (in uS) that the previous mainloop started */
extern volatile uint16_t ignitionCount; /**< The count of ignition events that have taken place since the engine started */
extern uint8_t primaryTriggerEdge;
extern uint8_t secondaryTriggerEdge;
extern int16_t CRANK_ANGLE_MAX;
extern int16_t CRANK_ANGLE_MAX_IGN;
extern int16_t CRANK_ANGLE_MAX_INJ; //The number of crank degrees that the system track over. 360 for wasted / timed batch and 720 for sequential
extern volatile uint32_t runSecsX10; /**< Counter of seconds since cranking commenced (similar to runSecs) but in increments of 0.1 seconds */
extern volatile uint32_t seclx10; /**< Counter of seconds since powered commenced (similar to secl) but in increments of 0.1 seconds */
extern volatile uint8_t HWTest_INJ; /**< Each bit in this variable represents one of the injector channels and it's HW test status */
extern volatile uint8_t HWTest_INJ_50pc; /**< Each bit in this variable represents one of the injector channels and it's 50% HW test status */
extern volatile uint8_t HWTest_IGN; /**< Each bit in this variable represents one of the ignition channels and it's HW test status */
extern volatile uint8_t HWTest_IGN_50pc; /**< Each bit in this variable represents one of the ignition channels and it's 50% HW test status */

//This needs to be here because using the config page directly can prevent burning the setting
extern uint8_t resetControl;

extern volatile uint8_t TIMER_mask;
extern volatile uint8_t LOOP_TIMER;

//These functions all do checks on a pin to determine if it is already in use by another (higher importance) function
#define pinIsInjector(pin)  ( ((pin) == pinInjector1) || ((pin) == pinInjector2) || ((pin) == pinInjector3) || ((pin) == pinInjector4) || ((pin) == pinInjector5) || ((pin) == pinInjector6) || ((pin) == pinInjector7) || ((pin) == pinInjector8) )
#define pinIsIgnition(pin)  ( ((pin) == pinCoil1) || ((pin) == pinCoil2) || ((pin) == pinCoil3) || ((pin) == pinCoil4) || ((pin) == pinCoil5) || ((pin) == pinCoil6) || ((pin) == pinCoil7) || ((pin) == pinCoil8) )
#define pinIsSensor(pin)    ( ((pin) == pinCLT) || ((pin) == pinIAT) || ((pin) == pinMAP) || ((pin) == pinTPS) || ((pin) == pinO2) || ((pin) == pinBat) )
#define pinIsOutput(pin)    ( ((pin) == pinFuelPump) || ((pin) == pinFan) || ((pin) == pinVVT_1) || ((pin) == pinVVT_2) || ((pin) == pinBoost) || ((pin) == pinIdle1) || ((pin) == pinIdle2) || ((pin) == pinTachOut) )
#define pinIsUsed(pin)      ( pinIsInjector((pin)) || pinIsIgnition((pin)) || pinIsSensor((pin)) || pinIsOutput((pin)) || pinIsReserved((pin)) )

//The status struct contains the current values for all 'live' variables
//In current version this is 64 bytes
struct statuses {
  volatile bool hasSync;
  uint16_t RPM;
  uint8_t RPMdiv100;
  long longRPM;
  int16_t mapADC;
  int16_t baroADC;
  long MAP; //Has to be a long for PID calcs (Boost control)
  int16_t EMAP;
  int16_t EMAPADC;
  uint8_t baro; //Barometric pressure is simply the inital MAP reading, taken before the engine is running. Alternatively, can be taken from an external sensor
  uint8_t TPS; /**< The current TPS reading (0% - 100%). Is the tpsADC value after the calibration is applied */
  uint8_t tpsADC; /**< 0-255 byte representation of the TPS. Downsampled from the original 10-bit reading, but before any calibration is applied */
  uint8_t tpsDOT; /**< TPS delta over time. Measures the % per second that the TPS is changing. Value is divided by 10 to be stored in a byte */
  uint8_t mapDOT; /**< MAP delta over time. Measures the kpa per second that the MAP is changing. Value is divided by 10 to be stored in a byte */
  volatile int16_t rpmDOT;
  uint8_t VE; /**< The current VE value being used in the fuel calculation. Can be the same as VE1 or VE2, or a calculated value of both */
  uint8_t VE1; /**< The VE value from fuel table 1 */
  uint8_t VE2; /**< The VE value from fuel table 2, if in use (and required conditions are met) */
  uint8_t O2;
  uint8_t O2_2;
  int16_t coolant;
  int16_t cltADC;
  int16_t IAT;
  int16_t iatADC;
  int16_t batADC;
  int16_t O2ADC;
  int16_t O2_2ADC;
  int16_t dwell;
  uint8_t dwellCorrection; /**< The amount of correction being applied to the dwell time. */
  uint8_t battery10; /**< The current BRV in volts (multiplied by 10. Eg 12.5V = 125) */
  int8_t advance; /**< The current advance value being used in the spark calculation. Can be the same as advance1 or advance2, or a calculated value of both */
  int8_t advance1; /**< The advance value from ignition table 1 */
  int8_t advance2; /**< The advance value from ignition table 2 */
  uint16_t corrections; /**< The total current corrections % amount */
  uint16_t AEamount; /**< The amount of accleration enrichment currently being applied. 100=No change. Varies above 255 */
  uint8_t egoCorrection; /**< The amount of closed loop AFR enrichment currently being applied */
  uint8_t wueCorrection; /**< The amount of warmup enrichment currently being applied */
  uint8_t batCorrection; /**< The amount of battery voltage enrichment currently being applied */
  uint8_t iatCorrection; /**< The amount of inlet air temperature adjustment currently being applied */
  uint8_t baroCorrection; /**< The amount of correction being applied for the current baro reading */
  uint8_t launchCorrection; /**< The amount of correction being applied if launch control is active */
  uint8_t flexCorrection; /**< Amount of correction being applied to compensate for ethanol content */
  uint8_t fuelTempCorrection; /**< Amount of correction being applied to compensate for fuel temperature */
  int8_t flexIgnCorrection; /**< Amount of additional advance being applied based on flex. Note the type as this allows for negative values */
  uint8_t afrTarget;
  uint8_t idleDuty; /**< The current idle duty cycle amount if PWM idle is selected and active */
  uint8_t CLIdleTarget; /**< The target idle RPM (when closed loop idle control is active) */
  bool idleUpActive; /**< Whether the externally controlled idle up is currently active */
  bool CTPSActive; /**< Whether the externally controlled closed throttle position sensor is currently active */
  bool fanOn; /**< Whether or not the fan is turned on */
  volatile uint8_t ethanolPct; /**< Ethanol reading (if enabled). 0 = No ethanol, 100 = pure ethanol. Eg E85 = 85. */
  volatile int8_t fuelTemp;
  unsigned long AEEndTime; /**< The target end time used whenever AE is turned on */
  volatile uint8_t status1;
  volatile uint8_t spark;
  volatile uint8_t spark2;
  uint8_t engine;
  uint16_t PW1; //In uS
  uint16_t PW2; //In uS
  uint16_t PW3; //In uS
  uint16_t PW4; //In uS
  uint16_t PW5; //In uS
  uint16_t PW6; //In uS
  uint16_t PW7; //In uS
  uint16_t PW8; //In uS
  volatile uint8_t runSecs; /**< Counter of seconds since cranking commenced (Maxes out at 255 to prevent overflow) */
  volatile uint8_t secl; /**< Counter incrementing once per second. Will overflow after 255 and begin again. This is used by TunerStudio to maintain comms sync */
  volatile uint32_t loopsPerSecond; /**< A performance indicator showing the number of main loops that are being executed each second */ 
  bool launchingSoft; /**< Indicator showing whether soft launch control adjustments are active */
  bool launchingHard; /**< Indicator showing whether hard launch control adjustments are active */
  uint16_t freeRAM;
  uint16_t clutchEngagedRPM; /**< The RPM at which the clutch was last depressed. Used for distinguishing between launch control and flat shift */ 
  bool flatShiftingHard;
  volatile uint32_t startRevolutions; /**< A counter for how many revolutions have been completed since sync was achieved. */
  uint16_t boostTarget;
  uint8_t testOutputs;
  bool testActive;
  uint16_t boostDuty; //Percentage value * 100 to give 2 points of precision
  uint8_t idleLoad; /**< Either the current steps or current duty cycle for the idle control. */
  uint16_t canin[16];   //16bit raw value of selected canin data for channel 0-15
  uint8_t current_caninchannel = 0; /**< Current CAN channel, defaults to 0 */
  uint16_t crankRPM = 400; /**< The actual cranking RPM limit. This is derived from the value in the config page, but saves us multiplying it everytime it's used (Config page value is stored divided by 10) */
  volatile uint8_t status3;
  int16_t flexBoostCorrection; /**< Amount of boost added based on flex */
  uint8_t nitrous_status;
  uint8_t nSquirts;
  uint8_t nChannels; /**< Number of fuel and ignition channels.  */
  int16_t fuelLoad;
  int16_t fuelLoad2;
  int16_t ignLoad;
  bool fuelPumpOn; /**< Indicator showing the current status of the fuel pump */
  uint8_t syncLossCounter;
  uint8_t knockRetard;
  bool knockActive;
  bool toothLogEnabled;
  bool compositeLogEnabled;
  //int8_t vvt1Angle;
  long vvt1Angle;
  uint8_t vvt1TargetAngle;
  uint8_t vvt1Duty;
  uint16_t injAngle;
  uint8_t ASEValue;
  uint16_t vss; /**< Current speed reading. Natively stored in kph and converted to mph in TS if required */
  bool idleUpOutputActive; /**< Whether the idle up output is currently active */
  uint8_t gear; /**< Current gear (Calculated from vss) */
  uint8_t fuelPressure; /**< Fuel pressure in PSI */
  uint8_t oilPressure; /**< Oil pressure in PSI */
  uint8_t engineProtectStatus;
  uint8_t wmiPW;
  bool wmiEmpty;
  long vvt2Angle;
  uint8_t vvt2TargetAngle;
  uint8_t vvt2Duty;
  uint8_t outputsStatus;
  uint8_t TS_SD_Status; //TunerStudios SD card status
};

/**
 * @brief This mostly covers off variables that are required for fuel
 * 
 * See the ini file for further reference
 * 
 */
struct config2 {

  uint8_t aseTaperTime;
  uint8_t aeColdPct;  //AE cold clt modifier %
  uint8_t aeColdTaperMin; //AE cold modifier, taper start temp (full modifier), was ASE in early versions
  uint8_t aeMode : 2; /**< Acceleration Enrichment mode. 0 = TPS, 1 = MAP. Values 2 and 3 reserved for potential future use (ie blended TPS / MAP) */
  uint8_t battVCorMode : 1;
  uint8_t SoftLimitMode : 1;
  uint8_t useTachoSweep : 1;
  uint8_t aeApplyMode : 1; //0 = Multiply | 1 = Add
  uint8_t multiplyMAP : 2; //0 = off | 1 = baro | 2 = 100
  uint8_t wueValues[10]; //Warm up enrichment array (10 bytes)
  uint8_t crankingPct; //Cranking enrichment
  uint8_t pinMapping; // The board / ping mapping to be used
  uint8_t tachoPin : 6; //Custom pin setting for tacho output
  uint8_t tachoDiv : 2; //Whether to change the tacho speed
  uint8_t tachoDuration; //The duration of the tacho pulse in mS
  uint8_t maeThresh; /**< The MAPdot threshold that must be exceeded before AE is engaged */
  uint8_t taeThresh; /**< The TPSdot threshold that must be exceeded before AE is engaged */
  uint8_t aeTime;

  //Display config bits
  uint8_t displayType : 3; //21
  uint8_t display1 : 3;
  uint8_t display2 : 2;

  uint8_t display3 : 3;    //22
  uint8_t display4 : 2;
  uint8_t display5 : 3;

  uint8_t displayB1 : 4;   //23
  uint8_t displayB2 : 4;

  uint8_t reqFuel;       //24
  uint8_t divider;
  uint8_t injTiming : 1;
  uint8_t multiplyMAP_old : 1;
  uint8_t includeAFR : 1;
  uint8_t hardCutType : 1;
  uint8_t ignAlgorithm : 3;
  uint8_t indInjAng : 1;
  uint8_t injOpen; //Injector opening time (ms * 10)
  uint16_t injAng[4];

  //config1 in ini
  uint8_t mapSample : 2;
  uint8_t strokes : 1;
  uint8_t injType : 1;
  uint8_t nCylinders : 4; //Number of cylinders

  //config2 in ini
  uint8_t fuelAlgorithm : 3;
  uint8_t fixAngEnable : 1; //Whether fixed/locked timing is enabled
  uint8_t nInjectors : 4; //Number of injectors


  //config3 in ini
  uint8_t engineType : 1;
  uint8_t flexEnabled : 1;
  uint8_t legacyMAP  : 1;
  uint8_t baroCorr : 1;
  uint8_t injLayout : 2;
  uint8_t perToothIgn : 1;
  uint8_t dfcoEnabled : 1; //Whether or not DFCO is turned on

  uint8_t aeColdTaperMax;  //AE cold modifier, taper end temp (no modifier applied), was primePulse in early versions
  uint8_t dutyLim;
  uint8_t flexFreqLow; //Lowest valid frequency reading from the flex sensor
  uint8_t flexFreqHigh; //Highest valid frequency reading from the flex sensor

  uint8_t boostMaxDuty;
  uint8_t tpsMin;
  uint8_t tpsMax;
  int8_t mapMin; //Must be signed
  uint16_t mapMax;
  uint8_t fpPrime; //Time (In seconds) that the fuel pump should be primed for on power up
  uint8_t stoich;
  uint16_t oddfire2; //The ATDC angle of channel 2 for oddfire
  uint16_t oddfire3; //The ATDC angle of channel 3 for oddfire
  uint16_t oddfire4; //The ATDC angle of channel 4 for oddfire

  uint8_t idleUpPin : 6;
  uint8_t idleUpPolarity : 1;
  uint8_t idleUpEnabled : 1;

  uint8_t idleUpAdder;
  uint8_t aeTaperMin;
  uint8_t aeTaperMax;

  uint8_t iacCLminDuty;
  uint8_t iacCLmaxDuty;
  uint8_t boostMinDuty;

  int8_t baroMin; //Must be signed
  uint16_t baroMax;

  int8_t EMAPMin; //Must be signed
  uint16_t EMAPMax;

  uint8_t fanWhenOff : 1;      // Only run fan when engine is running
  uint8_t fanWhenCranking : 1;      //**< Setting whether the fan output will stay on when the engine is cranking */ 
  uint8_t fanUnused : 3;
  uint8_t rtc_mode : 2;
  uint8_t incorporateAFR : 1;  //Incorporate AFR
  uint8_t asePct[4];  //Afterstart enrichment (%)
  uint8_t aseCount[4]; //Afterstart enrichment cycles. This is the number of ignition cycles that the afterstart enrichment % lasts for
  uint8_t aseBins[4]; //Afterstart enrichment temp axis
  uint8_t primePulse[4]; //Priming pulsewidth
  uint8_t primeBins[4]; //Priming temp axis

  uint8_t CTPSPin : 6;
  uint8_t CTPSPolarity : 1;
  uint8_t CTPSEnabled : 1;

  uint8_t idleAdvEnabled : 2;
  uint8_t idleAdvAlgorithm : 1;
  uint8_t IdleAdvDelay : 5;
  
  uint8_t idleAdvRPM;
  uint8_t idleAdvTPS;

  uint8_t injAngRPM[4];

  uint8_t idleTaperTime;
  uint8_t dfcoDelay;
  uint8_t dfcoMinCLT;

  //VSS Stuff
  uint8_t vssMode : 2;
  uint8_t vssPin : 6;
  
  uint16_t vssPulsesPerKm;
  uint8_t vssSmoothing;
  uint16_t vssRatio1;
  uint16_t vssRatio2;
  uint16_t vssRatio3;
  uint16_t vssRatio4;
  uint16_t vssRatio5;
  uint16_t vssRatio6;

  uint8_t idleUpOutputEnabled : 1;
  uint8_t idleUpOutputInv : 1;
  uint8_t idleUpOutputPin  : 6;

  uint8_t tachoSweepMaxRPM;
  uint8_t primingDelay;

  uint8_t iacTPSlimit;
  uint8_t iacRPMlimitHysteresis;

  int8_t rtc_trim;

  uint8_t unused2_95[4];

#if defined(CORE_AVR)
  };
#else
  } __attribute__((__packed__)); //The 32 bi systems require all structs to be fully packed
#endif

//Page 4 of the config - See the ini file for further reference
//This mostly covers off variables that are required for ignition
struct config4 {

  int16_t triggerAngle;
  int8_t FixAng; //Negative values allowed
  uint8_t CrankAng;
  uint8_t TrigAngMul; //Multiplier for non evenly divisible tooth counts.

  uint8_t TrigEdge : 1;
  uint8_t TrigSpeed : 1;
  uint8_t IgInv : 1;
  uint8_t TrigPattern : 5;

  uint8_t TrigEdgeSec : 1;
  uint8_t fuelPumpPin : 6;
  uint8_t useResync : 1;

  uint8_t sparkDur; //Spark duration in ms * 10
  uint8_t trigPatternSec; //Mode for Missing tooth secondary trigger.  Either single tooth cam wheel or 4-1
  uint8_t bootloaderCaps; //Capabilities of the bootloader over stock. e.g., 0=Stock, 1=Reset protection, etc.

  uint8_t resetControlConfig : 2; //Which method of reset control to use (0=None, 1=Prevent When Running, 2=Prevent Always, 3=Serial Command)
  uint8_t resetControlPin : 6;

  uint8_t StgCycles; //The number of initial cycles before the ignition should fire when first cranking

  uint8_t boostType : 1; //Open or closed loop boost control
  uint8_t useDwellLim : 1; //Whether the dwell limiter is off or on
  uint8_t sparkMode : 3; //Spark output mode (Eg Wasted spark, single channel or Wasted COP)
  uint8_t triggerFilter : 2; //The mode of trigger filter being used (0=Off, 1=Light (Not currently used), 2=Normal, 3=Aggressive)
  uint8_t ignCranklock : 1; //Whether or not the ignition timing during cranking is locked to a CAS pulse. Only currently valid for Basic distributor and 4G63.

  uint8_t dwellCrank; //Dwell time whilst cranking
  uint8_t dwellRun; //Dwell time whilst running
  uint8_t triggerTeeth; //The full count of teeth on the trigger wheel if there were no gaps
  uint8_t triggerMissingTeeth; //The size of the tooth gap (ie number of missing teeth)
  uint8_t crankRPM; //RPM below which the engine is considered to be cranking
  uint8_t floodClear; //TPS value that triggers flood clear mode (No fuel whilst cranking)
  uint8_t SoftRevLim; //Soft rev limit (RPM/100)
  uint8_t SoftLimRetard; //Amount soft limit retards (degrees)
  uint8_t SoftLimMax; //Time the soft limit can run
  uint8_t HardRevLim; //Hard rev limit (RPM/100)
  uint8_t taeBins[4]; //TPS based acceleration enrichment bins (%/s)
  uint8_t taeValues[4]; //TPS based acceleration enrichment rates (% to add)
  uint8_t wueBins[10]; //Warmup Enrichment bins (Values are in configTable1)
  uint8_t dwellLimit;
  uint8_t dwellCorrectionValues[6]; //Correction table for dwell vs battery voltage
  uint8_t iatRetBins[6]; // Inlet Air Temp timing retard curve bins
  uint8_t iatRetValues[6]; // Inlet Air Temp timing retard curve values
  uint8_t dfcoRPM; //RPM at which DFCO turns off/on at
  uint8_t dfcoHyster; //Hysteris RPM for DFCO
  uint8_t dfcoTPSThresh; //TPS must be below this figure for DFCO to engage

  uint8_t ignBypassEnabled : 1; //Whether or not the ignition bypass is enabled
  uint8_t ignBypassPin : 6; //Pin the ignition bypass is activated on
  uint8_t ignBypassHiLo : 1; //Whether this should be active high or low.

  uint8_t ADCFILTER_TPS;
  uint8_t ADCFILTER_CLT;
  uint8_t ADCFILTER_IAT;
  uint8_t ADCFILTER_O2;
  uint8_t ADCFILTER_BAT;
  uint8_t ADCFILTER_MAP; //This is only used on Instantaneous MAP readings and is intentionally very weak to allow for faster response
  uint8_t ADCFILTER_BARO;
  
  uint8_t cltAdvBins[6]; /**< Coolant Temp timing advance curve bins */
  uint8_t cltAdvValues[6]; /**< Coolant timing advance curve values. These are translated by 15 to allow for negative values */

  uint8_t maeBins[4]; /**< MAP based AE MAPdot bins */
  uint8_t maeRates[4]; /**< MAP based AE values */

  int8_t batVoltCorrect; /**< Battery voltage calibration offset */

  uint8_t baroFuelBins[8];
  uint8_t baroFuelValues[8];

  uint8_t idleAdvBins[6];
  uint8_t idleAdvValues[6];

  uint8_t engineProtectMaxRPM;

  uint8_t unused4_120[7];

#if defined(CORE_AVR)
  };
#else
  } __attribute__((__packed__)); //The 32 bi systems require all structs to be fully packed
#endif

//Page 6 of the config - See the ini file for further reference
//This mostly covers off variables that are required for AFR targets and closed loop
struct config6 {

  uint8_t egoAlgorithm : 2;
  uint8_t egoType : 2;
  uint8_t boostEnabled : 1;
  uint8_t vvtEnabled : 1;
  uint8_t engineProtectType : 2;

  uint8_t egoKP;
  uint8_t egoKI;
  uint8_t egoKD;
  uint8_t egoTemp; //The temperature above which closed loop functions
  uint8_t egoCount; //The number of ignition cylces per step
  uint8_t vvtMode : 2; //Valid VVT modes are 'on/off', 'open loop' and 'closed loop'
  uint8_t vvtLoadSource : 2; //Load source for VVT (TPS or MAP)
  uint8_t vvtPWMdir : 1; //VVT direction (normal or reverse)
  uint8_t vvtCLUseHold : 1; //Whether or not to use a hold duty cycle (Most cases are Yes)
  uint8_t vvtCLAlterFuelTiming : 1;
  uint8_t boostCutEnabled : 1;
  uint8_t egoLimit; //Maximum amount the closed loop will vary the fueling
  uint8_t ego_min; //AFR must be above this for closed loop to function
  uint8_t ego_max; //AFR must be below this for closed loop to function
  uint8_t ego_sdelay; //Time in seconds after engine starts that closed loop becomes available
  uint8_t egoRPM; //RPM must be above this for closed loop to function
  uint8_t egoTPSMax; //TPS must be below this for closed loop to function
  uint8_t vvt1Pin : 6;
  uint8_t useExtBaro : 1;
  uint8_t boostMode : 1; //Simple of full boost control
  uint8_t boostPin : 6;
  uint8_t VVTasOnOff : 1; //Whether or not to use the VVT table as an on/off map
  uint8_t useEMAP : 1;
  uint8_t voltageCorrectionBins[6]; //X axis bins for voltage correction tables
  uint8_t injVoltageCorrectionValues[6]; //Correction table for injector PW vs battery voltage
  uint8_t airDenBins[9];
  uint8_t airDenRates[9];
  uint8_t boostFreq; //Frequency of the boost PWM valve
  uint8_t vvtFreq; //Frequency of the vvt PWM valve
  uint8_t idleFreq;

  uint8_t launchPin : 6;
  uint8_t launchEnabled : 1;
  uint8_t launchHiLo : 1;

  uint8_t lnchSoftLim;
  int8_t lnchRetard; //Allow for negative advance value (ATDC)
  uint8_t lnchHardLim;
  uint8_t lnchFuelAdd;

  //PID values for idle needed to go here as out of room in the idle page
  uint8_t idleKP;
  uint8_t idleKI;
  uint8_t idleKD;

  uint8_t boostLimit; //Is divided by 2, allowing kPa values up to 511
  uint8_t boostKP;
  uint8_t boostKI;
  uint8_t boostKD;

  uint8_t lnchPullRes : 2;
  uint8_t fuelTrimEnabled : 1;
  uint8_t flatSEnable : 1;
  uint8_t baroPin : 4;
  uint8_t flatSSoftWin;
  uint8_t flatSRetard;
  uint8_t flatSArm;

  uint8_t iacCLValues[10]; //Closed loop target RPM value
  uint8_t iacOLStepVal[10]; //Open loop step values for stepper motors
  uint8_t iacOLPWMVal[10]; //Open loop duty values for PMWM valves
  uint8_t iacBins[10]; //Temperature Bins for the above 3 curves
  uint8_t iacCrankSteps[4]; //Steps to use when cranking (Stepper motor)
  uint8_t iacCrankDuty[4]; //Duty cycle to use on PWM valves when cranking
  uint8_t iacCrankBins[4]; //Temperature Bins for the above 2 curves

  uint8_t iacAlgorithm : 3; //Valid values are: "None", "On/Off", "PWM", "PWM Closed Loop", "Stepper", "Stepper Closed Loop"
  uint8_t iacStepTime : 3; //How long to pulse the stepper for to ensure the step completes (ms)
  uint8_t iacChannels : 1; //How many outputs to use in PWM mode (0 = 1 channel, 1 = 2 channels)
  uint8_t iacPWMdir : 1; //Direction of the PWM valve. 0 = Normal = Higher RPM with more duty. 1 = Reverse = Lower RPM with more duty

  uint8_t iacFastTemp; //Fast idle temp when using a simple on/off valve

  uint8_t iacStepHome; //When using a stepper motor, the number of steps to be taken on startup to home the motor
  uint8_t iacStepHyster; //Hysteresis temperature (*10). Eg 2.2C = 22

  uint8_t fanInv : 1;        // Fan output inversion bit
  uint8_t fanEnable : 1;     // Fan enable bit. 0=Off, 1=On/Off
  uint8_t fanPin : 6;
  uint8_t fanSP;             // Cooling fan start temperature
  uint8_t fanHyster;         // Fan hysteresis
  uint8_t fanFreq;           // Fan PWM frequency
  uint8_t fanPWMBins[4];     //Temperature Bins for the PWM fan control

#if defined(CORE_AVR)
  };
#else
  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
#endif

//Page 9 of the config mostly deals with CANBUS control
//See ini file for further info (Config Page 10 in the ini)
struct config9 {
  uint8_t enable_secondarySerial:1;            //enable secondary serial
  uint8_t intcan_available:1;                     //enable internal can module
  uint8_t enable_intcan:1;
  uint8_t caninput_sel[16];                    //bit status on/Can/analog_local/digtal_local if input is enabled
  uint16_t caninput_source_can_address[16];        //u16 [15] array holding can address of input
  uint8_t caninput_source_start_byte[16];     //u08 [15] array holds the start byte number(value of 0-7)
  uint16_t caninput_source_num_bytes;     //u16 bit status of the number of bytes length 1 or 2
  uint8_t unused10_67;
  uint8_t unused10_68;
  uint8_t enable_candata_out : 1;
  uint8_t canoutput_sel[8];
  uint16_t canoutput_param_group[8];
  uint8_t canoutput_param_start_byte[8];
  uint8_t canoutput_param_num_bytes[8];

  uint8_t unused10_110;
  uint8_t unused10_111;
  uint8_t unused10_112;
  uint8_t unused10_113;
  uint8_t speeduino_tsCanId:4;         //speeduino TS canid (0-14)
  uint16_t true_address;            //speeduino 11bit can address
  uint16_t realtime_base_address;   //speeduino 11 bit realtime base address
  uint16_t obd_address;             //speeduino OBD diagnostic address
  uint8_t Auxinpina[16];            //analog  pin number when internal aux in use
  uint8_t Auxinpinb[16];            // digital pin number when internal aux in use

  uint8_t iacStepperInv : 1;  //stepper direction of travel to allow reversing. 0=normal, 1=inverted.
  uint8_t iacCoolTime : 3; // how long to wait for the stepper to cool between steps

  uint8_t iacMaxSteps; // Step limit beyond which the stepper won't be driven. Should always be less than homing steps. Stored div 3 as per home steps.

  uint8_t unused10_155;
  uint8_t unused10_156;
  uint8_t unused10_157;
  uint8_t unused10_158;
  uint8_t unused10_159;
  uint8_t unused10_160;
  uint8_t unused10_161;
  uint8_t unused10_162;
  uint8_t unused10_163;
  uint8_t unused10_164;
  uint8_t unused10_165;
  uint8_t unused10_166;
  uint8_t unused10_167;
  uint8_t unused10_168;
  uint8_t unused10_169;
  uint8_t unused10_170;
  uint8_t unused10_171;
  uint8_t unused10_172;
  uint8_t unused10_173;
  uint8_t unused10_174;
  uint8_t unused10_175;
  uint8_t unused10_176;
  uint8_t unused10_177;
  uint8_t unused10_178;
  uint8_t unused10_179;
  uint8_t unused10_180;
  uint8_t unused10_181;
  uint8_t unused10_182;
  uint8_t unused10_183;
  uint8_t unused10_184;
  uint8_t unused10_185;
  uint8_t unused10_186;
  uint8_t unused10_187;
  uint8_t unused10_188;
  uint8_t unused10_189;
  uint8_t unused10_190;
  uint8_t unused10_191;
  
#if defined(CORE_AVR)
  };
#else
  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
#endif

/*
Page 10 - No specific purpose. Created initially for the cranking enrich curve
192 bytes long
See ini file for further info (Config Page 11 in the ini)
*/
struct config10 {
  uint8_t crankingEnrichBins[4]; //Bytes 0-4
  uint8_t crankingEnrichValues[4]; //Bytes 4-7

  //Byte 8
  uint8_t rotaryType : 2;
  uint8_t stagingEnabled : 1;
  uint8_t stagingMode : 1;
  uint8_t EMAPPin : 4;

  uint8_t rotarySplitValues[8]; //Bytes 9-16
  uint8_t rotarySplitBins[8]; //Bytes 17-24

  uint16_t boostSens; //Bytes 25-26
  uint8_t boostIntv; //Byte 27
  uint16_t stagedInjSizePri; //Bytes 28-29
  uint16_t stagedInjSizeSec; //Bytes 30-31
  uint8_t lnchCtrlTPS; //Byte 32

  uint8_t flexBoostBins[6]; //Byets 33-38
  int16_t flexBoostAdj[6];  //kPa to be added to the boost target @ current ethanol (negative values allowed). Bytes 39-50
  uint8_t flexFuelBins[6]; //Bytes 51-56
  uint8_t flexFuelAdj[6];   //Fuel % @ current ethanol (typically 100% @ 0%, 163% @ 100%). Bytes 57-62
  uint8_t flexAdvBins[6]; //Bytes 63-68
  uint8_t flexAdvAdj[6];    //Additional advance (in degrees) @ current ethanol (typically 0 @ 0%, 10-20 @ 100%). NOTE: THIS SHOULD BE A SIGNED VALUE BUT 2d TABLE LOOKUP NOT WORKING WITH IT CURRENTLY!
                            //And another three corn rows die.
                            //Bytes 69-74

  //Byte 75
  uint8_t n2o_enable : 2;
  uint8_t n2o_arming_pin : 6;
  uint8_t n2o_minCLT; //Byte 76
  uint8_t n2o_maxMAP; //Byte 77
  uint8_t n2o_minTPS; //Byte 78
  uint8_t n2o_maxAFR; //Byte 79

  //Byte 80
  uint8_t n2o_stage1_pin : 6;
  uint8_t n2o_pin_polarity : 1;
  uint8_t n2o_stage1_unused : 1;
  uint8_t n2o_stage1_minRPM; //Byte 81
  uint8_t n2o_stage1_maxRPM; //Byte 82
  uint8_t n2o_stage1_adderMin; //Byte 83
  uint8_t n2o_stage1_adderMax; //Byte 84
  uint8_t n2o_stage1_retard; //Byte 85

  //Byte 86
  uint8_t n2o_stage2_pin : 6;
  uint8_t n2o_stage2_unused : 2;
  uint8_t n2o_stage2_minRPM; //Byte 87
  uint8_t n2o_stage2_maxRPM; //Byte 88
  uint8_t n2o_stage2_adderMin; //Byte 89
  uint8_t n2o_stage2_adderMax; //Byte 90
  uint8_t n2o_stage2_retard; //Byte 91

  //Byte 92
  uint8_t knock_mode : 2;
  uint8_t knock_pin : 6;

  //Byte 93
  uint8_t knock_trigger : 1;
  uint8_t knock_pullup : 1;
  uint8_t knock_limiterDisable : 1;
  uint8_t knock_unused : 2;
  uint8_t knock_count : 3;

  uint8_t knock_threshold; //Byte 94
  uint8_t knock_maxMAP; //Byte 95
  uint8_t knock_maxRPM; //Byte 96
  uint8_t knock_window_rpms[6]; //Bytes 97-102
  uint8_t knock_window_angle[6]; //Bytes 103-108
  uint8_t knock_window_dur[6]; //Bytes 109-114

  uint8_t knock_maxRetard; //Byte 115
  uint8_t knock_firstStep; //Byte 116
  uint8_t knock_stepSize; //Byte 117
  uint8_t knock_stepTime; //Byte 118
        
  uint8_t knock_duration; //Time after knock retard starts that it should start recovering. Byte 119
  uint8_t knock_recoveryStepTime; //Byte 120
  uint8_t knock_recoveryStep; //Byte 121

  //Byte 122
  uint8_t fuel2Algorithm : 3;
  uint8_t fuel2Mode : 3;
  uint8_t fuel2SwitchVariable : 2;

  //Bytes 123-124
  uint16_t fuel2SwitchValue;

  //Byte 125
  uint8_t fuel2InputPin : 6;
  uint8_t fuel2InputPolarity : 1;
  uint8_t fuel2InputPullup : 1;

  uint8_t vvtCLholdDuty; //Byte 126
  uint8_t vvtCLKP; //Byte 127
  uint8_t vvtCLKI; //Byte 128
  uint8_t vvtCLKD; //Byte 129
  int16_t vvtCLMinAng; //Bytes 130-131
  int16_t vvtCLMaxAng; //Bytes 132-133

  uint8_t crankingEnrichTaper; //Byte 134

  uint8_t fuelPressureEnable : 1;
  uint8_t oilPressureEnable : 1;
  uint8_t oilPressureProtEnbl : 1;
  uint8_t unused10_135 : 5;

  uint8_t fuelPressurePin : 4;
  uint8_t oilPressurePin : 4;

  int8_t fuelPressureMin;
  uint8_t fuelPressureMax;
  int8_t oilPressureMin;
  uint8_t oilPressureMax;

  uint8_t oilPressureProtRPM[4];
  uint8_t oilPressureProtMins[4];

  uint8_t wmiEnabled : 1; // Byte 149
  uint8_t wmiMode : 6;
  
  uint8_t wmiAdvEnabled : 1;

  uint8_t wmiTPS; // Byte 150
  uint8_t wmiRPM; // Byte 151
  uint8_t wmiMAP; // Byte 152
  uint8_t wmiMAP2; // Byte 153
  uint8_t wmiIAT; // Byte 154
  int8_t wmiOffset; // Byte 155

  uint8_t wmiIndicatorEnabled : 1; // 156
  uint8_t wmiIndicatorPin : 6;
  uint8_t wmiIndicatorPolarity : 1;

  uint8_t wmiEmptyEnabled : 1; // 157
  uint8_t wmiEmptyPin : 6;
  uint8_t wmiEmptyPolarity : 1;

  uint8_t wmiEnabledPin; // 158

  uint8_t wmiAdvBins[6]; //Bytes 159-164
  uint8_t wmiAdvAdj[6];  //Additional advance (in degrees)
                      //Bytes 165-170
  uint8_t vvtCLminDuty;
  uint8_t vvtCLmaxDuty;
  uint8_t vvt2Pin : 6;
  uint8_t unused11_174_1 : 1;
  uint8_t unused11_174_2 : 1;

  uint8_t fuelTempBins[6];
  uint8_t fuelTempValues[6]; //180

  //Byte 186
  uint8_t spark2Algorithm : 3;
  uint8_t spark2Mode : 3;
  uint8_t spark2SwitchVariable : 2;

  //Bytes 187-188
  uint16_t spark2SwitchValue;

  //Byte 189
  uint8_t spark2InputPin : 6;
  uint8_t spark2InputPolarity : 1;
  uint8_t spark2InputPullup : 1;

  uint8_t unused11_187_191[2]; //Bytes 187-191

#if defined(CORE_AVR)
  };
#else
  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
#endif

struct cmpOperation{
  uint8_t firstCompType : 3;
  uint8_t secondCompType : 3;
  uint8_t bitwise : 2;
};

/*
Page 13 - Programmable outputs conditions.
128 bytes long
*/
struct config13 {
  uint8_t outputInverted;
  uint8_t unused12_1;
  uint8_t outputPin[8];
  uint8_t outputDelay[8]; //0.1S
  uint8_t firstDataIn[8];
  uint8_t secondDataIn[8];
  uint8_t unused_13[16];
  int16_t firstTarget[8];
  int16_t secondTarget[8];
  //89bytes
  struct cmpOperation operation[8];

  uint16_t candID[8]; //Actual CAN ID need 16bits, this is a placeholder

  uint8_t unused12_106_127[22];

#if defined(CORE_AVR)
  };
#else
  } __attribute__((__packed__)); //The 32 bit systems require all structs to be fully packed
#endif

extern uint8_t pinInjector1; //Output pin injector 1
extern uint8_t pinInjector2; //Output pin injector 2
extern uint8_t pinInjector3; //Output pin injector 3
extern uint8_t pinInjector4; //Output pin injector 4
extern uint8_t pinInjector5; //Output pin injector 5
extern uint8_t pinInjector6; //Output pin injector 6
extern uint8_t pinInjector7; //Output pin injector 7
extern uint8_t pinInjector8; //Output pin injector 8
extern uint8_t injectorOutputControl; //Specifies whether the injectors are controlled directly (Via an IO pin) or using something like the MC33810
extern uint8_t pinCoil1; //Pin for coil 1
extern uint8_t pinCoil2; //Pin for coil 2
extern uint8_t pinCoil3; //Pin for coil 3
extern uint8_t pinCoil4; //Pin for coil 4
extern uint8_t pinCoil5; //Pin for coil 5
extern uint8_t pinCoil6; //Pin for coil 6
extern uint8_t pinCoil7; //Pin for coil 7
extern uint8_t pinCoil8; //Pin for coil 8
extern uint8_t ignitionOutputControl; //Specifies whether the coils are controlled directly (Via an IO pin) or using something like the MC33810
extern uint8_t pinTrigger; //The CAS pin
extern uint8_t pinTrigger2; //The Cam Sensor pin
extern uint8_t pinTrigger3;	//the 2nd cam sensor pin
extern uint8_t pinTPS;//TPS input pin
extern uint8_t pinMAP; //MAP sensor pin
extern uint8_t pinEMAP; //EMAP sensor pin
extern uint8_t pinMAP2; //2nd MAP sensor (Currently unused)
extern uint8_t pinIAT; //IAT sensor pin
extern uint8_t pinCLT; //CLS sensor pin
extern uint8_t pinO2; //O2 Sensor pin
extern uint8_t pinO2_2; //second O2 pin
extern uint8_t pinBat; //Battery voltage pin
extern uint8_t pinDisplayReset; // OLED reset pin
extern uint8_t pinTachOut; //Tacho output
extern uint8_t pinFuelPump; //Fuel pump on/off
extern uint8_t pinIdle1; //Single wire idle control
extern uint8_t pinIdle2; //2 wire idle control (Not currently used)
extern uint8_t pinIdleUp; //Input for triggering Idle Up
extern uint8_t pinIdleUpOutput; //Output that follows (normal or inverted) the idle up pin
extern uint8_t pinCTPS; //Input for triggering closed throttle state
extern uint8_t pinFuel2Input; //Input for switching to the 2nd fuel table
extern uint8_t pinSpark2Input; //Input for switching to the 2nd ignition table
extern uint8_t pinSpareTemp1; // Future use only
extern uint8_t pinSpareTemp2; // Future use only
extern uint8_t pinSpareOut1; //Generic output
extern uint8_t pinSpareOut2; //Generic output
extern uint8_t pinSpareOut3; //Generic output
extern uint8_t pinSpareOut4; //Generic output
extern uint8_t pinSpareOut5; //Generic output
extern uint8_t pinSpareOut6; //Generic output
extern uint8_t pinSpareHOut1; //spare high current output
extern uint8_t pinSpareHOut2; // spare high current output
extern uint8_t pinSpareLOut1; // spare low current output
extern uint8_t pinSpareLOut2; // spare low current output
extern uint8_t pinSpareLOut3;
extern uint8_t pinSpareLOut4;
extern uint8_t pinSpareLOut5;
extern uint8_t pinBoost;
extern uint8_t pinVVT_1;		// vvt output 1
extern uint8_t pinVVT_2;		// vvt output 2
extern uint8_t pinFan;       // Cooling fan output
extern uint8_t pinStepperDir; //Direction pin for the stepper motor driver
extern uint8_t pinStepperStep; //Step pin for the stepper motor driver
extern uint8_t pinStepperEnable; //Turning the DRV8825 driver on/off
extern uint8_t pinLaunch;
extern uint8_t pinIgnBypass; //The pin used for an ignition bypass (Optional)
extern uint8_t pinFlex; //Pin with the flex sensor attached
extern uint8_t pinVSS; 
extern uint8_t pinBaro; //Pin that an external barometric pressure sensor is attached to (If used)
extern uint8_t pinResetControl; // Output pin used control resetting the Arduino
extern uint8_t pinFuelPressure;
extern uint8_t pinOilPressure;
extern uint8_t pinWMIEmpty; // Water tank empty sensor
extern uint8_t pinWMIIndicator; // No water indicator bulb
extern uint8_t pinWMIEnabled; // ON-OFF ouput to relay/pump/solenoid 
extern uint8_t pinMC33810_1_CS;
extern uint8_t pinMC33810_2_CS;
#ifdef USE_SPI_EEPROM
  extern uint8_t pinSPIFlash_CS;
#endif


/* global variables */ // from speeduino.ino
//#ifndef UNIT_TEST

//#endif

extern struct statuses currentStatus; //The global status object
extern struct config2 configPage2;
extern struct config4 configPage4;
extern struct config6 configPage6;
extern struct config9 configPage9;
extern struct config10 configPage10;
extern struct config13 configPage13;
//extern uint8_t cltCalibrationTable[CALIBRATION_TABLE_SIZE]; /**< An array containing the coolant sensor calibration values */
//extern uint8_t iatCalibrationTable[CALIBRATION_TABLE_SIZE]; /**< An array containing the inlet air temperature sensor calibration values */
//extern uint8_t o2CalibrationTable[CALIBRATION_TABLE_SIZE]; /**< An array containing the O2 sensor calibration values */

extern uint16_t cltCalibration_bins[32];
extern uint16_t cltCalibration_values[32];
extern uint16_t iatCalibration_bins[32];
extern uint16_t iatCalibration_values[32];
extern uint16_t o2Calibration_bins[32];
extern uint8_t  o2Calibration_values[32]; // Note 8-bit values
extern struct table2D cltCalibrationTable; /**< A 32 bin array containing the coolant temperature sensor calibration values */
extern struct table2D iatCalibrationTable; /**< A 32 bin array containing the inlet air temperature sensor calibration values */
extern struct table2D o2CalibrationTable; /**< A 32 bin array containing the O2 sensor calibration values */

static_assert(sizeof(struct config2) == 128, "configPage2 size is not 128");
static_assert(sizeof(struct config4) == 128, "configPage4 size is not 128");
static_assert(sizeof(struct config6) == 128, "configPage6 size is not 128");
static_assert(sizeof(struct config9) == 192, "configPage9 size is not 192");
static_assert(sizeof(struct config10) == 192, "configPage10 size is not 192");
static_assert(sizeof(struct config13) == 128, "configPage13 size is not 128");
#endif // GLOBALS_H
