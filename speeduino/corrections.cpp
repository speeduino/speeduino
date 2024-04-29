/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/** @file
Corrections to injection pulsewidth.
The corrections functions in this file affect the fuel pulsewidth (Either increasing or decreasing)
based on factors other than the VE lookup.

These factors include:
- Temperature (Warmup Enrichment and After Start Enrichment)
- Acceleration/Deceleration
- Flood clear mode
- etc.

Most correction functions return value 100 (like 100% == 1) for no need for correction.

There are 2 top level functions that call more detailed corrections for Fuel and Ignition respectively:
- @ref correctionsFuel() - All fuel related corrections
- @ref correctionsIgn() - All ignition related corrections
*/
//************************************************************************************************************

#include "globals.h"
#include "corrections.h"
#include "timers.h"
#include "maths.h"
#include "sensors.h"
#include "unit_testing.h"
#include "preprocessor.h"
#include "src/PID_v1/PID_v1.h"
#include "units.h"
#include "fuel_calcs.h"
#include "unit_testing.h"

static long PID_O2, PID_output, PID_AFRTarget;
/** Instance of the PID object in case that algorithm is used (Always instantiated).
* Needs to be global as it maintains state outside of each function call.
* Comes from Arduino (?) PID library.
*/
static PID egoPID(&PID_O2, &PID_output, &PID_AFRTarget, configPage6.egoKP, configPage6.egoKI, configPage6.egoKD, REVERSE);

static uint8_t aeActivatedReading; //The mapDOT/tpsDOT value seen when the MAE/TAE was activated. 

TESTABLE_STATIC uint16_t AFRnextCycle;
static unsigned long knockStartTime;
static uint8_t knockLastRecoveryStep;
//static int16_t knockWindowMin; //The current minimum crank angle for a knock pulse to be valid
//static int16_t knockWindowMax;//The current maximum crank angle for a knock pulse to be valid
static uint8_t dfcoTaper;

TESTABLE_STATIC table2D_u8_u8_4 taeTable(&configPage4.taeBins, &configPage4.taeValues);
TESTABLE_STATIC table2D_u8_u8_4 maeTable(&configPage4.maeBins, &configPage4.maeRates);
TESTABLE_STATIC table2D_u8_u8_10 WUETable(&configPage4.wueBins, &configPage2.wueValues);
TESTABLE_STATIC table2D_u8_u8_4 ASETable(&configPage2.aseBins, &configPage2.asePct);
TESTABLE_STATIC table2D_u8_u8_4 ASECountTable(&configPage2.aseBins, &configPage2.aseCount);
TESTABLE_STATIC table2D_u8_u8_4 crankingEnrichTable(&configPage10.crankingEnrichBins, &configPage10.crankingEnrichValues);
TESTABLE_STATIC table2D_u8_u8_6 dwellVCorrectionTable(&configPage6.voltageCorrectionBins, &configPage4.dwellCorrectionValues);
TESTABLE_STATIC table2D_u8_u8_6 injectorVCorrectionTable(&configPage6.voltageCorrectionBins, &configPage6.injVoltageCorrectionValues);
TESTABLE_STATIC table2D_u8_u8_9 IATDensityCorrectionTable(&configPage6.airDenBins, &configPage6.airDenRates);
TESTABLE_STATIC table2D_u8_u8_8 baroFuelTable(&configPage4.baroFuelBins, &configPage4.baroFuelValues);
TESTABLE_STATIC table2D_u8_u8_6 IATRetardTable(&configPage4.iatRetBins, &configPage4.iatRetValues);
TESTABLE_STATIC table2D_u8_u8_6 idleAdvanceTable(&configPage4.idleAdvBins, &configPage4.idleAdvValues);
TESTABLE_STATIC table2D_u8_u8_6 CLTAdvanceTable(&configPage4.cltAdvBins, &configPage4.cltAdvValues);
TESTABLE_STATIC table2D_u8_u8_6 flexFuelTable(&configPage10.flexFuelBins, &configPage10.flexFuelAdj);
TESTABLE_STATIC table2D_u8_u8_6 flexAdvTable(&configPage10.flexAdvBins, &configPage10.flexAdvAdj);
TESTABLE_STATIC table2D_u8_u8_6 fuelTempTable(&configPage10.fuelTempBins, &configPage10.fuelTempValues);
TESTABLE_STATIC table2D_u8_u8_6 wmiAdvTable(&configPage10.wmiAdvBins, &configPage10.wmiAdvAdj);

// Constant that represents "no fuel correction"
static constexpr uint8_t NO_FUEL_CORRECTION = ONE_HUNDRED_PCT;
// Constant that represents the baseline fuel correction to be modified
// (yes, it's the same as NO_FUEL_CORRECTION, but captures a slightly different concept)
static constexpr uint8_t BASELINE_FUEL_CORRECTION = ONE_HUNDRED_PCT;


/** Initialise instances and vars related to corrections (at ECU boot-up).
 */
void initialiseCorrections(void)
{
  PID_output = 0L;
  PID_O2 = 0L;
  PID_AFRTarget = 0L;
  // Toggling between modes resets the PID internal state
  // This is required by the unit tests
  // TODO: modify PID code to provide a method to reset it. 
  egoPID.SetMode(AUTOMATIC);
  egoPID.SetMode(MANUAL);
  egoPID.SetMode(AUTOMATIC);

  currentStatus.flexIgnCorrection = 0;
  //Default value of no adjustment must be set to avoid randomness on first correction cycle after startup
  currentStatus.egoCorrection = NO_FUEL_CORRECTION; 
  currentStatus.ASEValue = NO_FUEL_CORRECTION;
  currentStatus.wueCorrection = NO_FUEL_CORRECTION;
  currentStatus.iatCorrection = NO_FUEL_CORRECTION;
  currentStatus.baroCorrection = NO_FUEL_CORRECTION;
  currentStatus.batCorrection = NO_FUEL_CORRECTION;
  AFRnextCycle = 0;
  currentStatus.knockRetardActive = false;
  currentStatus.knockPulseDetected = false;
  currentStatus.knockCount = 1;
  knockLastRecoveryStep = 0;
  knockStartTime = 0;
  currentStatus.battery10 = 125; //Set battery voltage to sensible value for dwell correction for "flying start" (else ignition gets spurious pulses after boot)  
}

// ============================= Warm Up Enrichment =============================

/** Warm Up Enrichment (WUE) corrections.
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add
*/
TESTABLE_INLINE_STATIC uint8_t correctionWUE(void)
{
  uint8_t WUEValue = currentStatus.wueCorrection;

  // Only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, CLT_READ_TIMER_BIT) ) { 
    if (currentStatus.coolant >= temperatureRemoveOffset(WUETable.axis[WUETable.size()-1U]))
    {
      //This prevents us doing the 2D lookup if we're already up to temp
      currentStatus.wueIsActive = false;
      WUEValue = WUETable.values[WUETable.size()-1U];
    }
    else
    {
      currentStatus.wueIsActive = true;
      WUEValue = table2D_getValue(&WUETable, temperatureAddOffset(currentStatus.coolant));
    }
  }

  return WUEValue;
}

// ============================= Cranking Enrichment =============================

/** Cranking Enrichment corrections.
Additional fuel % to be added when the engine is cranking
*/

static inline uint16_t lookUpCrankingEnrichmentPct(void) {
  return toWorkingU8U16(CRANKING_ENRICHMENT, 
                        table2D_getValue(&crankingEnrichTable, temperatureAddOffset(currentStatus.coolant)));
}

//Taper start value needs to account for ASE that is now running, so total correction does not increase when taper begins
static inline uint16_t computeCrankingTaperStartPct(uint16_t crankingPercent) {
  // Avoid 32-bit division if possible
  if (currentStatus.aseIsActive && currentStatus.ASEValue!=NO_FUEL_CORRECTION) {
    return fast_div32_16((uint32_t)crankingPercent * UINT32_C(100), currentStatus.ASEValue);
  };

  return crankingPercent;
}

TESTABLE_INLINE_STATIC uint16_t correctionCranking(void)
{
  static uint8_t crankingEnrichTaper = 0U;

  uint16_t crankingPercent = NO_FUEL_CORRECTION;

  //Check if we are actually cranking
  if ( currentStatus.engineIsCranking )
  {
    crankingPercent = lookUpCrankingEnrichmentPct();
    crankingEnrichTaper = 0U;
  }
  //If we're not cranking, check if if cranking enrichment tapering to ASE should be done
  else if ( crankingEnrichTaper < configPage10.crankingEnrichTaper )
  {
    crankingPercent = (uint16_t) map( crankingEnrichTaper, 
                                      0U, configPage10.crankingEnrichTaper, 
                                      computeCrankingTaperStartPct(lookUpCrankingEnrichmentPct()), NO_FUEL_CORRECTION); //Taper from start value to 100%
    if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { crankingEnrichTaper++; }
  } else {
    // Not cranking and taper not in effect, so no cranking enrichment needed.
    // just need to keep MISRA checker happy.
  }

  return max((uint16_t)NO_FUEL_CORRECTION, (uint16_t)crankingPercent);
}

// ============================= After Start Enrichment =============================

/** After Start Enrichment calculation.
 * This is a short period (Usually <20 seconds) immediately after the engine first fires (But not when cranking)
 * where an additional amount of fuel is added (Over and above the WUE amount).
 * 
 * @return uint8_t The After Start Enrichment modifier as a %. 100% = No modification. 
 */   
TESTABLE_INLINE_STATIC uint8_t correctionASE(void)
{
  // We use aseTaper both to track taper AND as a flag value
  // to tell when ASE is complete and avoid unnecessary table lookups.
  constexpr uint8_t ASE_COMPLETE = UINT8_MAX;
  static uint8_t aseTaper = 0U;

  uint8_t ASEValue = NO_FUEL_CORRECTION;

  if (currentStatus.engineIsCranking) {
    // Engine is cranking - mark ASE as inactive and ready to run 
    currentStatus.aseIsActive = false;
    aseTaper = 0U; 
    ASEValue = NO_FUEL_CORRECTION;
  } else if (aseTaper!=ASE_COMPLETE) {
    // ASE hasn't started or isn't complete.
    if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ))
    {
      // We only update ASE every 100ms for performance reasons - coolant
      // doesn't change temperature that quickly. 
      //
      // We must use 100ms (rather than CLT_READ_TIMER_BIT) since aseTaper counts tenths of a second.
      
      if (aseTaper==0U // Avoid table lookup if taper is being applied
       && (currentStatus.runSecs < table2D_getValue(&ASECountTable, temperatureAddOffset(currentStatus.coolant))))
      {
        currentStatus.aseIsActive = true;
        ASEValue = BASELINE_FUEL_CORRECTION + table2D_getValue(&ASETable, temperatureAddOffset(currentStatus.coolant));
      } else if ( aseTaper < configPage2.aseTaperTime ) { //Check if we've reached the end of the taper time
        currentStatus.aseIsActive = true;
        ASEValue = BASELINE_FUEL_CORRECTION + (uint8_t)map(aseTaper, 
                                        0U, configPage2.aseTaperTime, 
                                        table2D_getValue(&ASETable, temperatureAddOffset(currentStatus.coolant)), 0);
        aseTaper = aseTaper + 1U;
      } else {
        // ASE has finished
        currentStatus.aseIsActive = false;
        aseTaper = ASE_COMPLETE; // Flag ASE as complete
        ASEValue = NO_FUEL_CORRECTION;
      }
    } else {
      // ASE is in effect, but we're not due to update, so reuse previous value.
      ASEValue = currentStatus.ASEValue;
    }    
  } else {
    // ASE is finished, nothing to do but keep MISRA checker happy 
  }

  return ASEValue;
}

// ============================= Acceleration Enrichment =============================

static inline void accelEnrichmentOff(void) {
  currentStatus.isAcceleratingTPS = false;
  currentStatus.isDeceleratingTPS = false;
  currentStatus.AEamount = 0;
}

static inline bool isAccelEnrichmentOn(void) {
  return (currentStatus.isAcceleratingTPS) || (currentStatus.isDeceleratingTPS);
}

static inline uint8_t applyAeRpmTaper(uint8_t accelCorrection) {
  //Apply the RPM taper to the above
  //The RPM settings are stored divided by 100:
  if ((configPage2.aeTaperMax>configPage2.aeTaperMin) && (accelCorrection>0U)) {
    const uint16_t taperMinRpm = toWorkingU8U16(RPM_COARSE, configPage2.aeTaperMin);
    if ((currentStatus.RPM > taperMinRpm))
    {
      const uint16_t taperMaxRpm = toWorkingU8U16(RPM_COARSE, configPage2.aeTaperMax);
      if(currentStatus.RPM > taperMaxRpm) { 
        //RPM is beyond taper max limit, so accel enrich is turned off
        accelCorrection = 0U;
      } else {
        //The percentage of the way through the RPM taper range
        const uint8_t taperPercent = map( currentStatus.RPM,
                                          taperMinRpm, taperMaxRpm,
                                          ONE_HUNDRED_PCT, 0U); 
        accelCorrection = (uint8_t)percentage(taperPercent, accelCorrection); //Calculate the above percentage of the calculated accel amount. 
      }
    }
  }

  return accelCorrection;
}

static inline uint16_t applyAeCoolantTaper(uint16_t accelCorrection) {
  //Apply AE cold coolant modifier, if CLT is less than taper end temperature
  if ( (accelCorrection!=0U)
    && (configPage2.aeColdPct!=NO_FUEL_CORRECTION)
    && (currentStatus.coolant < temperatureRemoveOffset(configPage2.aeColdTaperMax) ))
  {
    //If CLT is less than taper min temp, apply full modifier on top of accelCorrection
    if ( currentStatus.coolant <= temperatureRemoveOffset(configPage2.aeColdTaperMin) )
    {
      accelCorrection =  (uint16_t)percentage(configPage2.aeColdPct, accelCorrection);
    }
    //If CLT is between taper min and max, taper the modifier value and apply it on top of accelCorrection
    else
    {
      // Tune uses 100% as no adjustment, range 100% to 255%. So subtract 100 to get the adjustment, 
      // scale the adjustment over the coolant RPM range & reapply the 100 offset 
      const uint8_t coldPct = BASELINE_FUEL_CORRECTION + map( temperatureAddOffset(currentStatus.coolant),
                                                              configPage2.aeColdTaperMin, configPage2.aeColdTaperMax,
                                                              configPage2.aeColdPct-ONE_HUNDRED_PCT, 0U);       
      accelCorrection = (uint16_t)percentage(coldPct, accelCorrection);
    }
  }

  return accelCorrection;
}

static inline uint16_t calcAccelEnrichment(const uint8_t accelCorrection) {
  currentStatus.isAcceleratingTPS = true;
  return BASELINE_FUEL_CORRECTION + applyAeCoolantTaper(applyAeRpmTaper(accelCorrection));
}

static inline uint16_t calcDeccelEnrichment(void) {
  currentStatus.isDeceleratingTPS = true;
  return configPage2.decelAmount; //In decel, use the decel fuel amount as accelCorrection
}

static inline bool aeTimeoutExpired(void) {
  return micros() >= currentStatus.AEEndTime;
}

//Set the time in the future where the enrichment will be turned off. 
static inline void updateAeTimeout(void) {
  // taeTime is stored as mS / 10, so multiply it by 10000 to get it in uS
  currentStatus.AEEndTime = micros() + toWorkingU32(TIME_TENTH_MILLIS, configPage2.aeTime); 
}

using aeTimeoutExpiredCallback_t = void (*)(void);
using shouldResetCurrentAeCallback_t = bool (*)(void);
using shouldStartAeCallback_t = bool (*)(void);
using computAeCallback_t = uint16_t (*)(void);

// Implements the skeleton of the AE algorithm. Callers fill in specific steps via callbacks
// (Template Method design pattern in C!)
static inline uint16_t correctionAccel( const aeTimeoutExpiredCallback_t onTimeoutExpired, 
                                        const shouldResetCurrentAeCallback_t shouldResetCurrentAe, 
                                        const shouldStartAeCallback_t shouldStartAe, 
                                        const computAeCallback_t computeAe) {
  uint16_t accelCorrection = NO_FUEL_CORRECTION;

  //First, check whether the accel. enrichment is already running
  if (isAccelEnrichmentOn()) {
    //If it is currently running, check whether it should still be running or whether it's reached it's end time
    if (aeTimeoutExpired()) {
      accelEnrichmentOff();
      // Timed out, reset	
      onTimeoutExpired();
    //Need to check whether the accel amount has increased from when AE was turned on
    //If the accel amount HAS increased, we clear the current enrich phase and a new one will be started below
     } else if(shouldResetCurrentAe()) {
        accelEnrichmentOff();
    } else {
      //Enrichment still needs to keep running. 
      //Simply return the current amount
      accelCorrection = currentStatus.AEamount;
    }
  }

  //Need to check this again as it may have been changed in the above section (Both ACC and DCC are off if this has changed)
  if ((!isAccelEnrichmentOn()) && (shouldStartAe())) {
    updateAeTimeout();
    accelCorrection = computeAe();
  } 

  return accelCorrection;
}

static inline void mapOnTimeoutExpired(void) { 
  currentStatus.mapDOT = 0; 
}

static inline bool mapShouldResetAe(void) {
  return (uint16_t)abs(currentStatus.mapDOT) > aeActivatedReading; 
}

static inline bool mapShouldStartAe(void) { 
  return (uint16_t)abs(currentStatus.mapDOT) > configPage2.maeThresh; 
};

static inline uint16_t mapComputeAe(void) {
  uint16_t aeEnrichment = 0U;

  if (currentStatus.mapDOT < 0) {
    aeEnrichment = calcDeccelEnrichment();
  } else if (currentStatus.mapDOT > 0) {
    aeEnrichment = calcAccelEnrichment(table2D_getValue(&maeTable, toRawU8(MAP_DOT, currentStatus.mapDOT)));
  } else {
    // Steady state - nothing to do.
  }
  
  aeActivatedReading = abs(currentStatus.mapDOT);
  
  return aeEnrichment;
}

static inline int16_t computeMapDot(void) {
  int16_t mapDOT = 0U;
  const int16_t mapChange = getMAPDelta();
  // Check for only very small movement. This not only means we can skip the lookup, but helps reduce false triggering around 0-2% throttle openings
  if ( ((uint16_t)abs(mapChange) > configPage2.maeMinChange)) {
    const uint32_t mapDeltaT = getMAPDeltaTime();

    static constexpr uint32_t MAX_udiv_32_16 = UINT16_MAX; 
    static constexpr uint32_t MIN_udiv_32_16 = (MICROS_PER_SEC/UINT16_MAX)+1U; 
    // Faster division path. Will almost always be taken when above idle - a 360° cycle time of 65535µS
    // equals 915 RPM.
    if ((mapDeltaT<=MAX_udiv_32_16) && (mapDeltaT>MIN_udiv_32_16)) {
      mapDOT = (int16_t)fast_div32_16(MICROS_PER_SEC, mapDeltaT) * (int16_t)mapChange; 
    } else {
      mapDOT = (int16_t)(MICROS_PER_SEC / mapDeltaT) * mapChange;
    }

    static constexpr int16_t MAP_DOT_MIN = -2550;
    static constexpr int16_t MAP_DOT_MAX = 2550;
    mapDOT = constrain(mapDOT, MAP_DOT_MIN, MAP_DOT_MAX);
  }
  return mapDOT;
}

static inline uint16_t correctionAccelModeMap(void) {
  uint16_t aeCorrection = currentStatus.AEamount;

  // No point in updating faster than the MAP sensor is read
  if (BIT_CHECK(LOOP_TIMER, MAP_READ_TIMER_BIT)) {
    currentStatus.mapDOT = computeMapDot();

    aeCorrection = correctionAccel(mapOnTimeoutExpired, mapShouldResetAe, mapShouldStartAe, mapComputeAe);
  }

  return aeCorrection;
}

static inline void tpsOnTimeoutExpired(void) { 
  currentStatus.tpsDOT = 0; 
}

static inline bool tpsShouldResetAe(void) { 
  return (uint16_t)abs(currentStatus.tpsDOT) > aeActivatedReading; 
}

static inline bool tpsShouldStartAe(void) { 
  return (uint16_t)abs(currentStatus.tpsDOT) > configPage2.taeThresh;
}

static inline uint16_t tpsComputeAe(void) {
  uint16_t aeEnrichment = 0U;

  //Check if the TPS rate of change is negative or positive. Negative means deceleration.
  if (currentStatus.tpsDOT < 0) {
    aeEnrichment = calcDeccelEnrichment();
  } else if (currentStatus.tpsDOT > 0) {
    aeEnrichment = calcAccelEnrichment(table2D_getValue(&taeTable, toRawU8(TPS_DOT, currentStatus.tpsDOT))); 
  } else {
    // Steady state - nothing to do.
  }
  aeActivatedReading = abs(currentStatus.tpsDOT);

  return aeEnrichment;
}

static inline int16_t computeTPSDOT(void) {
  //Get the TPS rate change
  const int16_t tpsChange = (int16_t)currentStatus.TPS - (int16_t)currentStatus.TPSlast;
  
  int16_t tpsDOT = 0;
  // Check for only very small movement. This not only means we can skip the lookup, but helps reduce false triggering around 0-2% throttle openings
  if ((uint16_t)abs(tpsChange) > configPage2.taeMinChange) {
    tpsDOT = (TPS_READ_FREQUENCY * tpsChange) / 2; //This is the % per second that the TPS has moved, adjusted for the 0.5% resolution of the TPS

    static constexpr int16_t TPS_DOT_MIN = -2550;
    static constexpr int16_t TPS_DOT_MAX = 2550;
    tpsDOT = constrain(tpsDOT, TPS_DOT_MIN, TPS_DOT_MAX);
  } 
  return tpsDOT;
}

static inline uint16_t correctionAccelModeTps(void) {
  uint16_t aeCorrection = currentStatus.AEamount;

  // No point in updating faster than the TPS is read
  if (BIT_CHECK(LOOP_TIMER, TPS_READ_TIMER_BIT)) {
    currentStatus.tpsDOT = computeTPSDOT();

    aeCorrection = correctionAccel(tpsOnTimeoutExpired, tpsShouldResetAe, tpsShouldStartAe, tpsComputeAe);
  }

  return aeCorrection;
}

/** Acceleration enrichment correction calculation.
 * 
 * Calculates the % change of the throttle over time (%/second) and performs a lookup based on this
 * Coolant-based modifier is applied on the top of this.
 * When the enrichment is turned on, it runs at that amount for a fixed period of time (taeTime)
 * 
 * @return uint16_t The Acceleration enrichment modifier as a %. 100% = No modification.
 * 
 * As the maximum enrichment amount is +255% and maximum cold adjustment for this is 255%, the overall return value
 * from this function can be 100+(255*255/100)=750. Hence this function returns a uint16_t rather than uint8_t.
 */
TESTABLE_INLINE_STATIC uint16_t correctionAccel(void)
{
  if(AE_MODE_MAP==configPage2.aeMode) {
    return correctionAccelModeMap();
  }
  if(AE_MODE_TPS==configPage2.aeMode) {
    return correctionAccelModeTps();
  }
  return NO_FUEL_CORRECTION;
}

// ============================= Flood Clear =============================

static inline bool isFloodClearActive(const statuses &current, const config4 &page4) {
  return current.engineIsCranking
      && current.TPS >= page4.floodClear;
}

/** Simple check to see whether we are cranking with the TPS above the flood clear threshold.
@return 100 (not cranking and thus no need for flood-clear) or 0 (Engine cranking and TPS above @ref config4.floodClear limit).
*/
TESTABLE_INLINE_STATIC uint8_t correctionFloodClear(void)
{
  return isFloodClearActive(currentStatus, configPage4) ? 0U : NO_FUEL_CORRECTION;
}

/** Battery Voltage correction.
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add.
*/
TESTABLE_INLINE_STATIC byte correctionBatVoltage(void)
{
  // No point in updating more often than the sensor is read
  uint8_t correction = currentStatus.batCorrection;
  if( BIT_CHECK(LOOP_TIMER, BAT_READ_TIMER_BIT) ) { 
    correction = table2D_getValue(&injectorVCorrectionTable, currentStatus.battery10);
  }
  return correction;
}

/** Simple temperature based corrections lookup based on the inlet air temperature (IAT).
This corrects for changes in air density from movement of the temperature.
*/
TESTABLE_INLINE_STATIC uint8_t correctionIATDensity(void)
{
  // Performance: only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, IAT_READ_TIMER_BIT) ) { 
    return table2D_getValue(&IATDensityCorrectionTable, temperatureAddOffset(currentStatus.IAT)); //currentStatus.IAT is the actual temperature, values in IATDensityCorrectionTable.axisX are temp+offset
  }
  return currentStatus.iatCorrection;
}

// ============================= Baro pressure correction =============================

/** Correction for current barometric / ambient pressure.
 * @returns A percentage value indicating the amount the fuelling should be changed based on the barometric reading. 100 = No change. 110 = 10% increase. 90 = 10% decrease
 */
TESTABLE_INLINE_STATIC uint8_t correctionBaro(void)
{
  // No point in updating more often than the sensor is read
  if( BIT_CHECK(LOOP_TIMER, BARO_READ_TIMER_BIT) ) { 
    return (uint8_t)table2D_getValue(&baroFuelTable, currentStatus.baro);
  }
  return currentStatus.baroCorrection;
}

// ============================= Launch control correction =============================

/** Launch control has a setting to increase the fuel load to assist in bringing up boost.
This simple check applies the extra fuel if we're currently launching
*/
TESTABLE_INLINE_STATIC uint8_t correctionLaunch(void)
{
  return BASELINE_FUEL_CORRECTION + ((currentStatus.launchingHard || currentStatus.launchingSoft) ? configPage6.lnchFuelAdd : 0U);
}

// ============================= Deceleration Fuel Cut Off (DFCO) correction =============================

TESTABLE_INLINE_STATIC uint8_t correctionDFCOfuel(void)
{
  uint8_t scaleValue = NO_FUEL_CORRECTION;
  if ( currentStatus.isDFCOActive )
  {
    if ( (configPage9.dfcoTaperEnable == 1U) && (dfcoTaper != 0U) )
    {
      //Do a check if the user reduced the duration while active to avoid overflow
      if (dfcoTaper > configPage9.dfcoTaperTime) { dfcoTaper = configPage9.dfcoTaperTime; }
      scaleValue = map(dfcoTaper, configPage9.dfcoTaperTime, 0, NO_FUEL_CORRECTION, configPage9.dfcoTaperFuel);
      if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { dfcoTaper--; }
    }
    else { scaleValue = 0; } //Taper ended or disabled, disable fuel
  }
  else { dfcoTaper = configPage9.dfcoTaperTime; } //Keep updating the duration until DFCO is active

  return scaleValue;
}

/*
 * Returns true if deceleration fuel cutoff should be on, false if its off
 */
TESTABLE_INLINE_STATIC bool correctionDFCO(void)
{
  bool DFCOValue = false;
  if ( configPage2.dfcoEnabled == 1U )
  {
    static uint8_t dfcoDelay;

    if ( currentStatus.isDFCOActive ) 
    {
      DFCOValue = ( currentStatus.RPM > ( configPage4.dfcoRPM * 10U) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ); 
      if ( DFCOValue == false) { dfcoDelay = 0; }
    }
    else 
    {
      if ( (currentStatus.TPS < configPage4.dfcoTPSThresh) && (currentStatus.coolant >= temperatureRemoveOffset(configPage2.dfcoMinCLT)) && ( currentStatus.RPM > (unsigned int)( (configPage4.dfcoRPM * 10U) + (configPage4.dfcoHyster * 2U)) ) )
      {
        if( dfcoDelay < configPage2.dfcoDelay )
        {
          if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { dfcoDelay++; }
        }
        else { DFCOValue = true; }
      }
      else { dfcoDelay = 0; } //Prevent future activation right away if previous time wasn't activated
    } // DFCO active check
  } // DFCO enabled check
  return DFCOValue;
}

// ============================= Flex fuel correction =============================

/** Flex fuel adjustment to vary fuel based on ethanol content.
 * The amount of extra fuel required is a linear relationship based on the % of ethanol.
*/
TESTABLE_INLINE_STATIC uint8_t correctionFlex(void)
{
  return configPage2.flexEnabled ? table2D_getValue(&flexFuelTable, currentStatus.ethanolPct) : NO_FUEL_CORRECTION;
}

// ============================= Fuel temperature correction =============================

/*
 * Fuel temperature adjustment to vary fuel based on fuel temperature reading
*/
TESTABLE_INLINE_STATIC uint8_t correctionFuelTemp(void)
{
  return configPage2.flexEnabled ? table2D_getValue(&fuelTempTable, temperatureAddOffset(currentStatus.fuelTemp)) : NO_FUEL_CORRECTION;
}


// ============================= Air Fuel Ratio (AFR) correction =============================

uint8_t calculateAfrTarget(table3d16RpmLoad &afrLookUpTable, const statuses &current, const config2 &page2, const config6 &page6) {
  //afrTarget value lookup must be done if O2 sensor is enabled, and always if incorporateAFR is enabled
  if (page2.incorporateAFR == true) {
    return get3DTableValue(&afrLookUpTable, current.fuelLoad, current.RPM);
  }
  if (page6.egoType!=EGO_TYPE_OFF) 
  {
    //Determine whether the Y axis of the AFR target table tshould be MAP (Speed-Density) or TPS (Alpha-N)
    //Note that this should only run after the sensor warmup delay when using Include AFR option,
    if( current.runSecs > page6.ego_sdelay) { 
      return get3DTableValue(&afrLookUpTable, current.fuelLoad, current.RPM); 
    }
    return current.O2; //Catch all
  }
  return current.afrTarget;
}

static inline uint8_t computeSimpleLeanCorrection(const statuses &current, const config6 &page6) {
  if(current.egoCorrection < (BASELINE_FUEL_CORRECTION + page6.egoLimit) ) //Fuelling adjustment must be at most the egoLimit amount (up or down)
  {
    return current.egoCorrection+1U; //Increase the fuelling by 1%
  }
  return current.egoCorrection; //Means we're at the maximum adjustment amount, so simply return that again
}

static inline uint8_t computeSimpleRichCorrection(const statuses &current, const config6 &page6) {
  if(current.egoCorrection > (BASELINE_FUEL_CORRECTION - page6.egoLimit) ) //Fuelling adjustment must be at most the egoLimit amount (up or down)
  {
    return (current.egoCorrection - 1U); //Decrease the fuelling by 1%
  }
  return current.egoCorrection; //Means we're at the maximum adjustment amount, so simply return that again  
}

static inline uint8_t computeSimpleCorrection(const statuses &current, const config6 &page6) {
  if(current.O2 > current.afrTarget) {
    return computeSimpleLeanCorrection(current, page6);
  }
  if(current.O2 < current.afrTarget) {
    return computeSimpleRichCorrection(current, page6);
  }
  return current.egoCorrection; //Means we're already right on target
}

static inline uint8_t computePIDCorrection(const statuses &current, const config6 &page6) {
  //Set the limits again, just in case the user has changed them since the last loop. 
  //Note that these are sent to the PID library as (Eg:) -15 and +15
  egoPID.SetOutputLimits(-page6.egoLimit, page6.egoLimit); 
  //Set the PID values again, just in case the user has changed them since the last loop
  egoPID.SetTunings(page6.egoKP, page6.egoKI, page6.egoKD); 
  PID_O2 = (long)(current.O2);
  PID_AFRTarget = (long)(current.afrTarget);

  (void)egoPID.Compute();
  // Can't do this in one step: MISRA compliance.
  int8_t correction = (int8_t)BASELINE_FUEL_CORRECTION + (int8_t)PID_output;
  return (uint8_t)correction;
}

static inline bool nextAfrCycleHasStarted(void) {
  //Check whether ignitionCount has exceeded AFRnextCycle.
  //This method prevents any issues when AFRnextCycle overflows but these variables 
  //cannot be more than UINT16_HALF_RANGE apart
  return (((uint16_t)(ignitionCount - AFRnextCycle)) < UINT16_HALF_RANGE);
}

static inline void setNextAfrCycle(void) {
  AFRnextCycle = ignitionCount + configPage6.egoCount; //Set the target ignition event for the next calculation
}

static inline bool isAfrClosedLoopOperational(const statuses &current, const config6 &page6, const config9 &page9) {
  return (current.coolant > temperatureRemoveOffset(page6.egoTemp)) 
      && (current.RPM > toWorkingU8U16(RPM_COARSE, page6.egoRPM)) 
      && (current.TPS <= page6.egoTPSMax) 
      && (current.O2 < page6.ego_max) 
      && (current.O2 > page6.ego_min) 
      && (current.runSecs > page6.ego_sdelay) 
      && (!current.isDFCOActive) 
      && (current.MAP <= (long)toWorkingU8U16(MAP, page9.egoMAPMax) ) 
      && (current.MAP >= (long)toWorkingU8U16(MAP, page9.egoMAPMin) )
      ;
}

static inline bool isValidEgoAlgorithm(const config6 &page6) {
  return (page6.egoAlgorithm != EGO_ALGORITHM_INVALID1)
      && (page6.egoAlgorithm != EGO_ALGORITHM_NONE);
}

static inline bool isAfrCorrectionEnabled(const statuses &current, const config6 &page6) {
  return (page6.egoType!=EGO_TYPE_OFF) 
      // If DFCO is active do not run the ego controllers to prevent iterator wind-up.
      && !current.isDFCOActive
      && isValidEgoAlgorithm(page6);
}

static inline uint8_t computeAFRCorrection(const statuses &current, const config6 &page6) {
  uint8_t correction = NO_FUEL_CORRECTION;

  if (page6.egoAlgorithm == EGO_ALGORITHM_SIMPLE) {
    correction = computeSimpleCorrection(current, page6);
  } else if(page6.egoAlgorithm == EGO_ALGORITHM_PID) {
    correction = computePIDCorrection(current, page6);
  } else {
    // Unknown algorithm - use default & keep MISRA checker happy;
  }

  return correction;
}

/** Lookup the AFR target table and perform either a simple or PID adjustment based on this.

Simple (Best suited to narrowband sensors):
If the O2 sensor reports that the mixture is lean/rich compared to the desired AFR target, it will make a 1% adjustment
It then waits egoDelta number of ignition events and compares O2 against the target table again. If it is still lean/rich then the adjustment is increased to 2%.

This continues until either:
- the O2 reading flips from lean to rich, at which point the adjustment cycle starts again at 1% or
- the adjustment amount increases to egoLimit at which point it stays at this level until the O2 state (rich/lean) changes

PID (Best suited to wideband sensors):

*/
TESTABLE_INLINE_STATIC uint8_t correctionAFRClosedLoop(void)
{
  uint8_t correction = NO_FUEL_CORRECTION;
  
  if (isAfrCorrectionEnabled(currentStatus, configPage6)) {
    if (nextAfrCycleHasStarted()) {
      setNextAfrCycle();
        
      //Check all other requirements for closed loop adjustments
      if (isAfrClosedLoopOperational(currentStatus, configPage6, configPage9)) {
        correction = computeAFRCorrection(currentStatus, configPage6);
      }
    } else {
      // Not within the upcoming cycle, so reuse current correction
      correction = currentStatus.egoCorrection; 
    }
  } //egoType

  // Final check to ensure within authority range (This can be needed if the user has lowered the authority limit)
  return clamp( correction, 
                (uint8_t)(BASELINE_FUEL_CORRECTION - configPage6.egoLimit), 
                (uint8_t)(BASELINE_FUEL_CORRECTION + configPage6.egoLimit));
}


static inline uint32_t combineCorrections(uint32_t sumCorrections, uint16_t correction) {
  if (correction == NO_FUEL_CORRECTION) {
    return sumCorrections;
  }
  return percentage(correction, sumCorrections);
}

/** Dispatch calculations for all fuel related corrections.
Calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
uint16_t correctionsFuel(void)
{
  //The values returned by each of the correction functions are multiplied together and then divided back to give a single 0-255 value.
  currentStatus.wueCorrection = correctionWUE();
  uint32_t sumCorrections = currentStatus.wueCorrection;

  currentStatus.ASEValue = correctionASE();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.ASEValue);

  sumCorrections = combineCorrections(sumCorrections, correctionCranking());

  currentStatus.AEamount = correctionAccel();
  if ( (configPage2.aeApplyMode == AE_MODE_MULTIPLIER) || (currentStatus.isDeceleratingTPS) ) // multiply by the AE amount in case of multiplier AE mode or Decel
  {
    sumCorrections = combineCorrections(sumCorrections, currentStatus.AEamount);
  }

  sumCorrections = combineCorrections(sumCorrections, correctionFloodClear());

  currentStatus.egoCorrection = correctionAFRClosedLoop();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.egoCorrection);

  //Voltage correction is applied to the injector opening time
  currentStatus.batCorrection = correctionBatVoltage();
  
  currentStatus.iatCorrection = correctionIATDensity();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.iatCorrection);

  currentStatus.baroCorrection = correctionBaro();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.baroCorrection);

  currentStatus.flexCorrection = correctionFlex();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.flexCorrection);

  currentStatus.fuelTempCorrection = correctionFuelTemp();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.fuelTempCorrection);

  currentStatus.launchCorrection = correctionLaunch();
  sumCorrections = combineCorrections(sumCorrections, currentStatus.launchCorrection);

  currentStatus.isDFCOActive = correctionDFCO();
  sumCorrections = combineCorrections(sumCorrections, correctionDFCOfuel());

  //This is the maximum allowable increase
  return min((uint16_t)1500U, (uint16_t)sumCorrections);
}

//******************************** IGNITION ADVANCE CORRECTIONS ********************************

/** Correct ignition timing to configured fixed value.
 * Must be called near end to override all other corrections.
 */
int8_t correctionFixedTiming(int8_t advance)
{
  return (configPage2.fixAngEnable == 1U) ? configPage4.FixAng : advance; //Check whether the user has set a fixed timing angle
}

/** Ignition correction for coolant temperature (CLT).
 */
TESTABLE_INLINE_STATIC int8_t correctionCLTadvance(int8_t advance)
{
  static uint8_t cachedValue = 0U; // Setting this to non-zero will use additional RAM for static initialisation
  // Performance: only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, CLT_READ_TIMER_BIT)) { 
    cachedValue = table2D_getValue(&CLTAdvanceTable, temperatureAddOffset(currentStatus.coolant));
  }
  return advance + (int8_t)cachedValue - 15;
}

/** Correct ignition timing to configured fixed value to use during craning.
 * Must be called near end to override all other corrections.
 */
int8_t correctionCrankingFixedTiming(int8_t advance)
{
  if ( currentStatus.engineIsCranking )
  { 
    if ( configPage2.crkngAddCLTAdv == 0U ) { 
      advance = configPage4.CrankAng; //Use the fixed cranking ignition angle
    } else { 
      advance = correctionCLTadvance(configPage4.CrankAng); //Use the CLT compensated cranking ignition angle
    }
  }
  return advance;
}

TESTABLE_INLINE_STATIC int8_t correctionFlexTiming(int8_t advance)
{
  if( configPage2.flexEnabled == 1U ) //Check for flex being enabled
  {
    //This gets cast to a signed 8 bit value to allows for negative advance (ie retard) values here.
    currentStatus.flexIgnCorrection = (int16_t) table2D_getValue(&flexAdvTable, currentStatus.ethanolPct) - OFFSET_IGNITION; //Negative values are achieved with offset
    return advance + currentStatus.flexIgnCorrection;
  }
  return advance;
}

TESTABLE_INLINE_STATIC int8_t correctionWMITiming(int8_t advance)
{
  if( (configPage10.wmiEnabled == 1U) && (configPage10.wmiAdvEnabled == 1U) && (!currentStatus.wmiTankEmpty) ) //Check for wmi being enabled
  {
    if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPM >= configPage10.wmiRPM) && (currentStatus.MAP/2 >= configPage10.wmiMAP) && (temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT) )
    {
      return advance + (int8_t)table2D_getValue(&wmiAdvTable, (uint8_t)((uint16_t)currentStatus.MAP/2U)) - OFFSET_IGNITION; //Negative values are achieved with offset
    }
  }
  return advance;
}

/** 
 * Ignition correction for inlet air temperature (IAT).
 */
TESTABLE_INLINE_STATIC int8_t correctionIATretard(int8_t advance)
{
  static uint8_t cachedValue = 0U; // Setting this to non-zero will use additional RAM for static initialisation
  // Performance: only update as fast as the sensor is read
  if( BIT_CHECK(LOOP_TIMER, IAT_READ_TIMER_BIT)) { 
    cachedValue = (uint8_t)table2D_getValue(&IATRetardTable, (uint8_t)currentStatus.IAT); // TODO: check if this needs converted
  }
  return (int16_t)advance - (int16_t)cachedValue;
}


/** Ignition Idle advance correction.
 */
static constexpr uint16_t IGN_IDLE_THRESHOLD = 200U; //RPM threshold (below CL idle target) for when ign based idle control will engage

static inline uint8_t computeIdleAdvanceRpmDelta(void) {
  int16_t idleRPMdelta = ((int16_t)currentStatus.CLIdleTarget - ((int16_t)currentStatus.RPM / 10) ) + 50;
  // Limit idle rpm delta between 0rpm - 100rpm
  return constrain(idleRPMdelta, 0, 100);
}

static inline int8_t applyIdleAdvanceAdjust(int8_t advance, int8_t adjustment) {
  if(configPage2.idleAdvEnabled == IDLEADVANCE_MODE_ADDED) { 
    return (advance + adjustment); 
  } else if(configPage2.idleAdvEnabled == IDLEADVANCE_MODE_SWITCHED) { 
    return adjustment;
  } else {
    // Unknown idle advance mode - do nothing
    return advance;
  }
}

static inline bool isIdleAdvanceOn(void) {
  return (configPage2.idleAdvEnabled != IDLEADVANCE_MODE_OFF) 
      && (runSecsX10 >= (configPage2.idleAdvDelay * 5U))
      && currentStatus.engineIsRunning
      /* When Idle advance is the only idle speed control mechanism, activate as soon as not cranking. 
      When some other mechanism is also present, wait until the engine is no more than 200 RPM below idle target speed on first time
      */
      && ((configPage6.iacAlgorithm == IAC_ALGORITHM_NONE) 
        || (currentStatus.RPM > (((uint16_t)currentStatus.CLIdleTarget * 10U) - (uint16_t)IGN_IDLE_THRESHOLD)));
}

static inline bool isIdleAdvanceOperational(void) {
  return (currentStatus.RPM < (configPage2.idleAdvRPM * 100U))
      && ((configPage2.vssMode == VSS_MODE_OFF) || (currentStatus.vss < configPage2.idleAdvVss))
      && (((configPage2.idleAdvAlgorithm == IDLEADVANCE_ALGO_TPS) && (currentStatus.TPS < configPage2.idleAdvTPS)) 
        || ((configPage2.idleAdvAlgorithm == IDLEADVANCE_ALGO_CTPS) && (currentStatus.CTPSActive == true)));// closed throttle position sensor (CTPS) based idle state
}

TESTABLE_INLINE_STATIC int8_t correctionIdleAdvance(int8_t advance)
{
  //Adjust the advance based on idle target rpm.
  if (isIdleAdvanceOn())
  {
    static uint8_t idleAdvDelayCount;
    if(isIdleAdvanceOperational())
    {
      if( idleAdvDelayCount < configPage9.idleAdvStartDelay )
      {
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { idleAdvDelayCount++; }
      }
      else
      {
        int8_t advanceIdleAdjust = (int8_t)(table2D_getValue(&idleAdvanceTable, computeIdleAdvanceRpmDelta())) - (int8_t)15;
        advance = applyIdleAdvanceAdjust(advance, (int8_t)advanceIdleAdjust); 
      }
    }
    else { idleAdvDelayCount = 0; }
  }

  return advance;
}

/** Ignition soft revlimit correction.
 */
static inline int8_t calculateSoftRevLimitAdvance(int8_t advance) {
  if (configPage2.SoftLimitMode == SOFT_LIMIT_RELATIVE) { 
    return advance - (int8_t)configPage4.SoftLimRetard; //delay timing by configured number of degrees in relative mode
  } else if (configPage2.SoftLimitMode == SOFT_LIMIT_FIXED) { 
    return (int8_t)configPage4.SoftLimRetard; //delay timing to configured number of degrees in fixed mode
  } else {
    // Unknown limit mode - do nothing, keep MISRA checker happy
    return advance;
  }
}

TESTABLE_INLINE_STATIC int8_t correctionSoftRevLimit(int8_t advance)
{
  currentStatus.softLimitActive = false;

  if (configPage6.engineProtectType == PROTECT_CUT_IGN || configPage6.engineProtectType == PROTECT_CUT_BOTH) 
  {
    if (currentStatus.RPMdiv100 >= configPage4.SoftRevLim) //Softcut RPM limit
    {
      currentStatus.softLimitActive = true;
      if( softLimitTime < configPage4.SoftLimMax )
      {
        advance = calculateSoftRevLimitAdvance(advance);
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { 
          softLimitTime++; 
        }
      }
    }
    else if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { 
      softLimitTime = 0; //Only reset time at runSecsX10 update rate
    } else {
      // Nothing to do, keep MISRA checker happy.
    }
  }

  return advance;
}

/** Ignition Nitrous oxide correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionNitrous(int8_t advance)
{
  //Check if nitrous is currently active
  if(configPage10.n2o_enable != NITROUS_OFF)
  {
    //Check which stage is running (if any)
    if( (currentStatus.nitrous_status == NITROUS_STAGE1) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      advance -= (int8_t)configPage10.n2o_stage1_retard;
    }
    if( (currentStatus.nitrous_status == NITROUS_STAGE2) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      advance -= (int8_t)configPage10.n2o_stage2_retard;
    }
  }

  return advance;
}
/** Ignition soft launch correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftLaunch(int8_t advance)
{
  //SoftCut rev limit for 2-step launch control.
  if(  configPage6.launchEnabled && currentStatus.clutchTrigger &&
      (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100U)) &&
      (currentStatus.RPM > ((unsigned int)(configPage6.lnchSoftLim) * 100U)) &&
      (currentStatus.TPS >= configPage10.lnchCtrlTPS) &&
      ( (configPage2.vssMode == VSS_MODE_OFF) || ((configPage2.vssMode!=VSS_MODE_OFF) && (currentStatus.vss <= configPage10.lnchCtrlVss)) )
    )
  {
    currentStatus.launchingSoft = true;
    currentStatus.softLaunchActive = true;
    advance = configPage6.lnchRetard;
  }
  else
  {
    currentStatus.launchingSoft = false;
    currentStatus.softLaunchActive = false;
  }

  return advance;
}
/** Ignition correction for soft flat shift.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftFlatShift(int8_t advance)
{
  if(configPage6.flatSEnable && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM > ((unsigned int)(configPage6.flatSArm) * 100U)) && (currentStatus.RPM > (currentStatus.clutchEngagedRPM - (configPage6.flatSSoftWin * 100U) ) ) )
  {
    currentStatus.flatShiftSoftCut = true;
    advance = configPage6.flatSRetard;
  }
  else { currentStatus.flatShiftSoftCut = false; }

  return advance;
}

static inline uint8_t _calculateKnockRecovery(uint8_t curKnockRetard)
{
  uint8_t tmpKnockRetard = curKnockRetard;
  //Check whether we are in knock recovery
  if((micros() - knockStartTime) > (configPage10.knock_duration * 100000UL)) //knock_duration is in seconds*10
  {
    //Calculate how many recovery steps have occurred since the 
    uint32_t timeInRecovery = (micros() - knockStartTime) - (configPage10.knock_duration * 100000UL);
    uint8_t recoverySteps = timeInRecovery / (configPage10.knock_recoveryStepTime * 100000UL);
    int8_t recoveryTimingAdj = 0;
    if(recoverySteps > knockLastRecoveryStep) 
    { 
      recoveryTimingAdj = (recoverySteps - knockLastRecoveryStep) * configPage10.knock_recoveryStep;
      knockLastRecoveryStep = recoverySteps;
    }

    if(recoveryTimingAdj < currentStatus.knockRetard)
    {
      //Add the timing back in provided we haven't reached the end of the recovery period
      tmpKnockRetard = currentStatus.knockRetard - recoveryTimingAdj;
    }
    else 
    {
      //Recovery is complete. Knock adjustment is set to 0 and we reset the knock status
      tmpKnockRetard = 0;
      currentStatus.knockRetardActive = false;
      knockStartTime = 0;
      currentStatus.knockCount = 0;
    }
  }

  return tmpKnockRetard;
}

/** Ignition knock (retard) correction.
 */
static inline int8_t correctionKnockTiming(int8_t advance)
{
  byte tmpKnockRetard = 0;

  if( (configPage10.knock_mode == KNOCK_MODE_DIGITAL)  )
  {
    //
    if(currentStatus.knockCount >= configPage10.knock_count)
    {
      if(currentStatus.knockRetardActive)
      {
        //Knock retard is currently active already.
        tmpKnockRetard = currentStatus.knockRetard;

        //Check if additional knock events occurred
        if(currentStatus.knockPulseDetected)
        {
          //Check if the latest event was far enough after the initial knock event to pull further timing
          if((micros() - knockStartTime) > (configPage10.knock_stepTime * 1000UL))
          {
            //Recalculate the amount timing being pulled
            currentStatus.knockCount++;
            tmpKnockRetard = configPage10.knock_firstStep + ((currentStatus.knockCount - configPage10.knock_count) * configPage10.knock_stepSize);
            knockStartTime = micros();
            knockLastRecoveryStep = 0;
          }
        }
        tmpKnockRetard = _calculateKnockRecovery(tmpKnockRetard);
      }
      else
      {
        //Knock currently inactive but needs to be active now
        knockStartTime = micros();
        tmpKnockRetard = configPage10.knock_firstStep + ((currentStatus.knockCount - configPage10.knock_count) * configPage10.knock_stepSize); //
        currentStatus.knockRetardActive = true;
        knockLastRecoveryStep = 0;
      }
    }

    currentStatus.knockPulseDetected = false; //Reset the knock pulse indicator
  }
  else if( (configPage10.knock_mode == KNOCK_MODE_ANALOG)  )
  {
    if(currentStatus.knockRetardActive)
    {
      //Check if additional knock events occurred
      //Additional knock events are when the step time has passed and the voltage remains above the threshold
      if((micros() - knockStartTime) > (configPage10.knock_stepTime * 1000UL))
      {
        //Sufficient time has passed, check the current knock value
        uint16_t tmpKnockReading = getAnalogKnock();

        if(tmpKnockReading > configPage10.knock_threshold)
        {
          currentStatus.knockCount++;
          tmpKnockRetard = configPage10.knock_firstStep + ((currentStatus.knockCount - configPage10.knock_count) * configPage10.knock_stepSize);
          knockStartTime = micros();
          knockLastRecoveryStep = 0;
        }   
      }
      tmpKnockRetard = _calculateKnockRecovery(tmpKnockRetard);
    }
    else
    {
      //If not is not currently active, we read the analog pin every 30Hz
      if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_30HZ) ) 
      { 
        uint16_t tmpKnockReading = getAnalogKnock();

        if(tmpKnockReading > configPage10.knock_threshold)
        {
          //Knock detected
          knockStartTime = micros();
          tmpKnockRetard = configPage10.knock_firstStep; //
          currentStatus.knockRetardActive = true;
          knockLastRecoveryStep = 0;
        }
      }
    }
  }
  

  tmpKnockRetard = min(tmpKnockRetard, configPage10.knock_maxRetard); //Ensure the commanded retard is not higher than the maximum allowed.
  currentStatus.knockRetard = tmpKnockRetard;
  return advance - tmpKnockRetard;
}

/** Ignition DFCO taper correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionDFCOignition(int8_t advance)
{
  if ( (configPage9.dfcoTaperEnable == 1U) && (currentStatus.isDFCOActive) )
  {
    if ( dfcoTaper != 0U )
    {
      advance -= map(dfcoTaper, configPage9.dfcoTaperTime, 0, 0, configPage9.dfcoTaperAdvance);
    }
    else { advance -= (int8_t)configPage9.dfcoTaperAdvance; } //Taper ended, use full value
  }
  else { dfcoTaper = configPage9.dfcoTaperTime; } //Keep updating the duration until DFCO is active
  return advance;
}

/** Ignition Dwell Correction.
 */
static inline uint8_t getPulsesPerRev(void) {
  if( ( (configPage4.sparkMode == IGN_MODE_SINGLE) || 
     ((configPage4.sparkMode == IGN_MODE_ROTARY) && (configPage10.rotaryType != ROTARY_IGN_RX8)) ) 
     //No point in running this for 1 cylinder engines
     && (configPage2.nCylinders > 1U) )  {
    return configPage2.nCylinders >> 1U;
  }
  return 1U;
}

static inline uint16_t adjustDwellClosedLoop(uint16_t dwell) {
    int16_t error = dwell - currentStatus.actualDwell;
    if(dwell > (uint16_t)INT16_MAX) { dwell = (uint16_t)INT16_MAX; } //Prevent overflow when casting to signed int
    if(error > ((int16_t)dwell / 2)) { error += error; } //Double correction amount if actual dwell is less than 50% of the requested dwell
    if(error > 0) { 
      return dwell + (uint16_t)error;
    }
    return dwell;
}

uint16_t correctionsDwell(uint16_t dwell)
{
  //Initialise the actualDwell value if this is the first time being called
  if(currentStatus.actualDwell == 0U) { 
    currentStatus.actualDwell = dwell; 
  } 

  //**************************************************************************************************************************
  //Pull battery voltage based dwell correction and apply if needed
  static uint8_t dwellCorrection = ONE_HUNDRED_PCT;
  if (BIT_CHECK(LOOP_TIMER, BAT_READ_TIMER_BIT)) { // Performance: only update as fast as the sensor is read
    dwellCorrection = (uint8_t)table2D_getValue(&dwellVCorrectionTable, currentStatus.battery10);
  }
  if (dwellCorrection != ONE_HUNDRED_PCT) { 
    dwell = div100(dwell) * dwellCorrection; 
  }

  //**************************************************************************************************************************
  //Dwell error correction is a basic closed loop to keep the dwell time consistent even when adjusting its end time for the per tooth timing.
  //This is mostly of benefit to low resolution triggers at low rpm (<1500)
  if( (configPage2.perToothIgn  == true) && (configPage4.dwellErrCorrect == 1U) ) {
    dwell = adjustDwellClosedLoop(dwell);
  }

  //**************************************************************************************************************************
  /*
  Dwell limiter - If the total required dwell time per revolution is longer than the maximum time available at the current RPM, reduce dwell. This can occur if there are multiple sparks per revolution
  This only times this can occur are:
  1. Single channel spark mode where there will be nCylinders/2 sparks per revolution
  2. Rotary ignition in wasted spark configuration (FC/FD), results in 2 pulses per rev. RX-8 is fully sequential resulting in 1 pulse, so not required
  */
  uint16_t sparkDur_uS = (configPage4.sparkDur * 100U); //Spark duration is in mS*10. Multiple it by 100 to get spark duration in uS
  uint8_t pulsesPerRevolution = getPulsesPerRev();
  uint16_t dwellPerRevolution = (dwell + sparkDur_uS) * pulsesPerRevolution;
  if(dwellPerRevolution > currentStatus.revolutionTime)
  {
    //Possibly need some method of reducing spark duration here as well, but this is a start
    uint16_t adjustedSparkDur = fast_div32_16(sparkDur_uS * currentStatus.revolutionTime, dwellPerRevolution);
    dwell = (pulsesPerRevolution==1U ? currentStatus.revolutionTime : fast_div32_16(currentStatus.revolutionTime, (uint16_t)pulsesPerRevolution)) - adjustedSparkDur;
  }

  return dwell;
}

/** Dispatch calculations for all ignition related corrections.
 * @param base_advance - Base ignition advance (deg. ?)
 * @return Advance considering all (~12) individual corrections
 */
int8_t correctionsIgn(int8_t base_advance)
{
  int8_t advance;
  advance = correctionFlexTiming(base_advance);
  advance = correctionWMITiming(advance);
  advance = correctionIATretard(advance);
  advance = correctionCLTadvance(advance);
  advance = correctionIdleAdvance(advance);
  advance = correctionSoftRevLimit(advance);
  advance = correctionNitrous(advance);
  advance = correctionSoftLaunch(advance);
  advance = correctionSoftFlatShift(advance);
  advance = correctionKnockTiming(advance);

  advance = correctionDFCOignition(advance);

  //Fixed timing check must go last
  advance = correctionFixedTiming(advance);
  advance = correctionCrankingFixedTiming(advance); //This overrides the regular fixed timing, must come last

  return advance;
}