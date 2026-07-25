#ifndef AUX_H
#define AUX_H

#include "config_pages.h"
#include "statuses.h"

void initialiseAuxPWM(void);
void vvtControl(void);
void initialiseAirCon(void);

void nitrousControl(void);
void airConControl(void);
void wmiControl(void);

void vvt1On(void);
void vvt1Off(void);
void vvt2On(void);
void vvt2Off(void);

void vvtInterrupt(void);

#endif