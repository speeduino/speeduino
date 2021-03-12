/*
 * coru utilities and config
 *
 * Copyright (c) 2019 Christopher Haster
 * Distributed under the MIT license
 *
 * Can be overridden by users with their own configuration by defining
 * CORU_CONFIG as a header file (-DCORU_CONFIG=coru_config.h)
 *
 * If CORU_CONFIG is defined, none of the default definitions will be emitted
 * and must be provided by the user's config file. To start, I would suggest
 * copying coru_util.h and modifying as needed.
 */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(STM32F407xx)
#ifndef CORU_UTIL_H
#define CORU_UTIL_H

#ifdef CORU_CONFIG
#define CORU_STRINGIZE(x) CORU_STRINGIZE2(x)
#define CORU_STRINGIZE2(x) #x
#include CORU_STRINGIZE(CORU_CONFIG)
#else

// Standard includes, mostly needed for type definitions
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#ifndef CORU_NO_MALLOC
#include <stdlib.h>
#endif
#ifndef CORU_NO_ASSERT
#include <assert.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


// Runtime assertions
#ifndef CORU_NO_ASSERT
#define CORU_ASSERT(test) assert(test)
#else
#define CORU_ASSERT(test)
#endif


// Optional memory allocation
static inline void *coru_malloc(size_t size) {
#ifndef CORU_NO_MALLOC
    return malloc(size);
#else
    (void)size;
    return NULL;
#endif
}

static inline void coru_free(void *p) {
#ifndef CORU_NO_MALLOC
    free(p);
#else
    (void)p;
#endif
}


#ifdef __cplusplus
}
#endif

#endif
#endif
#endif