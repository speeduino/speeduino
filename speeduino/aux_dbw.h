#ifndef DBW_H
#define DBW_H

#include "globals.h"

void initialiseDBW(void);
void dbwControl(void);
void dbwReceivePacket(uint16_t, uint16_t);
void dbwBlipThrottle(void);

#endif