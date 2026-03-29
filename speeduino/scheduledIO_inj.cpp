#include "globals.h"
#include "acc_mc33810.h"
#include "scheduledIO_direct_inj.h"

/** @file
 * Injector and Coil (toggle/open/close) control (under various situations, eg with particular cylinder count, rotary engine type or wasted spark ign, etc.).
 * Also accounts for presence of MC33810 injector/ignition (dwell, etc.) control circuit.
 * Functions here are typically assigned (at initialisation) to callback function variables (e.g. inj1StartFunction or inj1EndFunction) 
 * form where they are called (by scheduler.ino).
 */

 // LCOV_EXCL_START
 // Exclude from code coverage, since this is all board output control

void openInjector1(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector1_DIRECT(); }   else { openInjector1_MC33810(); } }
void closeInjector1(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector1_DIRECT(); }  else { closeInjector1_MC33810(); } }
void openInjector2(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector2_DIRECT(); }   else { openInjector2_MC33810(); } }
void closeInjector2(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector2_DIRECT(); }  else { closeInjector2_MC33810(); } }
void openInjector3(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector3_DIRECT(); }   else { openInjector3_MC33810(); } }
void closeInjector3(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector3_DIRECT(); }  else { closeInjector3_MC33810(); } }
void openInjector4(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector4_DIRECT(); }   else { openInjector4_MC33810(); } }
void closeInjector4(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector4_DIRECT(); }  else { closeInjector4_MC33810(); } }
void openInjector5(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector5_DIRECT(); }   else { openInjector5_MC33810(); } }
void closeInjector5(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector5_DIRECT(); }  else { closeInjector5_MC33810(); } }
void openInjector6(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector6_DIRECT(); }   else { openInjector6_MC33810(); } }
void closeInjector6(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector6_DIRECT(); }  else { closeInjector6_MC33810(); } }
void openInjector7(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector7_DIRECT(); }   else { openInjector7_MC33810(); } }
void closeInjector7(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector7_DIRECT(); }  else { closeInjector7_MC33810(); } }
void openInjector8(void)   { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { openInjector8_DIRECT(); }   else { openInjector8_MC33810(); } }
void closeInjector8(void)  { if(injectorOutputControl != OUTPUT_CONTROL_MC33810) { closeInjector8_DIRECT(); }  else { closeInjector8_MC33810(); } }

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