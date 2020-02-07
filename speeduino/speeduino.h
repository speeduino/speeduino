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
//#include "globals.h"

uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen);
byte getVE1();
byte getVE2();
byte getAdvance();

uint16_t calculateInjector2StartAngle(unsigned int);
uint16_t calculateInjector3StartAngle(unsigned int);
uint16_t calculateInjector4StartAngle(unsigned int);
uint16_t calculateInjector5StartAngle(unsigned int);

/*
struct config2 configPage2;
struct config4 configPage4;
struct config6 configPage6;
struct config9 configPage9;
struct config10 configPage10;
*/

extern uint16_t req_fuel_uS; /**< The required fuel variable (As calculated by TunerStudio) in uS */
extern uint16_t inj_opentime_uS; /**< The injector opening time. This is set within Tuner Studio, but stored here in uS rather than mS */

extern bool ignitionOn; /**< The current state of the ignition system (on or off) */
extern bool fuelOn; /**< The current state of the fuel system (on or off) */

extern byte maxIgnOutputs; /**< Used for rolling rev limiter to indicate how many total ignition channels should currently be firing */
extern byte curRollingCut; /**< Rolling rev limiter, current ignition channel being cut */
extern byte rollingCutCounter; /**< how many times (revolutions) the ignition has been cut in a row */
extern uint32_t rollingCutLastRev; /**< Tracks whether we're on the same or a different rev for the rolling cut */

extern int channel1IgnDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
extern int channel2IgnDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
extern int channel3IgnDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
extern int channel4IgnDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
extern int channel5IgnDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
extern int channel6IgnDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
extern int channel7IgnDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
extern int channel8IgnDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */
extern int channel1InjDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
extern int channel2InjDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
extern int channel3InjDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
extern int channel4InjDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
extern int channel5InjDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
extern int channel6InjDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
extern int channel7InjDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
extern int channel8InjDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */

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
extern uint16_t staged_req_fuel_mult_pri;
extern uint16_t staged_req_fuel_mult_sec;
///@}

/** @name IgnitionCallbacks
 * These are the function pointers that get called to begin and end the ignition coil charging. They are required for the various spark output modes
*/
///@{
extern void (*ign1StartFunction)();
extern void (*ign1EndFunction)();
extern void (*ign2StartFunction)();
extern void (*ign2EndFunction)();
extern void (*ign3StartFunction)();
extern void (*ign3EndFunction)();
extern void (*ign4StartFunction)();
extern void (*ign4EndFunction)();
extern void (*ign5StartFunction)();
extern void (*ign5EndFunction)();
extern void (*ign6StartFunction)();
extern void (*ign6EndFunction)();
extern void (*ign7StartFunction)();
extern void (*ign7EndFunction)();
extern void (*ign8StartFunction)();
extern void (*ign8EndFunction)();
///@}

#endif
