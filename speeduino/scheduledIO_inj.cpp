#include "globals.h"
#include "acc_mc33810.h"
#include "scheduledIO_direct_inj.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

 volatile byte injStatusMask = 0;

 /** @brief Injector open/close status bits */
char getInjectorStatus(void)
{
    return injStatusMask;
}

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control

#define OPEN_INJECTOR(channel) \
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { \
        openInjector ## channel ## _DIRECT(); \
    } else { \
        openInjector ## channel ## _MC33810(); \
    }; \
    BIT_SET(injStatusMask, (channel)-1U);

#define CLOSE_INJECTOR(channel) \
    if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { \
        closeInjector ## channel ## _DIRECT(); \
    } else { \
        closeInjector ## channel ## _MC33810(); \
    }; \
    BIT_CLEAR(injStatusMask, (channel)-1U); 

void openInjector1(void)   { OPEN_INJECTOR(1); }
void closeInjector1(void)  { CLOSE_INJECTOR(1); }
void openInjector2(void)   { OPEN_INJECTOR(2); }
void closeInjector2(void)  { CLOSE_INJECTOR(2); }
void openInjector3(void)   { OPEN_INJECTOR(3); }
void closeInjector3(void)  { CLOSE_INJECTOR(3); }
void openInjector4(void)   { OPEN_INJECTOR(4); }
void closeInjector4(void)  { CLOSE_INJECTOR(4); }
void openInjector5(void)   { OPEN_INJECTOR(5); }
void closeInjector5(void)  { CLOSE_INJECTOR(5); }
void openInjector6(void)   { OPEN_INJECTOR(6); }
void closeInjector6(void)  { CLOSE_INJECTOR(6); }
void openInjector7(void)   { OPEN_INJECTOR(7); }
void closeInjector7(void)  { CLOSE_INJECTOR(7); }
void openInjector8(void)   { OPEN_INJECTOR(8); }
void closeInjector8(void)  { CLOSE_INJECTOR(8); }

// These are for Semi-Sequential and 5 Cylinder injection
//Standard 4 cylinder pairings
void openInjector1and3(void) { openInjector1(); openInjector3(); }
void closeInjector1and3(void) { closeInjector1(); closeInjector3(); }
void openInjector2and4(void) { openInjector2(); openInjector4(); }
void closeInjector2and4(void) { closeInjector2(); closeInjector4(); }
//Alternative output pairings
void openInjector1and4(void) { openInjector1(); openInjector4(); }
void closeInjector1and4(void) { closeInjector1(); closeInjector4(); }
void openInjector2and3(void) { openInjector2(); openInjector3(); }
void closeInjector2and3(void) { closeInjector2(); closeInjector3(); }

void openInjector3and5(void) { openInjector3(); openInjector5(); }
void closeInjector3and5(void) { closeInjector3(); closeInjector5(); }

void openInjector2and5(void) { openInjector2(); openInjector5(); }
void closeInjector2and5(void) { closeInjector2(); closeInjector5(); }
void openInjector3and6(void) { openInjector3(); openInjector6(); }
void closeInjector3and6(void) { closeInjector3(); closeInjector6(); }

void openInjector1and5(void) { openInjector1(); openInjector5(); }
void closeInjector1and5(void) { closeInjector1(); closeInjector5(); }
void openInjector2and6(void) { openInjector2(); openInjector6(); }
void closeInjector2and6(void) { closeInjector2(); closeInjector6(); }
void openInjector3and7(void) { openInjector3(); openInjector7(); }
void closeInjector3and7(void) { closeInjector3(); closeInjector7(); }
void openInjector4and8(void) { openInjector4(); openInjector8(); }
void closeInjector4and8(void) { closeInjector4(); closeInjector8(); }

// LCOV_EXCL_STOP