/*
 * coru, a small coroutine library
 *
 * Copyright (c) 2019 Christopher Haster
 * Distributed under the MIT license
 */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(STM32F407xx)
#include "coru.h"
#include "coru_platform.h"


// Global object for the currently running coroutine
//
// Note that the active coroutine's stack pointer is swapped with
// its parent's while running
static coru_t *coru_active = NULL;


// Coroutine operations
int coru_create(coru_t *coru, void (*cb)(void*), void *data, size_t size) {
    void *buffer = coru_malloc(size);
    if (!buffer) {
        return CORU_ERR_NOMEM;
    }

    int err = coru_create_inplace(coru, cb, data, buffer, size);
    coru->allocated = buffer;
    return err;
}

int coru_create_inplace(coru_t *coru,
        void (*cb)(void*), void *data,
        void *buffer, size_t size) {
    coru->canary = NULL;
    coru->allocated = NULL;

    int err = coru_plat_init(&coru->sp, &coru->canary, cb, data, buffer, size);
    if (err) {
        return err;
    }

    if (coru->canary) {
        *coru->canary = (uintptr_t)0x636f7275;
    }

    return 0;
}

void coru_destroy(coru_t *coru) {
    coru_free(coru->allocated);
}

int coru_resume(coru_t *coru) {
    // we should not be resuming ourselves
    CORU_ASSERT(coru != coru_active);
    // push previous coroutine's info on the current stack
    coru_t *prev = coru_active;
    coru_active = coru;
    // yield into coroutine
    int state = coru_plat_yield(&coru->sp, 0);
    // restore previous coroutine's info
    coru_active = prev;
    return state;
}

void coru_yield(void) {
    // do nothing if we are not a coroutine, this lets yield be used in
    // shared libraries
    if (!coru_active) {
        return;
    }

    // check canary, if this fails a stack overflow occured
    CORU_ASSERT(!coru_active->canary ||
            *coru_active->canary == (uintptr_t)0x636f7275);

    // yield out of coroutine
    coru_plat_yield(&coru_active->sp, CORU_ERR_AGAIN);
}

// terminate a coroutine, not public but must be called below
// when a coroutine ends
void coru_halt(void) {
    while (true) {
        coru_plat_yield(&coru_active->sp, 0);
    }
}
#endif