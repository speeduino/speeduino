#ifndef SPEEDUINO_H
#define SPEEDUINO_H

uint16_t PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen);
byte getVE();
byte getAdvance();

uint16_t calculateInjector2StartAngle(unsigned int);
uint16_t calculateInjector3StartAngle(unsigned int);
uint16_t calculateInjector4StartAngle(unsigned int);
uint16_t calculateInjector4StartAngle(unsigned int);

#endif
