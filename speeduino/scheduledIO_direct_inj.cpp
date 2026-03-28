#include "globals.h"

//Macros are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT macros (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file
void openInjector1_DIRECT(void)  { *inj1_pin_port |= (inj1_pin_mask); currentStatus.isInj1Open = true; }
void closeInjector1_DIRECT(void) { *inj1_pin_port &= ~(inj1_pin_mask);  currentStatus.isInj1Open = false; }
void openInjector2_DIRECT(void)  { *inj2_pin_port |= (inj2_pin_mask); currentStatus.isInj2Open = true; }
void closeInjector2_DIRECT(void) { *inj2_pin_port &= ~(inj2_pin_mask);  currentStatus.isInj2Open = false; }
void openInjector3_DIRECT(void)  { *inj3_pin_port |= (inj3_pin_mask); currentStatus.isInj3Open = true; }
void closeInjector3_DIRECT(void) { *inj3_pin_port &= ~(inj3_pin_mask);  currentStatus.isInj3Open = false; }
void openInjector4_DIRECT(void)  { *inj4_pin_port |= (inj4_pin_mask); currentStatus.isInj4Open = true; }
void closeInjector4_DIRECT(void) { *inj4_pin_port &= ~(inj4_pin_mask);  currentStatus.isInj4Open = false; }
void openInjector5_DIRECT(void)  { *inj5_pin_port |= (inj5_pin_mask); }
void closeInjector5_DIRECT(void) { *inj5_pin_port &= ~(inj5_pin_mask); }
void openInjector6_DIRECT(void)  { *inj6_pin_port |= (inj6_pin_mask); }
void closeInjector6_DIRECT(void) { *inj6_pin_port &= ~(inj6_pin_mask); }
void openInjector7_DIRECT(void)  { *inj7_pin_port |= (inj7_pin_mask); }
void closeInjector7_DIRECT(void) { *inj7_pin_port &= ~(inj7_pin_mask); }
void openInjector8_DIRECT(void)  { *inj8_pin_port |= (inj8_pin_mask); }
void closeInjector8_DIRECT(void) { *inj8_pin_port &= ~(inj8_pin_mask); }
