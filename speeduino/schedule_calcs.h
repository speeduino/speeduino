#pragma once

#include <stdint.h>
#include "scheduler.h"

extern int ignition1StartAngle;
extern int ignition2StartAngle;
extern int ignition3StartAngle;
extern int ignition4StartAngle;
extern int ignition5StartAngle;
extern int ignition6StartAngle;
extern int ignition7StartAngle;
extern int ignition8StartAngle;

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

uint16_t calculateInjectorStartAngle(uint16_t PWdivTimerPerDegree, int16_t injChannelDegrees);

uint32_t calculateInjector1Timeout(int injector1StartAngle, int crankAngle);
uint32_t calculateInjectorNTimeout(const FuelSchedule &schedule, int channelInjDegrees, int injectorStartAngle, int crankAngle);

void calculateIgnitionAngle1(int dwellAngle);
void calculateIgnitionAngle2(int dwellAngle);
void calculateIgnitionAngle3(int dwellAngle);
// ignition 3 for rotary
void calculateIgnitionAngle3(int dwellAngle, int rotarySplitDegrees);
void calculateIgnitionAngle4(int dwellAngle);
// ignition 4 for rotary
void calculateIgnitionAngle4(int dwellAngle, int rotarySplitDegrees);
void calculateIgnitionAngle5(int dwellAngle);
void calculateIgnitionAngle6(int dwellAngle);
void calculateIgnitionAngle7(int dwellAngle);
void calculateIgnitionAngle8(int dwellAngle);

uint32_t calculateIgnition1Timeout(int crankAngle);
uint32_t calculateIgnitionNTimeout(const Schedule &schedule, int startAngle, int channelIgnDegrees, int crankAngle);
