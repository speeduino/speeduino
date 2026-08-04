#ifndef AUX_H
#define AUX_H

#include "config_pages.h"
#include "statuses.h"
#include "src/pins/pinNumbers_t.h"

void initialiseAuxPWM(statuses &current, const pinNumbers_t &pins, const config4 &page4, const config6 &page6, config10 &page10);

void vvtControl(statuses &current, const config4 &page4, const config6 &page6, config10 &page10);
void wmiControl(statuses &current, const config10 &page10);

void vvtInterrupt(void);

#endif