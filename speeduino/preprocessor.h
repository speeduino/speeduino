#pragma once

/** @file 
 * This file contains various shared utility macros that make writing function
 * style macros easier.
*/

#if !defined(UNUSED)
/** @brief Used to suppress unused parameter compiler warnings */
#define UNUSED(x) \
    (void)(x)
#endif

/** @brief Compile time calculation of an array size */
#if !defined(_countof)
#define _countof(x) \
    (sizeof((x)) / sizeof ((x)[0]))
#endif

/** @brief Obtain a pointer to 1 *element* past the end of an array */
#if !defined(_end_range_address)
#define _end_range_address(array) \
    ((array) + _countof((array)))
#endif

/** @brief Obtain a pointer to 1 *byte* past the end of an array */
#if !defined(_end_range_byte_address)
#define _end_range_byte_address(array) \
    (((byte*)(array)) + sizeof((array)))
#endif

/** @brief Pre-processor arithmetic increment (pulled from Boost.Preprocessor) */
#if !defined(PP_INC)
#define PP_INC(x) \
    PP_INC_I(x)
#endif

/// @cond 
// PP_INC() support macros
#define PP_INC_I(x) PP_INC_ ## x
#define PP_INC_0 1
#define PP_INC_1 2
#define PP_INC_2 3
#define PP_INC_3 4
#define PP_INC_4 5
#define PP_INC_5 6
#define PP_INC_6 7
#define PP_INC_7 8
#define PP_INC_8 9
#define PP_INC_9 10
#define PP_INC_10 11
#define PP_INC_11 12
#define PP_INC_12 13
/// @endcond

/// @cond 
// CONCAT() support macros
#define CAT_HELPER(a, b) a ## b
/// @endcond

/** @brief Concatenate A & B *after* macro expansion */
#if !defined(CONCAT)
#define CONCAT(A, B) \
    CAT_HELPER(A, B)
#endif
