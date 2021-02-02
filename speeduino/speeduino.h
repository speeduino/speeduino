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

uint16_t PW(int16_t REQ_FUEL, uint8_t VE, long MAP, uint16_t corrections, int16_t injOpen);
uint8_t getVE1();
uint8_t getAdvance1();

uint16_t calculateInjectorStartAngle(uint16_t, int16_t);
void calculateIgnitionAngle1(int16_t);
void calculateIgnitionAngle2(int16_t);
void calculateIgnitionAngle3(int16_t);
void calculateIgnitionAngle3(int16_t, int16_t);
void calculateIgnitionAngle4(int16_t);
void calculateIgnitionAngle4(int16_t, int16_t);
void calculateIgnitionAngle5(int16_t);
void calculateIgnitionAngle6(int16_t);
void calculateIgnitionAngle7(int16_t);
void calculateIgnitionAngle8(int16_t);
void calculateIgnitionAngles(int16_t);

extern uint16_t req_fuel_uS; /**< The required fuel variable (As calculated by TunerStudio) in uS */
extern uint16_t inj_opentime_uS; /**< The injector opening time. This is set within Tuner Studio, but stored here in uS rather than mS */

extern bool ignitionOn; /**< The current state of the ignition system (on or off) */
extern bool fuelOn; /**< The current state of the fuel system (on or off) */

extern uint8_t maxIgnOutputs; /**< Used for rolling rev limiter to indicate how many total ignition channels should currently be firing */
extern uint8_t curRollingCut; /**< Rolling rev limiter, current ignition channel being cut */
extern uint8_t rollingCutCounter; /**< how many times (revolutions) the ignition has been cut in a row */
extern uint32_t rollingCutLastRev; /**< Tracks whether we're on the same or a different rev for the rolling cut */

extern int16_t channel1IgnDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
extern int16_t channel2IgnDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
extern int16_t channel3IgnDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
extern int16_t channel4IgnDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
extern int16_t channel5IgnDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
extern int16_t channel6IgnDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
extern int16_t channel7IgnDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
extern int16_t channel8IgnDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */
extern int16_t channel1InjDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
extern int16_t channel2InjDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
extern int16_t channel3InjDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
extern int16_t channel4InjDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
extern int16_t channel5InjDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
extern int16_t channel6InjDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
extern int16_t channel7InjDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
extern int16_t channel8InjDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */

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



#endif
