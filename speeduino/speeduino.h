#ifndef SPEEDUINO_H
#define SPEEDUINO_H

static inline unsigned int PW(int REQ_FUEL, byte VE, long MAP, int corrections, int injOpen) __attribute__((always_inline));
static inline byte getVE();
static inline byte getAdvance();

#endif
