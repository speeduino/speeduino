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

static byte aeActivatedReading; //The mapDOT/tpsDOT value seen when the MAE/TAE was activated. 

static bool idleAdvActive = false;
TESTABLE_STATIC uint16_t AFRnextCycle;
static unsigned long knockStartTime;
static uint8_t knockLastRecoveryStep;
//static int16_t knockWindowMin; //The current minimum crank angle for a knock pulse to be valid
//static int16_t knockWindowMax;//The current maximum crank angle for a knock pulse to be valid
static uint8_t aseTaper;
TESTABLE_STATIC uint8_t dfcoDelay;
static uint8_t idleAdvTaper;
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
  currentStatus.egoCorrection = NO_FUEL_CORRECTION; //Default value of no adjustment must be set to avoid randomness on first correction cycle after startup
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
  uint8_t WUEValue;
  //Possibly reduce the frequency this runs at (Costs about 50 loops per second)
  if (currentStatus.coolant > temperatureRemoveOffset(WUETable.axis[WUETable.size()-1U]))
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
    return udiv_32_16((uint32_t)crankingPercent * UINT32_C(100), currentStatus.ASEValue);
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
TESTABLE_INLINE_STATIC byte correctionASE(void)
{
  int16_t ASEValue = currentStatus.ASEValue;
  //Two checks are required:
  //1) Is the engine run time less than the configured ase time
  //2) Make sure we're not still cranking
  if( currentStatus.engineIsCranking != true )
  {
    if ( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) || (currentStatus.ASEValue == 0) )
    {
      if ( (currentStatus.runSecs < (table2D_getValue(&ASECountTable, temperatureAddOffset(currentStatus.coolant)))) && (!currentStatus.engineIsCranking) )
      {
        currentStatus.aseIsActive = true; //Mark ASE as active.
        ASEValue = BASELINE_FUEL_CORRECTION + table2D_getValue(&ASETable, temperatureAddOffset(currentStatus.coolant));
        aseTaper = 0;
      }
      else
      {
        if ( aseTaper < configPage2.aseTaperTime ) //Check if we've reached the end of the taper time
        {
          currentStatus.aseIsActive = true; //Mark ASE as active.
          ASEValue = BASELINE_FUEL_CORRECTION + map(aseTaper, 0, configPage2.aseTaperTime, table2D_getValue(&ASETable, temperatureAddOffset(currentStatus.coolant)), 0);
          aseTaper++;
        }
        else
        {
          currentStatus.aseIsActive = false; //Mark ASE as inactive.
          ASEValue = NO_FUEL_CORRECTION;
        }
      }
      
      //Safety checks
      if(ASEValue > UINT8_MAX) { ASEValue = UINT8_MAX; }
      
      if(ASEValue < 0) { ASEValue = 0; }
      ASEValue = (byte)ASEValue;
    }
  }
  else
  {
    //Engine is cranking, ASE disabled
    currentStatus.aseIsActive = false; //Mark ASE as inactive.
    ASEValue = NO_FUEL_CORRECTION;
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
  if ( (abs(mapChange) > configPage2.maeMinChange)) {
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
  if (BIT_CHECK(LOOP_TIMER, MAP_TIMER_BIT)) {
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
  if (abs(tpsChange) > configPage2.taeMinChange) {
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
  if (BIT_CHECK(LOOP_TIMER, TPS_TIMER_BIT)) {
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
 * from this function can be 100+(255*255/100)=750. Hence this function returns a uint16_t rather than byte.
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

/** Simple check to see whether we are cranking with the TPS above the flood clear threshold.
@return 100 (not cranking and thus no need for flood-clear) or 0 (Engine cranking and TPS above @ref config4.floodClear limit).
*/
TESTABLE_INLINE_STATIC byte correctionFloodClear(void)
{
  byte floodValue = NO_FUEL_CORRECTION;
  if( currentStatus.engineIsCranking )
  {
    //Engine is currently cranking, check what the TPS is
    if(currentStatus.TPS >= configPage4.floodClear)
    {
      //Engine is cranking and TPS is above threshold. Cut all fuel
      floodValue = 0;
    }
  }
  return floodValue;
}

/** Battery Voltage correction.
Uses a 2D enrichment table (WUETable) where the X axis is engine temp and the Y axis is the amount of extra fuel to add.
*/
TESTABLE_INLINE_STATIC byte correctionBatVoltage(void)
{
  byte batValue = NO_FUEL_CORRECTION;
  batValue = table2D_getValue(&injectorVCorrectionTable, currentStatus.battery10);
  return batValue;
}

/** Simple temperature based corrections lookup based on the inlet air temperature (IAT).
This corrects for changes in air density from movement of the temperature.
*/
TESTABLE_INLINE_STATIC byte correctionIATDensity(void)
{
  byte IATValue = NO_FUEL_CORRECTION;
  IATValue = table2D_getValue(&IATDensityCorrectionTable, temperatureAddOffset(currentStatus.IAT)); //currentStatus.IAT is the actual temperature, values in IATDensityCorrectionTable.axisX are temp+offset

  return IATValue;
}

/** Correction for current barometric / ambient pressure.
 * @returns A percentage value indicating the amount the fuelling should be changed based on the barometric reading. 100 = No change. 110 = 10% increase. 90 = 10% decrease
 */
TESTABLE_INLINE_STATIC byte correctionBaro(void)
{
  byte baroValue = NO_FUEL_CORRECTION;
  baroValue = table2D_getValue(&baroFuelTable, currentStatus.baro);

  return baroValue;
}

/** Launch control has a setting to increase the fuel load to assist in bringing up boost.
This simple check applies the extra fuel if we're currently launching
*/
TESTABLE_INLINE_STATIC byte correctionLaunch(void)
{
  byte launchValue = NO_FUEL_CORRECTION;
  if(currentStatus.launchingHard || currentStatus.launchingSoft) { launchValue = (BASELINE_FUEL_CORRECTION + configPage6.lnchFuelAdd); }

  return launchValue;
}

/**
*/
TESTABLE_INLINE_STATIC byte correctionDFCOfuel(void)
{
  byte scaleValue = NO_FUEL_CORRECTION;
  if ( currentStatus.isDFCOActive )
  {
    if ( (configPage9.dfcoTaperEnable == 1) && (dfcoTaper != 0) )
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
  if ( configPage2.dfcoEnabled == 1 )
  {
    if ( currentStatus.isDFCOActive ) 
    {
      DFCOValue = ( currentStatus.RPM > ( configPage4.dfcoRPM * 10) ) && ( currentStatus.TPS < configPage4.dfcoTPSThresh ); 
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

/** Flex fuel adjustment to vary fuel based on ethanol content.
 * The amount of extra fuel required is a linear relationship based on the % of ethanol.
*/
TESTABLE_INLINE_STATIC byte correctionFlex(void)
{
  byte flexValue = NO_FUEL_CORRECTION;

  if (configPage2.flexEnabled == 1)
  {
    flexValue = table2D_getValue(&flexFuelTable, currentStatus.ethanolPct);
  }
  return flexValue;
}

/*
 * Fuel temperature adjustment to vary fuel based on fuel temperature reading
*/
TESTABLE_INLINE_STATIC byte correctionFuelTemp(void)
{
  byte fuelTempValue = NO_FUEL_CORRECTION;

  if (configPage2.flexEnabled == 1)
  {
    fuelTempValue = table2D_getValue(&fuelTempTable, temperatureAddOffset(currentStatus.fuelTemp));
  }
  return fuelTempValue;
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

/** Lookup the AFR target table and perform either a simple or PID adjustment based on this.

Simple (Best suited to narrowband sensors):
If the O2 sensor reports that the mixture is lean/rich compared to the desired AFR target, it will make a 1% adjustment
It then waits egoDelta number of ignition events and compares O2 against the target table again. If it is still lean/rich then the adjustment is increased to 2%.

This continues until either:
- the O2 reading flips from lean to rich, at which point the adjustment cycle starts again at 1% or
- the adjustment amount increases to egoLimit at which point it stays at this level until the O2 state (rich/lean) changes

PID (Best suited to wideband sensors):

*/
TESTABLE_INLINE_STATIC byte correctionAFRClosedLoop(void)
{
  byte AFRValue = NO_FUEL_CORRECTION;

  if((configPage6.egoType > 0) && ( !currentStatus.isDFCOActive ) ) //egoType of 0 means no O2 sensor. If DFCO is active do not run the ego controllers to prevent iterator wind-up.
  {
    AFRValue = currentStatus.egoCorrection; //Need to record this here, just to make sure the correction stays 'on' even if the nextCycle count isn't ready
    
    if(((uint16_t)(ignitionCount - AFRnextCycle)) < UINT16_HALF_RANGE) //Check whether ignitionCount has exceeded AFRnextCycle. This method prevents any issues when AFRnextCycle overflows but these variables cannot be more than UINT16_HALF_RANGE apart
    {
      AFRnextCycle = ignitionCount + configPage6.egoCount; //Set the target ignition event for the next calculation
        
      //Check all other requirements for closed loop adjustments
      if(    (currentStatus.coolant > temperatureRemoveOffset(configPage6.egoTemp)) 
          && (currentStatus.RPM > (configPage6.egoRPM * 100U)) 
          && (currentStatus.TPS <= configPage6.egoTPSMax) 
          && (currentStatus.O2 < configPage6.ego_max) 
          && (currentStatus.O2 > configPage6.ego_min)
          && (currentStatus.runSecs > configPage6.ego_sdelay) 
          && (currentStatus.MAP <= (long)(configPage9.egoMAPMax * 2U)) 
          && (currentStatus.MAP >= (long)(configPage9.egoMAPMin * 2U)) )
      {
        //Check which algorithm is used, simple or PID
        if (configPage6.egoAlgorithm == EGO_ALGORITHM_SIMPLE)
        {
          //*************************************************************************************************************************************
          //Simple algorithm
          if(currentStatus.O2 > currentStatus.afrTarget)
          {
            //Running lean
            if(currentStatus.egoCorrection < (BASELINE_FUEL_CORRECTION + configPage6.egoLimit) ) //Fuelling adjustment must be at most the egoLimit amount (up or down)
            {
              AFRValue = (currentStatus.egoCorrection + 1); //Increase the fuelling by 1%
            }
            else { AFRValue = currentStatus.egoCorrection; } //Means we're at the maximum adjustment amount, so simply return that again
          }
          else if(currentStatus.O2 < currentStatus.afrTarget)
          {
            //Running Rich
            if(currentStatus.egoCorrection > (BASELINE_FUEL_CORRECTION - configPage6.egoLimit) ) //Fuelling adjustment must be at most the egoLimit amount (up or down)
            {
              AFRValue = (currentStatus.egoCorrection - 1); //Decrease the fuelling by 1%
            }
            else { AFRValue = currentStatus.egoCorrection; } //Means we're at the maximum adjustment amount, so simply return that again
          }
          else { AFRValue = currentStatus.egoCorrection; } //Means we're already right on target

        }
        else if(configPage6.egoAlgorithm == EGO_ALGORITHM_PID)
        {
          //*************************************************************************************************************************************
          //PID algorithm
          egoPID.SetOutputLimits((long)(-configPage6.egoLimit), (long)(configPage6.egoLimit)); //Set the limits again, just in case the user has changed them since the last loop. Note that these are sent to the PID library as (Eg:) -15 and +15
          egoPID.SetTunings(configPage6.egoKP, configPage6.egoKI, configPage6.egoKD); //Set the PID values again, just in case the user has changed them since the last loop
          PID_O2 = (long)(currentStatus.O2);
          PID_AFRTarget = (long)(currentStatus.afrTarget);

          bool PID_compute = egoPID.Compute();
          if(PID_compute == true) { AFRValue = BASELINE_FUEL_CORRECTION + PID_output; }
          
        }
        else { AFRValue = NO_FUEL_CORRECTION; } // Occurs if the egoAlgorithm is set to 0 (No Correction)
      } //Multi variable check 
      else { AFRValue = NO_FUEL_CORRECTION; } // If multivariable check fails disable correction
    } //Ignition count check
  } //egoType

  //Final check to ensure within authority range (This can be needed if the user has lowered the authority limit)
  if(AFRValue < (100U - configPage6.egoLimit)) {AFRValue = (100U - configPage6.egoLimit); }
  if(AFRValue > (100U + configPage6.egoLimit)) {AFRValue = (100U + configPage6.egoLimit); }

  return AFRValue; //Catch all (Includes when AFR target = current AFR
}


/** Dispatch calculations for all fuel related corrections.
Calls all the other corrections functions and combines their results.
This is the only function that should be called from anywhere outside the file
*/
uint16_t correctionsFuel(void)
{
  uint32_t sumCorrections = NO_FUEL_CORRECTION;
  uint16_t result; //temporary variable to store the result of each corrections function

  //The values returned by each of the correction functions are multiplied together and then divided back to give a single 0-255 value.
  currentStatus.wueCorrection = correctionWUE();
  if (currentStatus.wueCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.wueCorrection); }

  currentStatus.ASEValue = correctionASE();
  if (currentStatus.ASEValue != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.ASEValue); }

  result = correctionCranking();
  if (result != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * result); }

  currentStatus.AEamount = correctionAccel();
  if ( (configPage2.aeApplyMode == AE_MODE_MULTIPLIER) || (currentStatus.isDeceleratingTPS) ) // multiply by the AE amount in case of multiplier AE mode or Decel
  {
    if (currentStatus.AEamount != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.AEamount);}
  }

  result = correctionFloodClear();
  if (result != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * result); }

  currentStatus.egoCorrection = correctionAFRClosedLoop();
  if (currentStatus.egoCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.egoCorrection); }

  //Voltage correction is applied to the injector opening time
  currentStatus.batCorrection = correctionBatVoltage();

  currentStatus.iatCorrection = correctionIATDensity();
  if (currentStatus.iatCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.iatCorrection); }

  currentStatus.baroCorrection = correctionBaro();
  if (currentStatus.baroCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.baroCorrection); }

  currentStatus.flexCorrection = correctionFlex();
  if (currentStatus.flexCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.flexCorrection); }

  currentStatus.fuelTempCorrection = correctionFuelTemp();
  if (currentStatus.fuelTempCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.fuelTempCorrection); }

  currentStatus.launchCorrection = correctionLaunch();
  if (currentStatus.launchCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * currentStatus.launchCorrection); }

  currentStatus.isDFCOActive = correctionDFCO();
  byte dfcoTaperCorrection = correctionDFCOfuel();
  if (dfcoTaperCorrection == 0) { sumCorrections = 0; }
  else if (dfcoTaperCorrection != NO_FUEL_CORRECTION) { sumCorrections = div100(sumCorrections * dfcoTaperCorrection); }

  if(sumCorrections > 1500) { sumCorrections = 1500; } //This is the maximum allowable increase during cranking
  return (uint16_t)sumCorrections;
}

//******************************** IGNITION ADVANCE CORRECTIONS ********************************

/** Correct ignition timing to configured fixed value.
 * Must be called near end to override all other corrections.
 */
int8_t correctionFixedTiming(int8_t advance)
{
  int8_t ignFixValue = advance;
  if (configPage2.fixAngEnable == 1) { ignFixValue = configPage4.FixAng; } //Check whether the user has set a fixed timing angle
  return ignFixValue;
}

/** Ignition correction for coolant temperature (CLT).
 */
TESTABLE_INLINE_STATIC int8_t correctionCLTadvance(int8_t advance)
{
  int8_t ignCLTValue = advance;
  //Adjust the advance based on CLT.
  int8_t advanceCLTadjust = (int16_t)(table2D_getValue(&CLTAdvanceTable, temperatureAddOffset(currentStatus.coolant))) - 15;
  ignCLTValue = (advance + advanceCLTadjust);
  
  return ignCLTValue;
}

/** Correct ignition timing to configured fixed value to use during craning.
 * Must be called near end to override all other corrections.
 */
int8_t correctionCrankingFixedTiming(int8_t advance)
{
  int8_t ignCrankFixValue = advance;
  if ( currentStatus.engineIsCranking )
  { 
    if ( configPage2.crkngAddCLTAdv == 0 ) { ignCrankFixValue = configPage4.CrankAng; } //Use the fixed cranking ignition angle
    else { ignCrankFixValue = correctionCLTadvance(configPage4.CrankAng); } //Use the CLT compensated cranking ignition angle
  }
  return ignCrankFixValue;
}

TESTABLE_INLINE_STATIC int8_t correctionFlexTiming(int8_t advance)
{
  int16_t ignFlexValue = advance;
  if( configPage2.flexEnabled == 1 ) //Check for flex being enabled
  {
    ignFlexValue = (int16_t) table2D_getValue(&flexAdvTable, currentStatus.ethanolPct) - OFFSET_IGNITION; //Negative values are achieved with offset
    currentStatus.flexIgnCorrection = (int8_t) ignFlexValue; //This gets cast to a signed 8 bit value to allows for negative advance (ie retard) values here. 
    ignFlexValue = (int8_t) advance + currentStatus.flexIgnCorrection;
  }
  return (int8_t) ignFlexValue;
}

TESTABLE_INLINE_STATIC int8_t correctionWMITiming(int8_t advance)
{
  if( (configPage10.wmiEnabled >= 1) && (configPage10.wmiAdvEnabled == 1) && (!currentStatus.wmiTankEmpty) ) //Check for wmi being enabled
  {
    if( (currentStatus.TPS >= configPage10.wmiTPS) && (currentStatus.RPM >= configPage10.wmiRPM) && (currentStatus.MAP/2 >= configPage10.wmiMAP) && (temperatureAddOffset(currentStatus.IAT) >= configPage10.wmiIAT) )
    {
      return advance + (int8_t)table2D_getValue(&wmiAdvTable, (uint8_t)((uint16_t)currentStatus.MAP/2U)) - OFFSET_IGNITION; //Negative values are achieved with offset
    }
  }
  return advance;
}
/** Ignition correction for inlet air temperature (IAT).
 */
TESTABLE_INLINE_STATIC int8_t correctionIATretard(int8_t advance)
{
  int8_t advanceIATadjust = table2D_getValue(&IATRetardTable, (uint8_t)currentStatus.IAT);

  return advance - advanceIATadjust;
}

/** Ignition Idle advance correction.
 */
#define IGN_IDLE_THRESHOLD 200 //RPM threshold (below CL idle target) for when ign based idle control will engage

TESTABLE_INLINE_STATIC int8_t correctionIdleAdvance(int8_t advance)
{

  int8_t ignIdleValue = advance;
  //Adjust the advance based on idle target rpm.
  if( (configPage2.idleAdvEnabled >= 1) && (runSecsX10 >= (configPage2.idleAdvDelay * 5)) && idleAdvActive)
  {
    //currentStatus.CLIdleTarget = (byte)table2D_getValue(&idleTargetTable, temperatureAddOffset(currentStatus.coolant)); //All temps are offset by 40 degrees
    int idleRPMdelta = (currentStatus.CLIdleTarget - (currentStatus.RPM / 10) ) + 50;
    // Limit idle rpm delta between -500rpm - 500rpm
    if(idleRPMdelta > 100) { idleRPMdelta = 100; }
    if(idleRPMdelta < 0) { idleRPMdelta = 0; }
    if( (currentStatus.RPM < (configPage2.idleAdvRPM * 100)) && ((configPage2.vssMode == 0) || (currentStatus.vss < configPage2.idleAdvVss))
    && (((configPage2.idleAdvAlgorithm == 0) && (currentStatus.TPS < configPage2.idleAdvTPS)) || ((configPage2.idleAdvAlgorithm == 1) && (currentStatus.CTPSActive == 1))) ) // closed throttle position sensor (CTPS) based idle state
    {
      if( idleAdvTaper < configPage9.idleAdvStartDelay )
      {
        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { idleAdvTaper++; }
      }
      else
      {
        int8_t advanceIdleAdjust = (int8_t)table2D_getValue(&idleAdvanceTable, (uint8_t)idleRPMdelta) - 15;
        if(configPage2.idleAdvEnabled == 1) { ignIdleValue = (advance + advanceIdleAdjust); }
        else if(configPage2.idleAdvEnabled == 2) { ignIdleValue = advanceIdleAdjust; }
      }
    }
    else { idleAdvTaper = 0; }
  }

/* When Idle advance is the only idle speed control mechanism, activate as soon as not cranking. 
When some other mechanism is also present, wait until the engine is no more than 200 RPM below idle target speed on first time
*/

  if ((!idleAdvActive && currentStatus.engineIsRunning) &&
   ((configPage6.iacAlgorithm == 0) || (currentStatus.RPM > (((uint16_t)currentStatus.CLIdleTarget * 10) - (uint16_t)IGN_IDLE_THRESHOLD))))
  { 
    idleAdvActive = true; 
  } 
  else 
    if (idleAdvActive && !currentStatus.engineIsRunning) { idleAdvActive = false; } //Clear flag if engine isn't running anymore

  return ignIdleValue;
}
/** Ignition soft revlimit correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftRevLimit(int8_t advance)
{
  byte ignSoftRevValue = advance;
  currentStatus.softLimitActive = false;

  if (configPage6.engineProtectType == PROTECT_CUT_IGN || configPage6.engineProtectType == PROTECT_CUT_BOTH) 
  {
    if (currentStatus.RPMdiv100 >= configPage4.SoftRevLim) //Softcut RPM limit
    {
      currentStatus.softLimitActive = true;
      if( softLimitTime < configPage4.SoftLimMax )
      {
        if (configPage2.SoftLimitMode == SOFT_LIMIT_RELATIVE) { ignSoftRevValue = ignSoftRevValue - configPage4.SoftLimRetard; } //delay timing by configured number of degrees in relative mode
        else if (configPage2.SoftLimitMode == SOFT_LIMIT_FIXED) { ignSoftRevValue = configPage4.SoftLimRetard; } //delay timing to configured number of degrees in fixed mode

        if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { softLimitTime++; }
      }
    }
    else if( BIT_CHECK(LOOP_TIMER, BIT_TIMER_10HZ) ) { softLimitTime = 0; } //Only reset time at runSecsX10 update rate
  }

  return ignSoftRevValue;
}
/** Ignition Nitrous oxide correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionNitrous(int8_t advance)
{
  byte ignNitrous = advance;
  //Check if nitrous is currently active
  if(configPage10.n2o_enable > 0)
  {
    //Check which stage is running (if any)
    if( (currentStatus.nitrous_status == NITROUS_STAGE1) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      ignNitrous -= configPage10.n2o_stage1_retard;
    }
    if( (currentStatus.nitrous_status == NITROUS_STAGE2) || (currentStatus.nitrous_status == NITROUS_BOTH) )
    {
      ignNitrous -= configPage10.n2o_stage2_retard;
    }
  }

  return ignNitrous;
}
/** Ignition soft launch correction.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftLaunch(int8_t advance)
{
  uint8_t ignSoftLaunchValue = advance;
  //SoftCut rev limit for 2-step launch control.
  if(  configPage6.launchEnabled && currentStatus.clutchTrigger && \
      (currentStatus.clutchEngagedRPM < ((unsigned int)(configPage6.flatSArm) * 100)) && \
      (currentStatus.RPM > ((unsigned int)(configPage6.lnchSoftLim) * 100)) && \
      (currentStatus.TPS >= configPage10.lnchCtrlTPS) && \
      ( (configPage2.vssMode == 0) || ((configPage2.vssMode > 0) && (currentStatus.vss <= configPage10.lnchCtrlVss)) ) \
    )
  {
    currentStatus.launchingSoft = true;
    currentStatus.softLaunchActive = true;
    ignSoftLaunchValue = configPage6.lnchRetard;
  }
  else
  {
    currentStatus.launchingSoft = false;
    currentStatus.softLaunchActive = false;
  }

  return ignSoftLaunchValue;
}
/** Ignition correction for soft flat shift.
 */
TESTABLE_INLINE_STATIC int8_t correctionSoftFlatShift(int8_t advance)
{
  int8_t ignSoftFlatValue = advance;

  if(configPage6.flatSEnable && currentStatus.clutchTrigger && (currentStatus.clutchEngagedRPM > ((unsigned int)(configPage6.flatSArm) * 100)) && (currentStatus.RPM > (currentStatus.clutchEngagedRPM - (configPage6.flatSSoftWin * 100) ) ) )
  {
    currentStatus.flatShiftSoftCut = true;
    ignSoftFlatValue = configPage6.flatSRetard;
  }
  else { currentStatus.flatShiftSoftCut = false; }

  return ignSoftFlatValue;
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
  int8_t dfcoRetard = advance;
  if ( (configPage9.dfcoTaperEnable == 1) && currentStatus.isDFCOActive )
  {
    if ( dfcoTaper != 0 )
    {
      dfcoRetard -= map(dfcoTaper, configPage9.dfcoTaperTime, 0, 0, configPage9.dfcoTaperAdvance);
    }
    else { dfcoRetard -= configPage9.dfcoTaperAdvance; } //Taper ended, use full value
  }
  else { dfcoTaper = configPage9.dfcoTaperTime; } //Keep updating the duration until DFCO is active
  return dfcoRetard;
}

/** Ignition Dwell Correction.
 */
uint16_t correctionsDwell(uint16_t dwell)
{
  uint16_t tempDwell = dwell;
  uint16_t sparkDur_uS = (configPage4.sparkDur * 100); //Spark duration is in mS*10. Multiple it by 100 to get spark duration in uS
  if(currentStatus.actualDwell == 0) { currentStatus.actualDwell = tempDwell; } //Initialise the actualDwell value if this is the first time being called

  //**************************************************************************************************************************
  //Pull battery voltage based dwell correction and apply if needed
  currentStatus.dwellCorrection = table2D_getValue(&dwellVCorrectionTable, currentStatus.battery10);
  if (currentStatus.dwellCorrection != 100) { tempDwell = div100(dwell) * currentStatus.dwellCorrection; }


  //**************************************************************************************************************************
  //Dwell error correction is a basic closed loop to keep the dwell time consistent even when adjusting its end time for the per tooth timing.
  //This is mostly of benefit to low resolution triggers at low rpm (<1500)
  if( (configPage2.perToothIgn  == true) && (configPage4.dwellErrCorrect == 1) )
  {
    int16_t error = tempDwell - currentStatus.actualDwell;
    if(tempDwell > INT16_MAX) { tempDwell = INT16_MAX; } //Prevent overflow when casting to signed int
    if(error > ((int16_t)tempDwell / 2)) { error += error; } //Double correction amount if actual dwell is less than 50% of the requested dwell
    if(error > 0) { tempDwell += error; }
  }

  //**************************************************************************************************************************
  /*
  Dwell limiter - If the total required dwell time per revolution is longer than the maximum time available at the current RPM, reduce dwell. This can occur if there are multiple sparks per revolution
  This only times this can occur are:
  1. Single channel spark mode where there will be nCylinders/2 sparks per revolution
  2. Rotary ignition in wasted spark configuration (FC/FD), results in 2 pulses per rev. RX-8 is fully sequential resulting in 1 pulse, so not required
  */
  uint16_t dwellPerRevolution = tempDwell + sparkDur_uS;
  int8_t pulsesPerRevolution = 1;
  if( ( (configPage4.sparkMode == IGN_MODE_SINGLE) || ((configPage4.sparkMode == IGN_MODE_ROTARY) && (configPage10.rotaryType != ROTARY_IGN_RX8)) ) && (configPage2.nCylinders > 1) ) //No point in running this for 1 cylinder engines
  {
    pulsesPerRevolution = (configPage2.nCylinders >> 1);
    dwellPerRevolution = dwellPerRevolution * pulsesPerRevolution;
  }
  if(dwellPerRevolution > currentStatus.revolutionTime)
  {
    //Possibly need some method of reducing spark duration here as well, but this is a start
    uint16_t adjustedSparkDur = fast_div32_16(sparkDur_uS * currentStatus.revolutionTime, dwellPerRevolution);
    tempDwell = fast_div32_16(currentStatus.revolutionTime, (uint16_t)pulsesPerRevolution) - adjustedSparkDur;
  }

  return tempDwell;
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