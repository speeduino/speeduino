/**
 * @file
 * 
 * @brief The statuses struct and related defines. 
 * 
 */

#pragma once

#include <stdint.h>
#include "bit_manip.h"
#include "atomic.h"

using byte = uint8_t;

/** @brief The status struct with current values for all 'live' variables.
* 
* Instantiated as global currentStatus.
* 
* @note int *ADC (Analog-to-digital value / count) values contain the "raw" value from AD conversion, which get converted to
* unit based values in similar variable(s) without ADC part in name (see sensors.ino for reading of sensors).
*/
struct statuses {
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool hasSync : 1; /**< Flag for crank/cam position being known by decoders (See decoders.ino).
  This is used for sanity checking e.g. before logging tooth history or reading some sensors and computing readings. */
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool initialisationComplete : 1; ///< Tracks whether the setup() function has run completely
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool clutchTrigger : 1;
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool previousClutchTrigger : 1;
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool fpPrimed : 1; ///< Tracks whether or not the fuel pump priming has been completed yet
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool injPrimed : 1; ///< Tracks whether or not the injector priming has been completed yet
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool tachoSweepEnabled : 1;
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool tachoAlt : 1;
    
  uint16_t RPM;   ///< RPM - Current Revs per minute
  byte RPMdiv100; ///< RPM value scaled (divided by 100) to fit a byte (0-255, e.g. 12000 => 120)
  long longRPM;   ///< RPM as long int (gets assigned to / maintained in statuses.RPM as well)
  uint16_t baroADC;
  long MAP;     ///< Manifold absolute pressure. Has to be a long for PID calcs (Boost control)
  uint16_t EMAP; ///< EMAP ... (See @ref config6.useEMAP for EMAP enablement)
  uint8_t baro;   ///< Barometric pressure is simply the initial MAP reading, taken before the engine is running. Alternatively, can be taken from an external sensor
  uint8_t TPS;    /**< The current TPS reading (0% - 100%). Is the tpsADC value after the calibration is applied */
  uint8_t tpsADC; /**< byte (valued: 0-255) representation of the TPS. Downsampled from the original 10-bit (0-1023) reading, but before any calibration is applied */
  int16_t tpsDOT; /**< TPS delta over time. Measures the % per second that the TPS is changing. Note that is signed value, because TPSdot can be also negative */
  byte TPSlast; /**< The previous TPS reading */
  int16_t mapDOT; /**< MAP delta over time. Measures the kpa per second that the MAP is changing. Note that is signed value, because MAPdot can be also negative */
  volatile int rpmDOT; /**< RPM delta over time (RPM increase / s ?) */
  byte VE;     /**< The current VE value being used in the fuel calculation. Can be the same as VE1 or VE2, or a calculated value of both. */
  byte VE1;    /**< The VE value from fuel table 1 */
  byte VE2;    /**< The VE value from fuel table 2, if in use (and required conditions are met) */
  uint8_t O2;     /**< Primary O2 sensor reading */
  uint8_t O2_2;   /**< Secondary O2 sensor reading */
  int coolant; /**< Coolant temperature reading */
  uint16_t cltADC;
  int IAT;     /**< Inlet air temperature reading */
  uint16_t iatADC;
  uint16_t O2ADC;
  uint16_t O2_2ADC;
  uint16_t dwell;          ///< dwell (coil primary winding/circuit on) time (in ms * 10 ? See @ref correctionsDwell)
  volatile uint16_t actualDwell;    ///< actual dwell time if new ignition mode is used (in uS)
  byte dwellCorrection; /**< The amount of correction being applied to the dwell time (in unit ...). */
  byte battery10;     /**< The current BRV in volts (multiplied by 10. Eg 12.5V = 125) */
  int8_t advance;     /**< The current advance value being used in the spark calculation. Can be the same as advance1 or advance2, or a calculated value of both */
  int8_t advance1;    /**< The advance value from ignition table 1 */
  int8_t advance2;    /**< The advance value from ignition table 2 */
  uint16_t corrections; /**< The total current corrections % amount */
  uint16_t AEamount;    /**< The amount of acceleration enrichment currently being applied. 100=No change. Varies above 255 */
  byte egoCorrection; /**< The amount of closed loop AFR enrichment currently being applied */
  byte wueCorrection; /**< The amount of warmup enrichment currently being applied */
  byte batCorrection; /**< The amount of battery voltage enrichment currently being applied */
  byte iatCorrection; /**< The amount of inlet air temperature adjustment currently being applied */
  byte baroCorrection; /**< The amount of correction being applied for the current baro reading */
  byte launchCorrection;   /**< The amount of correction being applied if launch control is active */
  byte flexCorrection;     /**< Amount of correction being applied to compensate for ethanol content */
  byte fuelTempCorrection; /**< Amount of correction being applied to compensate for fuel temperature */
  int8_t flexIgnCorrection;/**< Amount of additional advance being applied based on flex. Note the type as this allows for negative values */
  byte afrTarget;    /**< Current AFR Target looked up from AFR target table (x10 ? See @ref afrTable)*/
  byte CLIdleTarget; /**< The target idle RPM (when closed loop idle control is active) */
  bool idleUpActive; /**< Whether the externally controlled idle up is currently active */
  bool CTPSActive;   /**< Whether the externally controlled closed throttle position sensor is currently active */
  volatile byte ethanolPct; /**< Ethanol reading (if enabled). 0 = No ethanol, 100 = pure ethanol. Eg E85 = 85. */
  volatile int8_t fuelTemp;
  unsigned long AEEndTime; /**< The target end time used whenever AE (acceleration enrichment) is turned on */

  // Status1 fields as defined in the INI
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool isInj1Open : 1; ///< Injector 1 status: true == open, false == closed 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool isInj2Open : 1; ///< Injector 2 status: true == open, false == closed
  // cppcheck-suppress misra-c2012-6.1
  volatile bool isInj3Open : 1; ///< Injector 3 status: true == open, false == closed
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool isInj4Open : 1; ///< Injector 4 status: true == open, false == closed
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool isDFCOActive : 1;  ///< Deceleration Fuel Cut Off status: true == active, false == inactive
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool isToothLog1Full : 1; ///< Boost Cut status: true == active, false == inactive

  // Status2 fields as defined in the INI. 
  // TODO: resolve duplication with launchingHard
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool hardLaunchActive : 1; ///< Hard Launch status: true == on, false == off 
  // TODO: resolve duplication with launchingSoft
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool softLaunchActive : 1; ///< Soft Launch status: true == on, false == off 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool hardLimitActive : 1; ///< Hard limit status: true == on, false == off 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool softLimitActive : 1; ///< Soft limit status: true == on, false == off 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool idleOn : 1; ///< Is the idle code active : true == active, false == inactive
  // TODO: resolve duplication with hasSync
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool hasFullSync : 1; // Whether engine has sync (true) or not (false)

  // Status3 fields as defined in the INI.   
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool resetPreventActive : 1; ///< Reset prevent on (true) or off (false) 
  // TODO: resolve duplication with nitrous_status
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool nitrousActive : 1; ///< Nitrous on (true) or off (false)
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool secondFuelTableActive : 1; ///< Secondary fuel table is use (true) or not (false)
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool vssUiRefresh : 1; ///< Flag to indicate that the VSS value needs to be refreshed in the UI 
  // TODO: resolve duplication with hasSync & hasFullSync
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  volatile bool halfSync : 1;  ///< 
  // TODO: resolve duplication with nSquirts
  unsigned int nSquirtsStatus: 3; ///< 

  // Status4 fields as defined in the INI.   
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool wmiTankEmpty : 1; ///< Is the Water Methanol Injection tank empty (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool vvt1AngleError : 1; ///< VVT1 cam angle within limits (false) or not (true)
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool vvt2AngleError : 1; ///< VVT2 cam angle within limits (false) or not (true)
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool fanOn : 1; ///< Engine fan status (true == on, false == off)
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool burnPending : 1;  ///< Is an EEPROM burn pending (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool stagingActive : 1; ///< Is fuel injection staging active (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool commCompat : 1; ///< 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool allowLegacyComms : 1; ///< 

  // Status5 fields as defined in the INI. 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool flatShiftSoftCut : 1; ///< Is the flat shift soft cut active (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool secondSparkTableActive : 1; ///< Secondary spark table is use (true) or not (false)
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool knockRetardActive : 1; ///< Is knock retardation active (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool knockPulseDetected : 1;  ///<
  // TODO: resolve duplication with clutchTrigger
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool clutchTriggerActive : 1; ///< Is the clutch trigger active (true) or not (false)

  // Engine status fields as defined in the INI.  
  // TODO: engine has 3 states: Off, Cranking, Running. Need to capture this better 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineIsRunning : 1; ///< Is engine running (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineIsCranking : 1; ///< Is engine cranking (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool aseIsActive : 1; ///< Is After Start Enrichment (ASE) active (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool wueIsActive : 1; ///< Is Warm Up Enrichment (WUE) active (true) or not (false) 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  // TODO: acceleration has 3 states: Steady, Accelerating, Decelerating. Need to capture this better 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool isAcceleratingTPS : 1;  ///< Are we accelerating (true) or not (false), based on TPS
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool isDeceleratingTPS : 1; ///< Are we decelerating (true) or not (false), based on TPS
  
  // TODO: make all pulse widths uint16_t
  unsigned int PW1; ///< In uS
  unsigned int PW2; ///< In uS
  unsigned int PW3; ///< In uS
  unsigned int PW4; ///< In uS
  unsigned int PW5; ///< In uS
  unsigned int PW6; ///< In uS
  unsigned int PW7; ///< In uS
  unsigned int PW8; ///< In uS
  volatile byte runSecs; /**< Counter of seconds since cranking commenced (Maxes out at 255 to prevent overflow) */
  volatile byte secl; /**< Counter incrementing once per second. Will overflow after 255 and begin again. This is used by TunerStudio to maintain comms sync */
  volatile uint16_t loopsPerSecond; /**< A performance indicator showing the number of main loops that are being executed each second */ 
  bool launchingSoft; /**< Indicator showing whether soft launch control adjustments are active */
  bool launchingHard; /**< Indicator showing whether hard launch control adjustments are active */
  // TODO: remove this: only updated & read in logger
  uint16_t freeRAM;
  // TODO: make all RPMs uint16_t
  unsigned int clutchEngagedRPM; /**< The RPM at which the clutch was last depressed. Used for distinguishing between launch control and flat shift */ 
  bool flatShiftingHard;
  volatile uint32_t startRevolutions; /**< A counter for how many revolutions have been completed since sync was achieved. */
  uint16_t boostTarget;
  // TODO: resolve conflict with testActive
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool isTestModeActive : 1; // Is hardware test mode on?
  bool testActive;    // Not in use ? Replaced by testOutputs ?
  uint16_t boostDuty; ///< Boost Duty percentage value * 100 to give 2 points of precision
  byte idleLoad;      ///< Either the current steps or current duty cycle for the idle control
  uint16_t canin[16]; ///< 16bit raw value of selected canin data for channels 0-15
  uint8_t current_caninchannel = 0; /**< Current CAN channel, defaults to 0 */
  uint16_t crankRPM = 400; /**< The actual cranking RPM limit. This is derived from the value in the config page, but saves us multiplying it every time it's used (Config page value is stored divided by 10) */
  int16_t flexBoostCorrection; /**< Amount of boost added based on flex */
  byte nitrous_status;
  byte nSquirts;  ///< Number of injector squirts per cycle (per injector)
  uint16_t fuelLoad;
  uint16_t ignLoad;
  bool fuelPumpOn; /**< Indicator showing the current status of the fuel pump */
  volatile byte syncLossCounter;
  byte knockRetard;
  volatile byte knockCount;
  bool toothLogEnabled;
  byte compositeTriggerUsed; // 0 means composite logger disabled, 2 means use secondary input (1st cam), 3 means use tertiary input (2nd cam), 4 means log both cams together
  int16_t vvt1Angle; //Has to be a long for PID calcs (CL VVT control)
  byte vvt1TargetAngle;
  long vvt1Duty; //Has to be a long for PID calcs (CL VVT control)
  uint16_t injAngle;
  byte ASEValue;
  uint16_t vss;      /**< Current speed reading. Natively stored in kph and converted to mph in TS if required */
  bool idleUpOutputActive; /**< Whether the idle up output is currently active */
  byte gear;         /**< Current gear (Calculated from vss) */
  byte fuelPressure; /**< Fuel pressure in PSI */
  byte oilPressure;  /**< Oil pressure in PSI */

  // engineProtectStatus fields as defined in the INI. Needs to be accessible as a byte for I/O, so use type punning.
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineProtectRpm : 1; ///< Engine protection is active (true) due to exceeding RPM limits 
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineProtectBoostCut : 1; ///< Engine protection is active (true) due to exceeding MAP limits
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineProtectOil : 1; ///< Engine protection is active (true) due to minimum oil pressure limits
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineProtectAfr : 1; ///< Engine protection is active (true) based on maximum AFR limits
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineProtectClt : 1; ///< Engine protection is active (true) based on exceeding coolant limits
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool engineProtectIoError : 1; ///<

  byte fanDuty;
  byte wmiPW;
  int16_t vvt2Angle; //Has to be a long for PID calcs (CL VVT control)
  byte vvt2TargetAngle;
  long vvt2Duty; //Has to be a long for PID calcs (CL VVT control)
  byte outputsStatus;

  // SD card status fields.
  // TODO: conditional compile on SD_LOGGING once board definition is separated from globals.h
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool sdCardPresent : 1; ///< true if a card is present, false if not
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  /** TODO ogalic unsigned int */ bool sdCardType : 1; ///< 0==SD, 1==SDHC
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool sdCardReady : 1; ///< true if ready, false if not
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool sdCardLogging : 1; ///< true if logging active, false if not
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool sdCardError : 1;  ///< true if error, false if not
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  /** TODO ogalic unsigned int */ bool sdCardFS : 1;  ///< File system type 0=no FAT16, 1=FAT32
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool sdCardUnused : 1;  ///< true if unused, false if not

  // airConStatus fields.
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconRequested : 1; ///< Indicates whether the A/C button is pressed
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconCompressorOn : 1; ///< Indicates whether the A/C compressor is running
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconRpmLockout : 1; ///< Indicates the A/C is locked out due to the RPM being too high/low, or the post-high/post-low-RPM "stand-down" lockout period
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconTpsLockout : 1; ///< Indicates the A/C is locked out due to high TPS, or the post-high-TPS "stand-down" lockout period
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconTurningOn : 1;  ///< Indicates the A/C request is on (i.e. A/C button pressed), the lockouts are off, however the start delay has not yet elapsed. This gives the idle up time to kick in before the compressor.
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconCltLockout : 1;  ///< Indicates the A/C is locked out either due to high coolant temp.
  // cppcheck-suppress misra-c2012-6.1 ; False positive - MISRA C:2012 Rule (R 6.1) permits the use of boolean for bit fields.
  bool airconFanOn : 1;  ///< Indicates whether the A/C fan is running
  
  uint8_t systemTemp;
};

/**
 * @brief Non-atomic version of HasAnySync. **Should only be called in an ATOMIC() block***
 * 
 */
static inline bool HasAnySyncUnsafe(const statuses &status) {
  return status.hasSync || status.halfSync;
}

static inline bool HasAnySync(const statuses &status) {
  ATOMIC() {
    return HasAnySyncUnsafe(status);
  }
  return false; // Just here to avoid compiler warning.
}

static inline bool isEngineProtectActive(const statuses &status) {
  return status.engineProtectRpm
        || status.engineProtectBoostCut
        || status.engineProtectOil
        || status.engineProtectAfr
        || status.engineProtectClt;
}

static inline void resetEngineProtect(statuses &status) {
  status.engineProtectRpm = false;
  status.engineProtectBoostCut = false;
  status.engineProtectOil = false;
  status.engineProtectAfr = false;
  status.engineProtectClt = false;
  status.engineProtectIoError = false;
}