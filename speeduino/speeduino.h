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
uint8_t getVE1(void);
int8_t getAdvance1(void);
void calculateIgnitionAngles(uint16_t dwellAngle);
void checkLaunchAndFlatShift();

extern uint16_t inj_opentime_uS; /**< The injector opening time. This is set within Tuner Studio, but stored here in uS rather than mS */

#endif
