#pragma once

void initInjDirectIO(const uint8_t (&pins)[INJ_CHANNELS]);

//Macros are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT macros (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file
void openInjector1_DIRECT(void);
void closeInjector1_DIRECT(void);
void openInjector2_DIRECT(void);
void closeInjector2_DIRECT(void);
void openInjector3_DIRECT(void);
void closeInjector3_DIRECT(void);
void openInjector4_DIRECT(void);
void closeInjector4_DIRECT(void);
void openInjector5_DIRECT(void);
void closeInjector5_DIRECT(void);
void openInjector6_DIRECT(void);
void closeInjector6_DIRECT(void);
void openInjector7_DIRECT(void);
void closeInjector7_DIRECT(void);
void openInjector8_DIRECT(void);
void closeInjector8_DIRECT(void);
