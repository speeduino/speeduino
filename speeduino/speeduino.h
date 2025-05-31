/** \file speeduino.h
 * @brief Speeduino main file containing initial setup and system loop functions 
 * @author Josh Stewart
 * 
 * This file contains the main system loop of the Speeduino core and thus much of the logic of the fuel and ignition algorithms is contained within this
 * It is where calls to all the auxiliary control systems, sensor reads, comms etc are made
 * 
 * It also contains the setup() function that is called by the bootloader on system startup
 * 
 */

#ifndef SPEEDUINO_H
#define SPEEDUINO_H
//#include "globals.h"

#define CRANK_RUN_HYSTER    15

void setup(void);
void loop(void);
uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen);
uint8_t getVE1(void);
int8_t getAdvance1(void);
uint16_t calculatePWLimit();
void calculateStaging(uint32_t);
void calculateIgnitionAngles(uint16_t dwellAngle);
void checkLaunchAndFlatShift();

extern uint16_t req_fuel_uS; /**< The required fuel variable (As calculated by TunerStudio) in uS */
extern uint16_t inj_opentime_uS; /**< The injector opening time. This is set within Tuner Studio, but stored here in uS rather than mS */

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

// This should be declared in speeduino.ino, but the Arduino INO file mangling 
// process *removes it* from speeduino.ino thus causing a compiler error (PlatformIO 
// works correctly)
struct InjectorStartAngles {
  uint16_t injector1 = 0;
  uint16_t injector2 = 0;
  uint16_t injector3 = 0;
  uint16_t injector4 = 0;

  #if INJ_CHANNELS >= 5
  uint16_t injector5 = 0;
  #endif
  #if INJ_CHANNELS >= 6
  uint16_t injector6 = 0;
  #endif
  #if INJ_CHANNELS >= 7
  uint16_t injector7 = 0;
  #endif
  #if INJ_CHANNELS >= 8
  uint16_t injector8 = 0;
  #endif
};

#endif
