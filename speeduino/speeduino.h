/** \file speeduino.h
 * @brief Speeduino main file containing initial setup and system loop functions 
 * @author Josh Stewart
 * 
 * This file contains the main system loop of the Speeduino core and thus much of the logic of the fuel and ignition algorithms is contained within this
 * It is where calls to all the auxilliary control systems, sensor reads, comms etc are made
 * 
 * It also contains the setup() function that is called by the bootloader on system startup
 * 
 */

#ifndef SPEEDUINO_H
#define SPEEDUINO_H

uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen);
byte getVE1();
byte getVE2();
byte getAdvance();

uint16_t calculateInjector2StartAngle(unsigned int);
uint16_t calculateInjector3StartAngle(unsigned int);
uint16_t calculateInjector4StartAngle(unsigned int);
uint16_t calculateInjector5StartAngle(unsigned int);

struct config2 configPage2;
struct config4 configPage4;
struct config6 configPage6;
struct config9 configPage9;
struct config10 configPage10;

uint16_t req_fuel_uS; /**< The required fuel variable (As calculated by TunerStudio) in uS */
uint16_t inj_opentime_uS; /**< The injector opening time. This is set within Tuner Studio, but stored here in uS rather than mS */

bool ignitionOn = false; /**< The current state of the ignition system (on or off) */
bool fuelOn = false; /**< The current state of the fuel system (on or off) */

byte cltCalibrationTable[CALIBRATION_TABLE_SIZE]; /**< An array containing the coolant sensor calibration values */
byte iatCalibrationTable[CALIBRATION_TABLE_SIZE]; /**< An array containing the inlet air temperature sensor calibration values */
byte o2CalibrationTable[CALIBRATION_TABLE_SIZE]; /**< An array containing the O2 sensor calibration values */

unsigned long currentLoopTime; /**< The time the current loop started (uS) */
unsigned long previousLoopTime; /**< The time the previous loop started (uS) */

byte maxIgnOutputs = 1; /**< Used for rolling rev limiter to indicate how many total ignition channels should currently be firing */
byte curRollingCut = 0; /**< Rolling rev limiter, current ignition channel being cut */
byte rollingCutCounter = 0; /**< how many times (revolutions) the ignition has been cut in a row */
uint32_t rollingCutLastRev = 0; /**< Tracks whether we're on the same or a different rev for the rolling cut */

int channel1IgnDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
int channel2IgnDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
int channel3IgnDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
int channel4IgnDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
int channel5IgnDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
int channel6IgnDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
int channel7IgnDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
int channel8IgnDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */
int channel1InjDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
int channel2InjDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
int channel3InjDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
int channel4InjDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
int channel5InjDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
int channel6InjDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
int channel7InjDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
int channel8InjDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */

/** @name Staging
 * These values are a percentage of the total (Combined) req_fuel value that would be required for each injector channel to deliver that much fuel.   
 * 
 * Eg:
 *  - Pri injectors are 250cc
 *  - Sec injectors are 500cc
 *  - Total injector capacity = 750cc
 * 
 *  - staged_req_fuel_mult_pri = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
 *  - staged_req_fuel_mult_sec = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
*/
///@{
uint16_t staged_req_fuel_mult_pri;
uint16_t staged_req_fuel_mult_sec;
///@}

/** @name IgnitionCallbacks
 * These are the function pointers that get called to begin and end the ignition coil charging. They are required for the various spark output modes
*/
///@{
void (*ign1StartFunction)();
void (*ign1EndFunction)();
void (*ign2StartFunction)();
void (*ign2EndFunction)();
void (*ign3StartFunction)();
void (*ign3EndFunction)();
void (*ign4StartFunction)();
void (*ign4EndFunction)();
void (*ign5StartFunction)();
void (*ign5EndFunction)();
void (*ign6StartFunction)();
void (*ign6EndFunction)();
void (*ign7StartFunction)();
void (*ign7EndFunction)();
void (*ign8StartFunction)();
void (*ign8EndFunction)();
///@}

#endif
