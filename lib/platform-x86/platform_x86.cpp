//
// Created by Ognjen Galić on 24. 11. 2025..
//

#include <stdio.h>
#include "platform_x86.h"

void (*trigger_interrupt)(void);

void fireInterrupts() {
    if (trigger_interrupt != nullptr) {
        trigger_interrupt();
    }
}