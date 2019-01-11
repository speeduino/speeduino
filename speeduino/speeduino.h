#ifndef SPEEDUINO_H
#define SPEEDUINO_H

uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen);
byte getVE();
byte getAdvance();

uint16_t calculateInjector2StartAngle(unsigned int);
uint16_t calculateInjector3StartAngle(unsigned int);
uint16_t calculateInjector4StartAngle(unsigned int);
uint16_t calculateInjector4StartAngle(unsigned int);

struct config2 configPage2;
struct config4 configPage4; //Done
struct config6 configPage6;
struct config9 configPage9;
struct config10 configPage10;

uint16_t req_fuel_uS, inj_opentime_uS;
uint16_t staged_req_fuel_mult_pri;
uint16_t staged_req_fuel_mult_sec;

bool ignitionOn = false; //The current state of the ignition system
bool fuelOn = false; //The current state of the ignition system

byte cltCalibrationTable[CALIBRATION_TABLE_SIZE];
byte iatCalibrationTable[CALIBRATION_TABLE_SIZE];
byte o2CalibrationTable[CALIBRATION_TABLE_SIZE];

unsigned long counter;
unsigned long currentLoopTime; //The time the current loop started (uS)
unsigned long previousLoopTime; //The time the previous loop started (uS)

byte maxIgnOutputs = 1; //Used for rolling rev limiter
byte curRollingCut = 0; //Rolling rev limiter, current ignition channel being cut
byte rollingCutCounter = 0; //how many times (revolutions) the ignition has been cut in a row
uint32_t rollingCutLastRev = 0; //Tracks whether we're on the same or a different rev for the rolling cut


unsigned long secCounter; //The next time to incremen 'runSecs' counter.
int channel1IgnDegrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2IgnDegrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3IgnDegrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4IgnDegrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC
int channel5IgnDegrees; //The number of crank degrees until cylinder 5 is at TDC
int channel6IgnDegrees; //The number of crank degrees until cylinder 6 is at TDC
int channel7IgnDegrees; //The number of crank degrees until cylinder 7 is at TDC
int channel8IgnDegrees; //The number of crank degrees until cylinder 8 is at TDC
int channel1InjDegrees; //The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones)
int channel2InjDegrees; //The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC
int channel3InjDegrees; //The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC
int channel4InjDegrees; //The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC
int channel5InjDegrees; //The number of crank degrees until cylinder 5 is at TDC
int channel6InjDegrees; //The number of crank degrees until cylinder 6 is at TDC
int channel7InjDegrees; //The number of crank degrees until cylinder 7 is at TDC
int channel8InjDegrees; //The number of crank degrees until cylinder 8 is at TDC

//These are the functions the get called to begin and end the ignition coil charging. They are required for the various spark output modes
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

#endif
