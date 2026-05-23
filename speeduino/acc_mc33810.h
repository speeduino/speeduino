#ifndef MC33810_H
#define MC33810_H

#include "config_pages.h"
#include "board_definition.h"
#include "src/pins/pinNumbers_t.h"

#if defined(MC33810_SUPPORT)
void initMC33810(const config4 &page4, const pinNumbers_t &pinNumbers);

void openInjector_MC33810(uint8_t channel);
void closeInjector_MC33810(uint8_t channel);

void coilCharging_MC33810(uint8_t channel);
void coilStopCharging_MC33810(uint8_t channel);

#endif

#endif