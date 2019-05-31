#pragma once

extern uint16_t req_fuel_uS;

extern int channel1InjDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
extern int channel2InjDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
extern int channel3InjDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
extern int channel4InjDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
extern int channel5InjDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */

uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen);
