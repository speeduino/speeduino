/*
 * coru, platform specific functions
 *
 * Copyright (c) 2019 Christopher Haster
 * Distributed under the MIT license
 *
 * Each platform needs a small bit of custom code for coroutines because
 * of the stack manipulation. Fortunately it only takes two functions. These
 * should be added to coru_platform.c.
 */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(STM32F407xx)
#ifndef CORU_PLATFORM_H
#define CORU_PLATFORM_H

#include "coru.h"


// Initialize a coroutine stack
//
// This should set up the stack so that two things happen:
// 1. On the first call to coru_plat_yield, the callback cb should be called
//    with the data argument.
// 2. After the callback cb returns, the coroutine should then transfer control
//    to coru_halt, which does not return.
//
// After coru_plat_init, sp should contain the stack pointer for the new
// coroutine. Also, canary can be set to the end of the stack to enable best
// effort stack checking. Highly suggested.
//
// Any other platform initializations or assertions can be carried out here.
int coru_plat_init(void **sp, uintptr_t **canary,
        void (*cb)(void*), void *data,
        void *buffer, size_t size);

// Yield a coroutine
//
// This is where the magic happens.
//
// Several things must happen:
// 1. Store any callee saved registers/state
// 2. Store arg in temporary register
// 3. Swap sp and stack pointer, must store old stack pointer in sp
// 4. Return arg from temporary register
//
// Looking at the i386 implementation may be helpful
uintptr_t coru_plat_yield(void **sp, uintptr_t arg);


#endif
#endif