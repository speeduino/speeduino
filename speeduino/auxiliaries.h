#ifndef AUX_H
#define AUX_H

#include "config_pages.h"
#include "statuses.h"
#include "src/pins/pinNumbers_t.h"

/** @brief Initialise the VVT & WMI controllers */
void initialiseVvtWmi(statuses &current, const pinNumbers_t &pins, const config4 &page4, const config6 &page6, config10 &page10);

/** @brief VVT control. Should be called from the main loop */
void vvtControl(statuses &current, const config4 &page4, const config6 &page6, config10 &page10);

/** @brief WMI control. Should be called from the main loop */
void wmiControl(statuses &current, const config10 &page10);

/** @brief VVT timer callback for PWM control*/
void vvtInterrupt(void);

#endif