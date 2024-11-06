#pragma once

/**
 * @file
 * @brief Unit testability support 
 * 
 */

#if !defined(UNIT_TEST) 
/** 
 * @brief Mark an entity as having static (internal) linkage, unless a unit
 * test is in progress - then the entity is given external linkage so the test can access it.
 * 
 * Most useful for translation unit scoped variables. I.e. static within a CPP file 
 * 
 */
#define TESTABLE_STATIC static 
#else
#define TESTABLE_STATIC
#endif

#if !defined(UNIT_TEST)
/** 
 * @brief Mark an entity as having static (internal) linkage & inlined, unless a unit
 * test is in progress - then the entity is given external linkage so the test can access it.
 * 
 * Most useful for translation unit scoped functions. I.e. "static inline" within a CPP file 
 * 
 */
#define TESTABLE_INLINE_STATIC static inline 
#else
#define TESTABLE_INLINE_STATIC
#endif