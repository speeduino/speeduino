//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifndef FIRMWARE_SIMPLYATOMIC_H
#define FIRMWARE_SIMPLYATOMIC_H

#include "log.h"

#define ATOMIC() { log(ATOM, "%s:%d Atomic block\n", __FILE__, __LINE__); }

#endif //FIRMWARE_SIMPLYATOMIC_H